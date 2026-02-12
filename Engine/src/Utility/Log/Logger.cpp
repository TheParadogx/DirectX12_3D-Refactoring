#include "pch.h"
#include "Utility/Log/Logger.hpp"


namespace Ecse::Utility
{
	FILE* Logger::mConInFP = nullptr;
	FILE* Logger::mConOutFP = nullptr;

	static bool operator&(const EConsoleTextColor a, const EConsoleTextColor b)
	{
		return static_cast<uint8_t>(a) & static_cast<uint8_t>(b);
	}

	/// <summary>
	/// コンソールウィンドウの作成とそのコンソールに書き込めるようにターゲット
	/// </summary>
	void Logger::OnCreate()
	{
		AllocConsole();

	}

	/// <summary>
	/// コンソールのリソース解放
	/// </summary>
	void Logger::OnDestroy()
	{
	}

	/// <summary>
	/// コンソールへの出力
	/// 直接呼ばずに ECSE_LOG のマクロから呼ぶことを推奨
	/// </summary>
	/// <param name="Sorcefile">__FILE__</param>
	/// <param name="Line">__LINE__</param>
	/// <param name="Level">ログの出力レベル</param>
	/// <param name="Text">出力テキスト</param>
	/// <param name="">可変引数</param>
	void Logger::Output(const char* Sourcefile, int Line, ELogLevel Level, const char* Text, ...)
	{
	}
}
