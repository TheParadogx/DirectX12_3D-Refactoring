#pragma once
#include<array>
#include<System/Input/IInputSubscriber.hpp>
#include<DirectXMath.h>
#include"MouseButton.hpp"

namespace Ecse::System
{
	class Mouse : public IInputSubscriber
	{
		/// <summary>
		/// Windowsのマウスメッセージから、eMouseButtonへの変換
		/// </summary>
		eMouseButton ToMouseButton(UINT message, WPARAM wParam) const;

		/// <summary>
		/// 入力状態の更新
		/// </summary>
		void SetInputState(eMouseButton button, bool isDown);

		/// <summary>
		/// 入力範囲の判定
		/// </summary>
		/// <returns>true:範囲内</returns>
		bool IsValid(eMouseButton button) const;
	public:
		Mouse();
		virtual ~Mouse() = default;

		/// <summary>
		/// 入力イベントの取得
		/// </summary>
		/// <returns>true:該当イベント</returns>
		bool OnInputEvent(UINT msg, WPARAM wp, LPARAM lp) override;

		/// <summary>
		/// 押した瞬間、離した瞬間の判定のために、前回状態の更新などを行う
		/// </summary>
		void Update() override;

		/// <summary>
		/// 押した瞬間
		/// </summary>
		/// <param name="button">ボタンコード</param>
		/// <returns></returns>
		bool IsPressed(eMouseButton button)const;

		/// <summary>
		/// 押している間
		/// </summary>
		/// <param name="keyCode">ボタンコード</param>
		/// <returns></returns>
		bool IsHeld(eMouseButton button)const;

		/// <summary>
		/// 離した瞬間
		/// </summary>
		/// <param name="keyCode">ボタンコード</param>
		/// <returns></returns>
		bool IsReleased(eMouseButton button)const;

		/// <summary>
		/// ImGuiManagerにデバッグUIを登録する
		/// </summary>
		void RegisterDebugUI();


	private:
		/// <summary>
		/// 入力状態
		/// </summary>
		using ButtonState = std::array<bool, static_cast<int>(eMouseButton::Count)>;
		ButtonState mCurrButton;
		ButtonState mPrevButton;

		/// <summary>
		/// 今の座標
		/// </summary>
		DirectX::XMFLOAT2 mPosition;
		/// <summary>
		/// 前の座標
		/// </summary>
		DirectX::XMFLOAT2 mPrevPosition;
		/// <summary>
		/// 移動量
		/// </summary>
		DirectX::XMFLOAT2 mDeltaPosition;

		/// <summary>
		/// ホイールの回転量
		/// </summary>
		float mWheel;
		/// <summary>
		/// 蓄積されたホイールの回転量
		/// </summary>
		float mWheelAccumulator;
	};
}

