#pragma once
#include<System/Input/IInputSubscriber.hpp>
#include<array>

#include "KeyCode.hpp"

namespace Ecse::System
{
	/// <summary>
	/// キーボード入力を管理するクラス
	/// </summary>
	class Keyboard : public IInputSubscriber
	{
		/// <summary>
		/// キーコードが範囲内か判定する
		/// </summary>
		/// <param name="keyCode"></param>
		/// <returns>true:範囲内</returns>
		bool IsValid(eKeyCode keyCode)const;

		/// <summary>
		/// Windowsの仮想キーコードをkeyboard::eKeyに変換する
		/// </summary>
		/// <param name="vkCode"></param>
		/// <returns></returns>
		eKeyCode ToKeyCode(WPARAM VKCode);

	public:
		Keyboard();
		virtual ~Keyboard() = default;

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
		bool IsPressed(eKeyCode keyCode)const;

		/// <summary>
		/// 押している間
		/// </summary>
		/// <param name="keyCode">キーコード</param>
		/// <returns></returns>
		bool IsHeld(eKeyCode keyCode)const;

		/// <summary>
		/// 離した瞬間
		/// </summary>
		/// <param name="keyCode">キーコード</param>
		/// <returns></returns>
		bool IsReleased(eKeyCode keyCode)const;

	private:
		/// <summary>
		/// 入力状態用
		/// </summary>
		using KeyStateArray = std::array<bool, static_cast<int>(eKeyCode::Count)>;
		KeyStateArray mCurrKeys;
		KeyStateArray mPrevKeys;

	};
} // namespace Ecse::System

