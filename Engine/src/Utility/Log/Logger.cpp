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
		std::cout << "Logger Online." << std::endl;
	}

	/// <summary>
	/// コンソールのリソース解放
	/// </summary>
	void Logger::OnDestroy()
	{
		std::cout << "Logger Shutdown." << std::endl;
		FreeConsole();
	}

	/// <summary>
	/// 色のセット
	/// </summary>
	void Logger::SetTextColor(ELogLevel Level)
	{
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		WORD color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; // デフォルトは白

		switch (Level) {
		case ELogLevel::Warning:
			color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY; // Yellow
			break;
		case ELogLevel::Error:
			color = FOREGROUND_RED | FOREGROUND_INTENSITY; // Red
			break;
		case ELogLevel::Fatal:
			color = BACKGROUND_RED | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
			break;
		}
		SetConsoleTextAttribute(hConsole, color);
	}

	/// <summary>
	/// ログの表示
	/// </summary>
	/// <param name="File">__FILE__</param>
	/// <param name="Line">__LINE__</param>
	/// <param name="Level">ログの出力レベル</param>
	/// <param name="Message">出力テキスト</param>
	void Logger::LogInternal(const char* File, int Line, ELogLevel Level, const std::string& Message)
	{
		//	Debug出力（vsの出力ウィンドウ）
		std::string debugMsg = std::format("[{}] {}\n", (int)Level, Message);
		OutputDebugStringA(debugMsg.c_str());
		
		//	エラー以外は色を変える
		if (Level != ELogLevel::Log) {
			SetTextColor(Level);
		}

		//	ログレベルのlabel
		std::string label = "";
		switch (Level) {
		case ELogLevel::Warning: 
			label = "[Warning] "; 
			break;

		case ELogLevel::Error:   
			label = "[Error]   "; 
			break;

		case ELogLevel::Fatal:   
			label = "[FATAL]   "; 
			break;
		default:                 
			label = "[Log]     "; 
			break;
		}

		// コンソールに表示
		std::cout << label << Message << std::endl;

		if (Level != ELogLevel::Log) {
			std::cout << "  -> " << File << "(" << Line<< ")" << std::endl;
			// 色を戻す
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		}

		//	Fatalの時は実行を止める+メッセージボックス
		if (Level == ELogLevel::Fatal) {
			std::string fatalDetail = std::format("{}\n\nLocation: {}({})", Message, File, Line);
			MessageBoxA(nullptr, fatalDetail.c_str(), "Fatal Error", MB_ICONERROR);
			DebugBreak();
		}
	}

}
