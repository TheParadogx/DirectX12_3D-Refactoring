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
#include<ECS/Entity/EntityManager.hpp>
#include<Graphics/Shader/ShaderManager.hpp>
#include<System/Resource/ResourceManager.hpp>

#include<Graphics/Sprite/Renderer/SpriteRenderer.hpp>
#include<Graphics/Texture/Texture.hpp>
#include<ECS/Component/Transform/TransformComponent.hpp>
#include<ECS/Component/Sprite/SpriteComponent.hpp>

static Ecse::Graphics::SpriteRenderer sRenderer;
static Ecse::Graphics::Texture sTexture;

void CreateText()
{
	using namespace Ecse;
	auto manager= System::ServiceLocator::Get<ECS::EntityManager>();
	auto& registry = manager->GetRegistry();
	auto entity = manager->CreateEntity();
	
	// 座標
	auto& trans = registry.emplace<ECS::Transform2D>(entity);
	//trans.Position = { 100,100 };

	auto& sprite = registry.emplace<ECS::Sprite>(entity,&sTexture);
	sprite.Size = { 1920,1080 };
}

namespace Ecse::System
{
	void PrintCurrentDir()
	{
		wchar_t buffer[MAX_PATH];
		GetCurrentDirectoryW(MAX_PATH, buffer);
		std::wcout << L"CurrentDir: " << buffer << std::endl;
	}

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

		ECSE_LOG(eLogLevel::Log, "Engine Initialize.");


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


#if defined(_DEBUG) || ECSE_DEV_TOOL_ENABLED
		// ImGui
		if (Debug::ImGuiManager::Create() == false) return false;
		mpImGui = ServiceLocator::Get<ImGuiManager>();
		if (mpImGui->Initialize() == false) return false;
#endif

		//	EntityManager
		if (ECS::EntityManager::Create() == false) return false;
		mpEntityManager = ServiceLocator::Get<ECS::EntityManager>();
		if (mpEntityManager->Initialize() == false) return false;

		// ShaderManager
		if (Graphics::ShaderManager::Create() == false) return false;

		PrintCurrentDir();

		// Resource
		if (System::ResourceManager::Create() == false) return false;


		if (sRenderer.Initialize() == false) return false;

		sTexture.Create("Assets/Test/test.png");

		CreateText();

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

#if defined(_DEBUG) || ECSE_DEV_TOOL_ENABLED
		mpImGui->Update();
#endif
		//	描画

		auto list = mpDX12->GetCommandList();
		auto& reg = mpEntityManager->GetRegistry();

		sRenderer.Begin();
		sRenderer.UpdateAndDraw(reg);
		sRenderer.End(list);


		this->EndFrame();

		//	エンティティの削除
		mpEntityManager->Update();

		return true;
	}

	void Engine::Shutdown()
	{
		if (mIsInitialized == false) return;

		ECSE_LOG(eLogLevel::Log, "Engine Shutdown.");

		if(mpImGui->IsCreated()) mpImGui->Release();
		Window::Release();
		mIsInitialized = false;
	}

	void Engine::NewFrame()
	{
		mpDX12->BeginRendering();
		mpDX12->SetViewPort(
			(float)mpWindow->GetWidth(),
			(float)mpWindow->GetHeight(),
			0.0f,
			0.0f
		); 
#if defined(_DEBUG) || ECSE_DEV_TOOL_ENABLED
		mpImGui->NewFrame();
#endif
	}

	void Engine::EndFrame()
	{
		// ImGuiのRenderなどもここに呼ぶ
#if defined(_DEBUG) || ECSE_DEV_TOOL_ENABLED
		mpImGui->EndFrame();
#endif
		mpDX12->Flip();
	}
}

