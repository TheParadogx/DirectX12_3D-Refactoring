#pragma once
#include<vector>
#include"Gamepad.hpp"

namespace Ecse::System
{
	class GamepadManager
	{
	public:
		GamepadManager();
		virtual ~GamepadManager() = default;

		// コピー禁止
		GamepadManager(const GamepadManager&) = delete;
		GamepadManager& operator=(const GamepadManager&) = delete;

		/// <summary>
		/// 全ての接続済みPadの状態更新
		/// </summary>
		void Update();

		/// <summary>
		/// 指定したインデックスのPadを取得
		/// </summary>
		/// <param name="index">Pad1~4</param>
		/// <returns>Padの参照（接続されていない場合はデフォルト状態のPadを返すか、別途IsConnectedで判定）</returns>
		const Gamepad& GetPad(ePadIndex index = ePadIndex::Pad1) const;

		/// <summary>
		/// 指定したPadが接続されているか確認
		/// </summary>
		bool IsConnected(ePadIndex index) const;

		/// <summary>
		/// 使用可能なPadのインデックス一覧を取得
		/// </summary>
		std::vector<ePadIndex> GetAvailableIndices() const;

	private:
		// WinRTのイベントハンドラ
		void OnGamepadAdded(winrt::Windows::Foundation::IInspectable const&, WgiGamepad const& pad);
		void OnGamepadRemoved(winrt::Windows::Foundation::IInspectable const&, WgiGamepad const& pad);

	private:
		// 接続されている実際のゲームパッドリスト
		std::vector<Gamepad> mGamepads;

		// 接続されていない時に返す「空のPad」
		static inline Gamepad sNullPad{ nullptr };
	};
}

