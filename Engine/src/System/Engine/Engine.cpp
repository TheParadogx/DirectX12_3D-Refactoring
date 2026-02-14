#include "pch.h"
#include<System/Engine/Engine.hpp>
#include<System/Engine/EngineContext.hpp>
#include<System/Service/ServiceLocator.hpp>

#include<System/Window/Window.hpp>
#include<System/Log/Logger.hpp>
#include<System/EngineConfig.hpp>
#include<Graphics/DX12/DX12.hpp>
#include<Debug/ImGui/ImGuiManager.hpp>
#include<Graphics/GraphicsDescriptorHeap/GDescriptorHeapManager.hpp>


namespace Ecse::System
{
	void Engine::OnCreate()
	{
		mpWindow = nullptr;
		mIsInitialized = false;
	}

	/// <summary>
	/// エンジン全体の初期化
	/// OSやGraphicsなど全て
	/// </summary>
	/// <param name="Context">初期化に必要な情報</param>
	/// <returns>true:成功</returns>
	bool Engine::Initialize(const EngineContext& Context)
	{

		using namespace Graphics;
		using namespace Debug;

		if (mIsInitialized == true) return false;

		// ログ
		Logger::Create();

		ECSE_LOG(ELogLevel::Log, "Engine Initialize.");


		//	短くしたら見やすいのか見にくいのか分らなくなってきた。
		//	ウィンドウ
		if (Window::Create() == false) return false;
		mpWindow = ServiceLocator::Get<Window>();
		if (mpWindow->Initialize(Context.WinSetting) == false)return false;

		// Dx12
		if (DX12::Create() == false) return false;
		mpDX12 = ServiceLocator::Get<DX12>();
		if (mpDX12->Initialize(mpWindow->GetHWND(), mpWindow->GetWidth(), mpWindow->GetHeight()) == false) return false;

		//	GDHManager
		if (GDescriptorHeapManager::Create() == false) return false;
		auto gdh = ServiceLocator::Get<GDescriptorHeapManager>();
		if (gdh->Initialize() == false) return false;


		// ImGui
		if (Debug::ImGuiManager::Create() == false) return false;
		mpImGui = ServiceLocator::Get<ImGuiManager>();
		if (mpImGui->Initialize() == false) return false;

		// 全ての初期化正常終了後にフラグを立てる
		mIsInitialized = true;

		return true;
	}

	/// <summary>
	/// メインループ
	/// これを While(Engine->Run()) みたいに呼び出す。
	/// エンジン外のデバックツールなどを表示できるようにする。
	/// </summary>
	/// <returns>true:続行 false:終了</returns>
	bool Engine::Run()
	{
		if (mIsInitialized == false) return false;

		//	OSメッセージ処理
		mpWindow->ProcessMessages();

		//	終了判定
		if (mpWindow->IsQuitRequested())
		{
			return false;
		}

		this->NewFrame();

		// 状態更新

		//	描画

		this->EndFrame();



		return true;
	}

	void Engine::Shutdown()
	{
		if (mIsInitialized == false) return;

		ECSE_LOG(ELogLevel::Log, "Engine Shutdown.");

		if(mpImGui->IsCreated()) mpImGui->Release();
		Window::Release();
		mIsInitialized == false;
	}

	void Engine::NewFrame()
	{
		mpDX12->BegineRendering();
		mpDX12->SetViewPort(0, 0, mpWindow->GetWidth(), mpWindow->GetHeight());
		mpImGui->NewFrame();
	}

	void Engine::EndFrame()
	{
		// ImGuiのRenderなどもここに呼ぶ
		mpImGui->EndFrame();
		mpDX12->Flip();
	}
}

