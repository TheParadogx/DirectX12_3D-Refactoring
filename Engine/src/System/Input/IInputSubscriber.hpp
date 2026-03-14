#pragma once
#include<Windows.h>

namespace Ecse::System
{
	class IInputSubscriber
	{
	public:
		virtual ~IInputSubscriber() = default;
		/// <summary>
		/// 入力イベントを受け取るための純粋仮想関数
		/// </summary>
		virtual bool OnInputEvent(UINT msg, WPARAM wp, LPARAM lp) = 0;
		 
		/// <summary>
		/// 前回状態の更新など、入力状態の更新を行うための純粋仮想関数
		/// </summary>
		virtual void Update() = 0;
	};
} // namespace Ecse::System