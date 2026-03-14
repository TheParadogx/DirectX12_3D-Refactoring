#pragma once
#include<array>
#include<System/Input/IInputSubscriber.hpp>
#include<DirectXMath.h>
#include"MouseButton.hpp"

namespace Ecse::System
{
	class Mouse : public IInputSubscriber
	{
		eMouseButton ToMouseButton(UINT message, WPARAM wParam) const;
		void SetInputState(eMouseButton button, bool isDown);
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
		/// <param name="keyCode">キーコード</param>
		/// <returns></returns>
		bool IsPressed(eMouseButton keyCode)const;

		/// <summary>
		/// 押している間
		/// </summary>
		/// <param name="keyCode">キーコード</param>
		/// <returns></returns>
		bool IsHeld(eMouseButton keyCode)const;

		/// <summary>
		/// 離した瞬間
		/// </summary>
		/// <param name="keyCode">キーコード</param>
		/// <returns></returns>
		bool IsReleased(eMouseButton keyCode)const;

		/// <summary>
		/// ImGuiManagerにデバッグUIを登録する
		/// </summary>
		void RegisterDebugUI();


	private:
		/// <summary>
		/// 入力状態
		/// </summary>
		using ButtonState = std::array<bool, static_cast<size_t>(eMouseButton::Count)>;
		ButtonState CurrButton;
		ButtonState PrevButton;

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
		float mInternalWheelAccumulator;
	};
}

