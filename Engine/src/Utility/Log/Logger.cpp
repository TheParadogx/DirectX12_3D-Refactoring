#include "pch.h"
#include "Utility/Log/Logger.hpp"


namespace Ecse::Utility
{
	static bool operator&(const EConsoleTextColor a, const EConsoleTextColor b)
	{
		return static_cast<uint8_t>(a) & static_cast<uint8_t>(b);
	}

	/// <summary>
	/// コンソールウィンドウの作成とそのコンソールに書き込めるようにターゲット
	/// </summary>
	void Logger::OnCreate()
	{
		if (AllocConsole() == true)
		{
			FILE* fp = nullptr;
			freopen_s(&fp, "CONOUT$", "w", stdout);
			freopen_s(&fp, "CONIN$", "r", stdin);

			//	日本語に対応させます。
			std::setlocale(LC_ALL, "japanese");
		}
		std::cout << "[System] Logger Online." << std::endl;
	}

	/// <summary>
	/// コンソールのリソース解放
	/// </summary>
	void Logger::OnDestroy()
	{
	}

}
