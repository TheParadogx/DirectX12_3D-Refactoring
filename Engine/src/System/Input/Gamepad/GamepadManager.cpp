#include "pch.h"
#include "GamepadManager.hpp"
#include<winrt/Windows.Foundation.Collections.h>

namespace wWGI = winrt::Windows::Gaming::Input;

Ecse::System::GamepadManager::GamepadManager()
{
	auto initGamepads = wWGI::Gamepad::Gamepads();
    for (auto const& pad : initGamepads)
    {
        mGamepads.emplace_back(pad);
    }
    wWGI::Gamepad::GamepadAdded({ this, &GamepadManager::OnGamepadAdded });
    wWGI::Gamepad::GamepadRemoved({ this, &GamepadManager::OnGamepadRemoved });

    if (auto imgui = SYS::ServiceLocator::Get<Debug::ImGuiManager>())
    {
        imgui->AddDebugUI([this]() {
            if (ImGui::Begin("Gamepad Debugger"))
            {
                if (mGamepads.empty())
                {
                    ImGui::TextColored(ImVec4(1, 0, 0, 1), "No Gamepads Connected");
                }
                else
                {
                    for (size_t i = 0; i < mGamepads.size(); ++i)
                    {
                        std::string label = "Gamepad " + std::to_string(i + 1);
                        if (ImGui::CollapsingHeader(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            // 各Gamepadクラスの描画関数を呼ぶ
                            mGamepads[i].ImGuiUpdate();
                        }
                    }
                }
            }
            ImGui::End();
           });
    }
}

/// <summary>
/// 全ての接続済みPadの状態更新
/// </summary>
void Ecse::System::GamepadManager::Update()
{
    for (auto& pad : mGamepads)
    {
        pad.Update();
    }
}

/// <summary>
/// 指定したインデックスのPadを取得
/// </summary>
/// <param name="index">Pad1~4</param>
/// <returns>Padの参照（接続されていない場合はデフォルト状態のPadを返すか、別途IsConnectedで判定）</returns>
const Ecse::System::Gamepad& Ecse::System::GamepadManager::GetPad(ePadIndex index) const
{
    size_t idx = static_cast<size_t>(index);
    if (idx < mGamepads.size())
    {
        return mGamepads[idx];
    }
    return sNullPad;
}

/// <summary>
/// 指定したPadが接続されているか確認
/// </summary>
bool Ecse::System::GamepadManager::IsConnected(ePadIndex index) const
{
    return static_cast<size_t>(index) < mGamepads.size();
}

/// <summary>
/// 使用可能なPadのインデックス一覧を取得
/// </summary>
std::vector<Ecse::System::ePadIndex> Ecse::System::GamepadManager::GetAvailableIndices() const
{
    std::vector<ePadIndex> result;
    for (size_t i = 0; i < mGamepads.size() && i < static_cast<size_t>(ePadIndex::Count); ++i)
    {
        result.push_back(static_cast<ePadIndex>(i));
    }
    return result;
}

void Ecse::System::GamepadManager::OnGamepadAdded(winrt::Windows::Foundation::IInspectable const&, WgiGamepad const& pad)
{
    // 最大接続数（4）を超えていなければ、自作Gamepadとしてラップして追加
    if (mGamepads.size() < static_cast<size_t>(ePadIndex::Count))
    {
        mGamepads.emplace_back(pad);
    }
}

void Ecse::System::GamepadManager::OnGamepadRemoved(winrt::Windows::Foundation::IInspectable const&, WgiGamepad const& pad)
{
    // 抜かれたOS側のGamepadと一致する自作Gamepadを探して削除
    auto it = std::find_if(mGamepads.begin(), mGamepads.end(), [&](const Gamepad& g) {
        return g.IsSameDevice(pad);
        });

    if (it != mGamepads.end())
    {
        mGamepads.erase(it);
    }
}
