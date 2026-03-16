#pragma once
#include <Utility/Export/Export.hpp>
#include <System/Service/ServiceProvider.hpp>
#include <System/Input/Mouse/Mouse.h>
#include <System/Input/Keyboard/Keyboard.hpp>
#include <System/Input/Gamepad/GamepadManager.hpp>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace Ecse::System
{
    // ─────────────────────────────────────────────
    //  入力アクション定義
    // ─────────────────────────────────────────────

    struct InputAction
    {
        std::vector<eKeyCode>     Keys;         ///< キーボード割り当て
        std::vector<eMouseButton> MouseButtons; ///< マウスボタン割り当て
        std::vector<ePadButton>   PadButtons;   ///< ゲームパッドボタン割り当て
        // 将来拡張: 軸入力（Axis）を追加予定
    };

    // ─────────────────────────────────────────────
    //  InputManager
    // ─────────────────────────────────────────────

    class ENGINE_API InputManager : public System::ServiceProvider<InputManager>
    {
        ECSE_SERVICE_ACCESS(InputManager);

    public:
        InputManager();
        virtual ~InputManager();

        InputManager(const InputManager&) = delete;
        InputManager& operator=(const InputManager&) = delete;

        // ── フレーム処理 ──────────────────────────

        /// @brief Windows メッセージを各デバイスへ転送する
        /// @return 該当イベントを処理した場合 true
        bool ProcessMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

        /// @brief 全デバイスの状態更新（フレーム先頭で呼ぶ）
        void Update();

        // ── アクション管理 ─────────────────────────

        /// @brief アクションを登録する（同名が既存の場合は上書き）
        void CreateAction(std::string_view name, const InputAction& action);

        /// @brief いずれかのデバイスでアクションが押された瞬間か
        [[nodiscard]] bool IsActionPressed(std::string_view name) const;

        /// @brief いずれかのデバイスでアクションが押され続けているか
        [[nodiscard]] bool IsActionHeld(std::string_view name) const;

        /// @brief いずれかのデバイスでアクションが離された瞬間か
        [[nodiscard]] bool IsActionReleased(std::string_view name) const;

        // ── デバイスアクセサ ───────────────────────

        [[nodiscard]] Mouse* GetMouse()    const noexcept { return mMouse.get(); }
        [[nodiscard]] Keyboard* GetKeyboard() const noexcept { return mKeyboard.get(); }
        [[nodiscard]] GamepadManager* GetGamepad()  const noexcept { return mGamepadManager.get(); }

    private:
        // ── 透過的ハッシュ（Heterogeneous Lookup） ──
        // find() に string_view を渡しても string が生成されないようにする
        struct StringHash
        {
            using is_transparent = void;
            std::size_t operator()(std::string_view sv) const noexcept
            {
                return std::hash<std::string_view>{}(sv);
            }
        };

        // アクション状態の種類（QueryAction 内部用）
        enum class ActionState { Pressed, Held, Released };

        /// @brief 指定したアクション・状態の組み合わせを全デバイスに対して問い合わせる
        [[nodiscard]] bool QueryAction(std::string_view name, ActionState state) const;

    private:
        // StringHash + equal_to<> の組み合わせで透過的ルックアップが有効になる
        std::unordered_map<std::string, InputAction, StringHash, std::equal_to<>> mActions;

        std::unique_ptr<Mouse>          mMouse;
        std::unique_ptr<Keyboard>       mKeyboard;
        std::unique_ptr<GamepadManager> mGamepadManager;
    };

} // namespace Ecse::System