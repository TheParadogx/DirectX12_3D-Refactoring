#pragma once

#include<winrt/Windows.Gaming.Input.h>
#include<array>
#include<DirectXMath.h>
#include"PadButton.hpp"

using WgiGamepad = winrt::Windows::Gaming::Input::Gamepad;

namespace Ecse::System
{
	class Gamepad
	{

		/// <summary>
		/// 指定フレームでのボタン状態を取得
		/// </summary>
		/// <returns></returns>
		bool GetButtonState(ePadButton button, bool current) const;

		/// <summary>
		/// デッドゾーンの適応
		/// </summary>
		/// <returns></returns>
		DirectX::XMFLOAT2 FilterStick(float x, float y, float deadZone) const;
	public:
		/// <summary>
		/// 初期化
		/// </summary>
		/// <param name="Gamepad"></param>
		Gamepad(const WgiGamepad& Gamepad, const float RightStickDeadZone = 0.1f, const float LeftStickDeadZone = 0.2f, const float TriggerDeadZone = 0.1f);
		virtual ~Gamepad() = default;

		/// <summary>
		/// 前回状態の保存
		/// </summary>
		void Update();

		/// <summary>
		/// 押した瞬間
		/// </summary>
		/// <param name="button">ボタン種別</param>
		/// <returns>true:押した瞬間</returns>
		bool IsPressed(const ePadButton Button) const;

		/// <summary>
		/// 押している間
		/// </summary>
		/// <param name="button"></param>
		/// <returns>true:押されている</returns>
		bool IsHeld(const ePadButton Button) const;

		/// <summary>
		/// 離した瞬間
		/// </summary>
		/// <param name="button"></param>
		/// <returns>true:離した瞬間</returns>
		bool IsReleased(const ePadButton Button) const;

		/// <summary>
		/// デバイスの固有IDなどを比較したい場合にも便利
		/// </summary>
		bool IsSameDevice(const WgiGamepad& other) const {
			return mGamepad == other;
		}

		/// <summary>
		/// 左スティック、右スティックの3D用ベクトルを取得する。（X=X,Y=Z）
		/// </summary>
		/// <returns></returns>
		DirectX::XMFLOAT2 GetLeftStick3D() const;
		DirectX::XMFLOAT2 GetRightStick3D() const;

		/// <summary>
		/// 左スティック、右スティックの2Dベクトルを取得する
		/// </summary>
		/// <returns></returns>
		DirectX::XMFLOAT2 GetLeftStick2D() const;
		DirectX::XMFLOAT2 GetRightStick2D() const;


		// --- トリガー・デッドゾーン管理 ---
		float GetLeftTrigger() const { return mLeftTrigger; }
		float GetRightTrigger() const { return mRightTrigger; }

		void SetRightStickDeadZone(float zone) { mRightStickDeadZone = zone; }
		void SetLeftStickDeadZone(float zone) { mLeftStickDeadZone = zone; }
		void SetTriggerDeadZone(float zone) { mTriggerDeadZone = zone; }

		Gamepad GetGamepad() const { return mGamepad; }

		void ImGuiUpdate();
	private:

		enum class Trigger :int
		{
			Left = 0,
			Right,
			Count,
		};

		/// <summary>
		/// PAD本体
		/// </summary>
		WgiGamepad mGamepad;

		/// <summary>
		/// 左スティック
		/// </summary>
		DirectX::XMFLOAT2 mLeftStick;
		/// <summary>
		/// 右スティック
		/// </summary>
		DirectX::XMFLOAT2 mRightStick;

		/// <summary>
		///	右スティックのデッドゾーン
		/// </summary>
		float mLeftStickDeadZone;

		/// <summary>
		///	右スティックのデッドゾーン
		/// </summary>
		float mRightStickDeadZone;

		/// <summary>
		/// 左トリガー 0.0~1.0
		/// </summary>
		float mLeftTrigger;

		/// <summary>
		/// 右トリガー 0.0~1.0
		/// </summary>
		float mRightTrigger;

		/// <summary>
		/// 今のトリガー入力状態
		/// </summary>
		std::array<bool, static_cast<int>(Trigger::Count)> mCurrTrigger;

		/// <summary>
		/// 前のトリガーの入力状態
		/// </summary>
		std::array<bool, static_cast<int>(Trigger::Count)> mPrevTrigger;

		/// <summary>
		/// トリガーのデッドゾーン
		/// </summary>
		float mTriggerDeadZone;

		/// <summary>
		/// 今のボタンの状態
		/// </summary>
		uint32_t mCurrButtons;

		/// <summary>
		/// 前のボタンの状態
		/// </summary>
		uint32_t mPrevButtons;


	};
}

