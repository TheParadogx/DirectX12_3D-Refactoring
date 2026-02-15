#pragma once
#include<filesystem>

namespace Ecse::System
{
	class IResource
	{
	public:
		virtual ~IResource() = default;

		/// <summary>
		/// リソースの生成・読込
		/// </summary>
		/// <param name="FilePath">ファイルパス</param>
		/// <returns>true:成功</returns>
		virtual bool Create(const std::filesystem::path& FilePath) = 0;

	};
}