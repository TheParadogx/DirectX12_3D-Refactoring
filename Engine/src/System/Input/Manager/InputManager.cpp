#include "pch.h"
#include "InputManager.hpp"

namespace Ecse::System
{

    // ─────────────────────────────────────────────
    //  構築 / 破棄
    // ─────────────────────────────────────────────

    InputManager::InputManager()
        : mKeyboard{ std::make_unique<Keyboard>() }
        , mMouse{ std::make_unique<Mouse>() }
        , mGamepadManager{ std::make_unique<GamepadManager>() }
    {
        mMouse->RegisterDebugUI();
    }

    InputManager::~InputManager() = default;

    // ─────────────────────────────────────────────
    //  フレーム処理
    // ─────────────────────────────────────────────

    bool InputManager::ProcessMessage(HWND /*hwnd*/, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        return mKeyboard->OnInputEvent(msg, wparam, lparam)
            || mMouse->OnInputEvent(msg, wparam, lparam);
    }

    void InputManager::Update()
    {
        mKeyboard->Update();
        mMouse->Update();
        mGamepadManager->Update();
    }

    // ─────────────────────────────────────────────
    //  アクション管理
    // ─────────────────────────────────────────────

    void InputManager::CreateAction(std::string_view name, const InputAction& action)
    {
        // 登録時だけ string を生成（マップのキーとして所有権ごと保存するため必要）
        mActions.insert_or_assign(std::string(name), action);
    }

    bool InputManager::IsActionPressed(std::string_view name) const { return QueryAction(name, ActionState::Pressed); }
    bool InputManager::IsActionHeld(std::string_view name) const { return QueryAction(name, ActionState::Held); }
    bool InputManager::IsActionReleased(std::string_view name) const { return QueryAction(name, ActionState::Released); }

    // ─────────────────────────────────────────────
    //  private: アクション問い合わせ共通処理
    // ─────────────────────────────────────────────

    bool InputManager::QueryAction(std::string_view name, ActionState state) const
    {
        // 透過的ルックアップにより string_view のまま検索（string 生成コストなし）
        const auto it = mActions.find(name);
        if (it == mActions.end()) return false;

        const InputAction& action = it->second;

        // デバイスと状態の組み合わせをテンプレートラムダで汎用化
        const auto checkAny = [&]<typename TDevice, typename TButton>
            (const TDevice * device, const std::vector<TButton>&buttons) -> bool
        {
            return std::ranges::any_of(buttons, [&](const TButton& btn)
                {
                    switch (state)
                    {
                    case ActionState::Pressed:  return device->IsPressed(btn);
                    case ActionState::Held:     return device->IsHeld(btn);
                    case ActionState::Released: return device->IsReleased(btn);
                    default:                    return false;
                    }
                });
        };

        if (checkAny(mKeyboard.get(), action.Keys))         return true;
        if (checkAny(mMouse.get(), action.MouseButtons)) return true;
        if (checkAny(&mGamepadManager->GetPad(), action.PadButtons))   return true;

        return false;
    }

} // namespace Ecse::System