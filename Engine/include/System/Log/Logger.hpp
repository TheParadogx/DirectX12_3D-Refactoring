#pragma once
#include<Utility/Export/Export.hpp>
#include<System/Service/ServiceProvider.hpp>
#include<System/Service/ServiceLocator.hpp>

#include<cstdint>
#include<cstdio>
#include<string>
#include<string_view>
#include<mutex>
#include <source_location>

namespace Ecse::System
{
	/// <summary>
	/// Debug出力のレベル定義
	/// </summary>
	enum class ELogLevel : uint8_t
	{
		/// <summary>
		/// 通常ログ 
		/// 出力先:出力ログ
		/// </summary>
		Log,

		/// <summary>
		/// 警告ログ 
		/// 出力先:出力ログ, コンソール画面
		/// </summary>
		Warning,

		/// <summary>
		/// エラーログ 
		/// 出力先:出力ログ, コンソール画面
		/// </summary>
		Error,

		/// <summary>
		/// 致命的エラー(実行をその場で止める) 
		/// 出力先:出力ログ, コンソール画面
		/// </summary>
		Fatal
	};

	/// <summary>
	/// コンソールの文字の色変えるための定義
	/// </summary>
	enum class EConsoleTextColor : uint8_t
	{
		Red = 0x01,
		Blue = 0x02,
		Green = 0x04,
		Yellow = Red | Green,
		Purple = Red | Blue,
		Cyan = Blue | Green,
		White = Red | Blue | Green,
	};

	/// <summary>
	/// ログを表示するためのクラス
	/// </summary>
	class ENGINE_API Logger : public ServiceProvider<Logger>
	{
	protected:		
		/// <summary>
		/// コンソールウィンドウの作成とそのコンソールに書き込めるようにターゲット
		/// </summary>
		virtual void OnCreate()override;

		/// <summary>
		/// コンソールのリソース解放
		/// </summary>
		virtual void OnDestroy() override;

	private:
		/// <summary>
		/// 色のセット
		/// </summary>
		void SetTextColor(ELogLevel Level);
		/// <summary>
		/// ログの表示
		/// </summary>
		/// <param name="File">__FILE__</param>
		/// <param name="Line">__LINE__</param>
		/// <param name="Level">ログの出力レベル</param>
		/// <param name="Message">出力テキスト</param>
		void LogInternal(const char* File, int Line, ELogLevel Level, const std::string& Message);

	public:

		/// <summary>
		/// ログ表示用のAPI
		/// </summary>
		/// <param name="File">__FILE__</param>
		/// <param name="Line">__LINE__</param>
		/// <param name="Level">ログの出力レベル</param>
		/// <param name="...args">可変引数</param>
		template<typename... Args>
		void Output(ELogLevel Level, std::string_view Fmt, Args&&... args, const std::source_location Location = std::source_location::current())
		{
			try
			{
				std::string message = std::vformat(Fmt, std::make_format_args(args...));
				LogInternal(Location.file_name(), static_cast<int>(Location.line()), Level, message);
			}
			catch (const std::format_error& e)
			{
				LogInternal(Location.file_name(), static_cast<int>(Location.line()), ELogLevel::Error, std::string("Format Error: ") + e.what());
			}
		}

	private:
		static std::mutex sLogMutex;
	};

}

/*
* 呼び出しを楽にするためのマクロ
* Loggerが登録されているときだけ処理をする
* ifとelseの間にマクロを展開すると内側のif分に結びつく可能性があるのでdo whileにします。
*/
#ifndef ESCE_LOG
#ifdef _DEBUG
// デバッグビルド：全てを記録
#define ESCE_LOG(level, fmt, ...) \
        do { \
            if (auto* logger = ::Ecse::System::ServiceLocator::Get<::Ecse::System::Logger>()) { \
                logger->Output(level, fmt __VA_OPT__(,) __VA_ARGS__); \
            } \
        } while (0)
#else
#define ESCE_MIN_LOG_LEVEL ::Ecse::System::ELogLevel::Error

// リリースビルド：Error 以上だけをコンパイル対象にする
#define ESCE_LOG(level, fmt, ...) \
        do { \
            if constexpr (level >= ESCE_MIN_LOG_LEVEL ) { \
                if (auto* logger = ::Ecse::System::ServiceLocator::Get<::Ecse::System::Logger>()) { \
                    logger->Output(level, fmt __VA_OPT__(,) __VA_ARGS__); \
                } \
            } \
        } while (0)
#endif
#endif