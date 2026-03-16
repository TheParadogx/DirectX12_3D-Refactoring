#pragma once
#include<Utility/Export/Export.hpp>
#include<System/Service/ServiceProvider.hpp>
#include<System/Input/Mouse/Mouse.h>
#include<System/Input/Keyboard/Keyboard.hpp>
#include<System/Input/Gamepad/GamepadManager.hpp>
#include<memory>
namespace Ecse::System
{
	class ENGINE_API InputManager: public System::ServiceProvider<InputManager>
	{
		ECSE_SERVICE_ACCESS(InputManager);
	public:
		InputManager();
		virtual ~InputManager();

		// コピー禁止
		InputManager(const InputManager&) = delete;
		InputManager& operator=(const InputManager&) = delete;

		/// <summary>
		/// Windowsのメッセージを処理する。該当する入力イベントがあれば、各デバイスに状態を反映させる。
		/// </summary>
		/// <returns>true:該当</returns>
		bool ProcessMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

		/// <summary>
		/// 全デバイスの状態更新（フレームの先頭で呼ぶ）
		/// </summary>
		void Update();

		// アクセサ
		Mouse* GetMouse()    const { return mMouse.get(); }
		Keyboard* GetKeyboard() const { return mKeyboard.get(); }
		GamepadManager* GetGamepad()  const { return mGamepadManager.get(); }
	private:
		/// <summary>
		/// マウス管理
		/// </summary>
		std::unique_ptr<Mouse> mMouse;
		/// <summary>
		/// キーボード管理
		/// </summary>
		std::unique_ptr<Keyboard> mKeyboard;
		/// <summary>
		/// ゲームパッド管理
		/// </summary>
		std::unique_ptr<GamepadManager> mGamepadManager;
	};
}


