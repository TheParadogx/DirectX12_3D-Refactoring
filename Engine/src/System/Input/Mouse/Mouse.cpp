#include "pch.h"
#include "Mouse.h"

namespace Ecse::System
{

	/// <summary>
	/// Windowsのマウスメッセージから、eMouseButtonへの変換
	/// </summary>
	eMouseButton Mouse::ToMouseButton(UINT message, WPARAM wParam) const
	{
		switch (message)
		{
		case WM_LBUTTONDOWN: case WM_LBUTTONUP: return eMouseButton::Left;
		case WM_RBUTTONDOWN: case WM_RBUTTONUP: return eMouseButton::Right;
		case WM_MBUTTONDOWN: case WM_MBUTTONUP: return eMouseButton::Middle;
		case WM_XBUTTONDOWN: case WM_XBUTTONUP:
			return (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? eMouseButton::XButton1 : eMouseButton::XButton2;
		default: return eMouseButton::Unknown;
		}
	}

	/// <summary>
	/// 入力状態の更新
	/// </summary>
	void Mouse::SetInputState(eMouseButton button, bool isDown)
	{
		if (IsValid(button))
		{
			mCurrButton[static_cast<int>(button)] = isDown;
		}
	}

	/// <summary>
	/// 入力範囲の判定
	/// </summary>
	/// <returns>true:範囲内</returns>
	bool Mouse::IsValid(eMouseButton button) const
	{
		return button > eMouseButton::Unknown && button < eMouseButton::Count;
	}

	Mouse::Mouse()
		:mPosition(0.0f,0.0f)
		,mPrevPosition(0.0f,0.0f)
		 ,mDeltaPosition(0.0f,0.0f)
		 ,mWheel(0.0f)
		, mWheelAccumulator(0.0f)
	{
		mCurrButton.fill(false);
		mPrevButton.fill(false);
	}

	/// <summary>
	/// 入力イベントの取得
	/// </summary>
	/// <returns>true:該当イベント</returns>
	bool Mouse::OnInputEvent(UINT msg, WPARAM wp, LPARAM lp)
	{
		if (msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST)
		{
			mPosition.x = static_cast<float>(LOWORD(lp));
			mPosition.y = static_cast<float>(HIWORD(lp));
		}

		switch (msg)
		{
		case WM_LBUTTONDOWN: case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN: case WM_XBUTTONDOWN:
			SetInputState(ToMouseButton(msg, wp), true);
			return true;

		case WM_LBUTTONUP: case WM_RBUTTONUP:
		case WM_MBUTTONUP: case WM_XBUTTONUP:
			SetInputState(ToMouseButton(msg, wp), false);
			return true;

		case WM_MOUSEWHEEL:
			// 120単位(WHEEL_DELTA)を1.0として累積
			mWheelAccumulator += static_cast<float>(GET_WHEEL_DELTA_WPARAM(wp)) / static_cast<float>(WHEEL_DELTA);
			return true;
		}

		return false;
	}

	/// <summary>
	/// 押した瞬間、離した瞬間の判定のために、前回状態の更新などを行う
	/// </summary>
	void Mouse::Update()
	{
		// 移動量の計算
		mDeltaPosition.x = mPosition.x - mPrevPosition.x;
		mDeltaPosition.y = mPosition.y - mPrevPosition.y;

		// 次フレームのためのバックアップ
		mPrevPosition = mPosition;
		mPrevButton = mCurrButton;

		// ホイール値の確定
		mWheel = mWheelAccumulator;
		mWheelAccumulator = 0.0f;
	}

	/// <summary>
	/// 押した瞬間
	/// </summary>
	/// <param name="button">ボタンコード</param>
	/// <returns></returns>
	bool Mouse::IsPressed(eMouseButton button) const
	{
		if (!IsValid(button)) return false;
		int idx = static_cast<int>(button);
		return mCurrButton[idx] && !mPrevButton[idx];
	}

	/// <summary>
	/// 押している間
	/// </summary>
	/// <param name="keyCode">ボタンコード</param>
	/// <returns></returns>
	bool Mouse::IsHeld(eMouseButton button) const
	{
		if (!IsValid(button)) return false;
		return mCurrButton[static_cast<int>(button)];
	}

	/// <summary>
	/// 離した瞬間
	/// </summary>
	/// <param name="keyCode">ボタンコード</param>
	/// <returns></returns>
	bool Mouse::IsReleased(eMouseButton button) const
	{
		if (!IsValid(button)) return false;
		int idx = static_cast<int>(button);
		return !mCurrButton[idx] && mPrevButton[idx];
	}

	/// <summary>
	/// ImGuiManagerにデバッグUIを登録する
	/// </summary>
	void Mouse::RegisterDebugUI()
	{
		auto imgui = System::ServiceLocator::Get<Debug::ImGuiManager>();
		if (imgui == nullptr) return;
		imgui->AddDebugUI([this]()
			{
				if (ImGui::Begin("Mouse Diagnostics"))
				{
					ImGui::Text("Client Pos: (%.1f, %.1f)", mPosition.x, mPosition.y);
					ImGui::Text("Delta:      (%.1f, %.1f)", mDeltaPosition.x, mDeltaPosition.y);
					ImGui::Text("Wheel:      %.2f", mWheel);

					ImGui::Separator();
					ImGui::Text("Buttons:");
					auto DisplayBtn = [this](const char* name, eMouseButton b) {
						bool h = IsHeld(b);
						if (h) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0, 1));
						ImGui::Text("[%s]", name);
						if (h) ImGui::PopStyleColor();
						ImGui::SameLine();
						};

					DisplayBtn("L", eMouseButton::Left);
					DisplayBtn("R", eMouseButton::Right);
					DisplayBtn("M", eMouseButton::Middle);
					DisplayBtn("X1", eMouseButton::XButton1);
					DisplayBtn("X2", eMouseButton::XButton2);
					ImGui::NewLine();
				}
				ImGui::End();
			});
	}
}
