#include "pch.h"
#include "Gamepad.hpp"

/// <summary>
/// 指定フレームでのボタン状態を取得
/// </summary>
/// <returns></returns>
bool Ecse::System::Gamepad::GetButtonState(ePadButton button, bool current) const
{
    if (button == ePadButton::L2) return current ? mCurrTrigger[0] : mPrevTrigger[0];
    if (button == ePadButton::R2) return current ? mCurrTrigger[1] : mPrevTrigger[1];

    const uint32_t mask = static_cast<uint32_t>(ToWgiButton(button));
    return ((current ? mCurrButtons : mPrevButtons) & mask) != 0;
}

DirectX::XMFLOAT2 Ecse::System::Gamepad::FilterStick(float x, float y, float deadZone) const
{
    float mag = std::sqrt(x * x + y * y);
    if (mag < deadZone) return { 0.0f, 0.0f };

    // デッドゾーンを考慮した 0.0 ~ 1.0 への再マッピング
    float normalizedMag = std::min(1.0f, (mag - deadZone) / (1.0f - deadZone));
    return { (x / mag) * normalizedMag, (y / mag) * normalizedMag };
}

Ecse::System::Gamepad::Gamepad(const WgiGamepad& Gamepad, const float RightStickDeadZone, const float LeftStickDeadZone, const float TriggerDeadZone)
	:mGamepad(Gamepad)
	,mLeftStick(0.0f,0.0f)
	,mRightStick(0.0f,0.0f)
	,mLeftStickDeadZone(LeftStickDeadZone)
	,mRightStickDeadZone(RightStickDeadZone)
	,mLeftTrigger(0.0f)
	,mRightTrigger(0.0f)
	, mTriggerDeadZone(TriggerDeadZone)
    , mCurrButtons(0)
	, mPrevButtons(0)
{
	mCurrTrigger.fill(false);
	mPrevTrigger.fill(false);
}

/// <summary>
/// 前回状態の保存
/// </summary>
void Ecse::System::Gamepad::Update()
{
    mPrevButtons = mCurrButtons;
    mPrevTrigger = mCurrTrigger;

    auto reading = mGamepad.GetCurrentReading();

    // ボタン状態（ビットフラグ）
    mCurrButtons = static_cast<uint32_t>(reading.Buttons);

    // スティック処理
    mLeftStick = FilterStick(reading.LeftThumbstickX, reading.LeftThumbstickY, mLeftStickDeadZone);
    mRightStick = FilterStick(reading.RightThumbstickX, reading.RightThumbstickY, mRightStickDeadZone);

    // トリガー処理
    mLeftTrigger = static_cast<float>(reading.LeftTrigger);
    mRightTrigger = static_cast<float>(reading.RightTrigger);

    mCurrTrigger[static_cast<int>(Trigger::Left)] = (mLeftTrigger > mTriggerDeadZone);
    mCurrTrigger[static_cast<int>(Trigger::Right)] = (mRightTrigger > mTriggerDeadZone);
}

/// <summary>
/// 押した瞬間
/// </summary>
/// <param name="button">ボタン種別</param>
/// <returns>true:押した瞬間</returns>
bool Ecse::System::Gamepad::IsPressed(const ePadButton Button) const
{
    return GetButtonState(Button, true) && !GetButtonState(Button, false);
}

/// <summary>
/// 押している間
/// </summary>
/// <param name="button"></param>
/// <returns>true:押されている</returns>
bool Ecse::System::Gamepad::IsHeld(const ePadButton Button) const
{
    return GetButtonState(Button, true);
}

/// <summary>
/// 離した瞬間
/// </summary>
/// <param name="button"></param>
/// <returns>true:離した瞬間</returns>
bool Ecse::System::Gamepad::IsReleased(const ePadButton Button) const
{
    return !GetButtonState(Button, true) && GetButtonState(Button, false);
}

DirectX::XMFLOAT2 Ecse::System::Gamepad::GetLeftStick3D() const
{
    return mLeftStick;
}

DirectX::XMFLOAT2 Ecse::System::Gamepad::GetRightStick3D() const
{
    return mRightStick;
}

DirectX::XMFLOAT2 Ecse::System::Gamepad::GetLeftStick2D() const
{
	DirectX::XMFLOAT2 mLeftStick2D = mLeftStick;
	mLeftStick2D.y *= -1.0f; // Y軸を反転
    return mLeftStick2D;
}

DirectX::XMFLOAT2 Ecse::System::Gamepad::GetRightStick2D() const
{
    DirectX::XMFLOAT2 mRightStick3D = mRightStick;
    mRightStick3D.y *= -1.0f; // Y軸を反転
    return mRightStick3D;
}

void Ecse::System::Gamepad::ImGuiUpdate()
{
    // スティックの可視化用設定
    const float radius = 40.0f;
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // 左スティックと右スティックを横に並べる
    ImGui::BeginGroup();
    {
        ImGui::Text("L-Stick");
        ImVec2 cursor = ImGui::GetCursorScreenPos();
        ImVec2 center(cursor.x + radius, cursor.y + radius);

        // 背景の円
        dl->AddCircle(center, radius, IM_COL32(200, 200, 200, 100), 32, 2.0f);
        // 入力位置の点 (Y軸は上がプラスなので、描画時はマイナスする)
        dl->AddCircleFilled(ImVec2(center.x + mLeftStick.x * radius, center.y - mLeftStick.y * radius), 5.0f, IM_COL32(255, 100, 100, 255));

        ImGui::Dummy(ImVec2(radius * 2, radius * 2));
        ImGui::Text("(%.2f, %.2f)", mLeftStick.x, mLeftStick.y);
    }
    ImGui::EndGroup();

    ImGui::SameLine(0, 20);

    ImGui::BeginGroup();
    {
        ImGui::Text("R-Stick");
        ImVec2 cursor = ImGui::GetCursorScreenPos();
        ImVec2 center(cursor.x + radius, cursor.y + radius);

        dl->AddCircle(center, radius, IM_COL32(200, 200, 200, 100), 32, 2.0f);
        dl->AddCircleFilled(ImVec2(center.x + mRightStick.x * radius, center.y - mRightStick.y * radius), 5.0f, IM_COL32(100, 255, 100, 255));

        ImGui::Dummy(ImVec2(radius * 2, radius * 2));
        ImGui::Text("(%.2f, %.2f)", mRightStick.x, mRightStick.y);
    }
    ImGui::EndGroup();

    // トリガー表示
    ImGui::Separator();
    ImGui::SliderFloat("L-Trigger", &mLeftTrigger, 0.0f, 1.0f, "%.2f");
    ImGui::SliderFloat("R-Trigger", &mRightTrigger, 0.0f, 1.0f, "%.2f");
}
