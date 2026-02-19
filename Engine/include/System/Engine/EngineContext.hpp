#pragma once
#include<System/Window/WindowSetting.hpp>
#include<Utility/Export/Export.hpp>

namespace Ecse::System
{
	/// <summary>
	/// エンジンの初期化情報全体をまとめる構造体
	/// </summary>
	struct ENGINE_API EngineContext
	{
		//	ウィンドウの初期化設定
		WindowSetting WinSetting;

		/*
		* エンジンの初期化で追加する場合はここで追加。
		*/
	};
}