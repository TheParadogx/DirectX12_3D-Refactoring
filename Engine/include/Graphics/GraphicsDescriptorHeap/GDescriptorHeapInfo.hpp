#pragma once

namespace Ecse::Graphics
{
	/// <summary>
	/// アクセスするための情報
	/// </summary>
	struct GDescritorHeapInfo
	{
		int Index = -1;
		int Size = 0;

		bool IsValid() const noexcept { return Index >= 0 && Size > 0; }
	};
}