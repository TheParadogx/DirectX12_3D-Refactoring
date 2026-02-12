#pragma once

namespace Ecse::System
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