#include "pch.h"
#include<Debug/ImGui/ImGuiManager.hpp>

#include<System/Window/Window.hpp>
#include<Graphics/DX12/DX12.hpp>
#include<Graphics/GraphicsDescriptorHeap/GDescriptorHeapManager.hpp>

#include<ImGui/imgui_impl_win32.h>
#include<ImGui/imgui_impl_dx12.h>

namespace Ecse::Debug
{
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
            ECSE_LOG(System::ELogLevel::Fatal, "Failed Services not find.");
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


        return true;
    }

}

