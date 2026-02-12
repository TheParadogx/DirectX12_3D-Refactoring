#pragma once

namespace Ecse::Utility
{
	/// <summary>
	/// サービスの基底クラス（コンセプト用）
	/// </summary>
	class IService
	{
	protected:
		IService() noexcept = default;
		virtual ~IService() noexcept = default;
	};
}