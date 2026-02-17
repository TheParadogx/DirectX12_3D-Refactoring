#pragma once
#include<Utility/Export/Export.hpp>

namespace Ecse::Graphics
{
	/// <summary>
	/// アクセスするための情報
	/// </summary>
	struct ENGINE_API GDescritorHeapInfo
	{
		int Index = -1;
		int Size = 0;

		bool IsValid() const noexcept { return Index >= 0 && Size > 0; }
	};
}