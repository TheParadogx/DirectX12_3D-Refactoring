#include "pch.h"
#include "Mouse.h"


namespace Ecse::System
{
	Mouse::Mouse()
	{
	}

	/// <summary>
	/// 入力イベントの取得
	/// </summary>
	/// <returns>true:該当イベント</returns>
	bool Mouse::OnInputEvent(UINT msg, WPARAM wp, LPARAM lp)
	{
		return false;
	}

	/// <summary>
	/// 押した瞬間、離した瞬間の判定のために、前回状態の更新などを行う
	/// </summary>
	void Mouse::Update()
	{

	}
}
