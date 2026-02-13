#include "pch.h"
#include<System/Engine/Engine.hpp>
#include<System/Engine/EngineContext.hpp>
#include<System/Service/ServiceLocator.hpp>

#include<System/Window/Window.hpp>

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
		if (mIsInitialized == true) return false;

		ECSE_LOG(ELogLevel::Log, "Engine Initialize.");

		//	ウィンドウ
		if (Window::Create() == false)
		{
			ECSE_LOG(ELogLevel::Fatal, "Failed Window Create.");
			return false;
		}
		mpWindow = ServiceLocator::Get<Window>();
		if (mpWindow == nullptr || mpWindow->Initialize(Context.WinSetting) == false)
		{
			return false;
		}





		// 全ての初期化正常終了後にフラグを立てる
		mIsInitialized = true;

		return true;
	}
}

