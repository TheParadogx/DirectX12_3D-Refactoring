#include "pch.h"
#include "InputManager.hpp"

Ecse::System::InputManager::InputManager()
{
	// 各マネージャーのインスタンス化
		// 各クラスが独自に ServiceLocator を使う場合も考慮し、ここで生成
	mKeyboard = std::make_unique<Keyboard>();
	mMouse = std::make_unique<Mouse>();
	mGamepadManager = std::make_unique<GamepadManager>();

	mMouse->RegisterDebugUI();
}

Ecse::System::InputManager::~InputManager() = default;

/// <summary>
/// Windowsのメッセージを処理する。該当する入力イベントがあれば、各デバイスに状態を反映させる。
/// </summary>
/// <returns>true:該当</returns>
bool Ecse::System::InputManager::ProcessMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	// キーボードに転送
	if (mKeyboard->OnInputEvent(msg, wparam, lparam)) return true;
	
	// マウスに転送
	if (mMouse->OnInputEvent(msg, wparam, lparam)) return true;
	
	return false;
}

/// <summary>
/// 全デバイスの状態更新（フレームの先頭で呼ぶ）
/// </summary>
void Ecse::System::InputManager::Update()
{
	mKeyboard->Update();
	mMouse->Update();
	mGamepadManager->Update();
}
