#include "pch.h"
#include<Debug/ImGui/ImGuiManager.hpp>

#include<System/Window/Window.hpp>
#include<Graphics/DX12/DX12.hpp>
#include<Graphics/GraphicsDescriptorHeap/GDescriptorHeapManager.hpp>
#include<Graphics/GraphicsDescriptorHeap/GDescriptorHeap.hpp>

#include<ImGui/imgui_impl_win32.h>
#include<ImGui/imgui_impl_dx12.h>

namespace Ecse::Debug
{
    ImGuiManager::ImGuiManager()
        :mHeap(nullptr)
        , mContext(nullptr)
        , mIsInitialized(false)
    {
    }

    ImGuiManager::~ImGuiManager() = default;

    /// <summary>
    /// 初期化
    /// </summary>
    /// <returns>true:成功</returns>
    bool ImGuiManager::Initialize()
    {
        //  必要なパーツを全て取得
        auto dx12 = System::ServiceLocator::Get<Graphics::DX12>();
        auto gdhManager = System::ServiceLocator::Get<Graphics::GDescriptorHeapManager>();
        auto window = System::ServiceLocator::Get<System::Window>();

        if (dx12 == nullptr || gdhManager == nullptr || window == nullptr)
        {
            ECSE_LOG(System::eLogLevel::Fatal, "Failed Services not find.");
            return false;
        }

        //  WindowクラスからHWNDの取得
        HWND hwnd = window->GetHWND();

        // ImGuiのコンテキスト作成
        IMGUI_CHECKVERSION();
        mContext = ImGui::CreateContext();
        ImGui::SetCurrentContext(mContext);

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // キーボード操作有効
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // ドッキング有効
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;    // マルチビューポート有効

        // スタイル設定
        ImGui::StyleColorsDark();

        // Win32のバックエンド初期化
        if (ImGui_ImplWin32_Init(hwnd) == false)
        {
            ECSE_LOG(System::eLogLevel::Fatal, "Failed to init Win32 backend.");
            return false;
        }

        //  ディスクリプタを1枠確保（フォント用）
        mHeap = std::make_unique<Graphics::GDescriptorHeap>();
        if (mHeap->Create(1) == false)
        {
            ECSE_LOG(System::eLogLevel::Fatal, "Failed to allocate Descriptor slot.");
            return false;
        }

        // Dx12バックエンド
        ImGui_ImplDX12_InitInfo initInfo = {};
        initInfo.Device = dx12->GetDevice();
        initInfo.CommandQueue = dx12->GetCommandQueue();
        initInfo.NumFramesInFlight = dx12->FRAME_COUNT;
        initInfo.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        initInfo.DSVFormat = DXGI_FORMAT_D32_FLOAT; // 未使用なら DXGI_FORMAT_UNKNOWN でも可

        // RAIIクラスからハンドルの取得
        initInfo.SrvDescriptorHeap = gdhManager->GetNativeHeap();
        initInfo.LegacySingleSrvCpuDescriptor = mHeap->GetCpuHandle();
        initInfo.LegacySingleSrvGpuDescriptor = mHeap->GetGpuHandle();

        if (ImGui_ImplDX12_Init(&initInfo) == false)
        {
            ECSE_LOG(System::eLogLevel::Fatal, "Failed to init DX12 backend.");
            return false;
        }

        mIsInitialized = true;
        ECSE_LOG(System::eLogLevel::Log, "Initialize Success.");
        return true;
    }

    /// <summary>
    ///フレーム開始 
    /// </summary>
    void ImGuiManager::NewFrame()
    {
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    /// <summary>
    /// 状態更新
    /// </summary>
    void ImGuiManager::Update()
    {
        for (auto& func : mDebugUIFunctions)
        {
            func();
        }
    }

    /// <summary>
    /// フレームの終了・描画
    /// </summary>
    void ImGuiManager::EndFrame()
    {
        //  描画データの作成
        ImGui::Render();

        auto dx12 = System::ServiceLocator::Get<Graphics::DX12>();
        auto gdhManager = System::ServiceLocator::Get<Graphics::GDescriptorHeapManager>();

        //  描画に使用するコマンドリストの取得
        auto cmdList = dx12->GetCommandList();

        // ImGuiを描画するためにディスクリプタヒープをセット
        // DX12ではSetDescriptorHeapsは最後にセットしたものが有効になるため、描画直前に呼ぶ
        ID3D12DescriptorHeap* heaps[] = { gdhManager->GetNativeHeap() };
        cmdList->SetDescriptorHeaps(_countof(heaps), heaps);

        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList);

        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

    }

    /// <summary>
    /// 解放
    /// </summary>
    void ImGuiManager::Shutdown()
    {
        if (mIsInitialized == false)return;

        //  バックエンドの終了
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();

        if (mContext != nullptr)
        {
            ImGui::DestroyContext(mContext);
            mContext = nullptr;
        }

        // RAIIオブジェクトをリセット（ディスクリプタ枠の返却）
        if (mHeap)
        {
            mHeap.reset();
        }

        mIsInitialized = false;
        ECSE_LOG(System::eLogLevel::Log, "ImGuiManager Shutdown.");
    }

    /// <summary>
    /// 外部からデバッグUI関数を登録する
    /// </summary>
    /// <param name="guiFunc"></param>
    void ImGuiManager::AddDebugUI(std::function<void()> guiFunc)
    {
        mDebugUIFunctions.push_back(guiFunc);
    }

}

