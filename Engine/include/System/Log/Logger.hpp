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
	enum class ENGINE_API eLogLevel : uint8_t
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
	enum class ENGINE_API EConsoleTextColor : uint8_t
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
		ECSE_SERVICE_ACCESS(Logger);
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
		void SetTextColor(eLogLevel Level);
		/// <summary>
		/// ログの表示
		/// </summary>
		/// <param name="File">__FILE__</param>
		/// <param name="Line">__LINE__</param>
		/// <param name="Level">ログの出力レベル</param>
		/// <param name="Message">出力テキスト</param>
		void LogInternal(const char* File, int Line, eLogLevel Level, const std::string& Message);

	public:

		/// <summary>
		/// ログ表示用のAPI
		/// </summary>
		/// <param name="File">__FILE__</param>
		/// <param name="Line">__LINE__</param>
		/// <param name="Level">ログの出力レベル</param>
		/// <param name="...args">可変引数</param>
		template<typename... Args>
		void Output(eLogLevel Level, std::string_view Fmt, const std::source_location Location, Args&&... args)
		{
			try
			{
				std::string message = std::vformat(Fmt, std::make_format_args(std::forward<Args>(args)...));				
				LogInternal(Location.file_name(), static_cast<int>(Location.line()), Level, message);
			}
			catch (const std::format_error& e)
			{
				LogInternal(Location.file_name(), static_cast<int>(Location.line()), eLogLevel::Error, std::string("Format Error: ") + e.what());
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

#ifdef _DEBUG
#define ECSE_LOG_ENABLED(level) true
#else
// リリース時は Error 以上のみ有効
#define ECSE_LOG_ENABLED(level) (level >= ::Ecse::System::eLogLevel::Error)
#endif

#define ECSE_LOG(level, format, ...) \
    do { \
        if constexpr (ECSE_LOG_ENABLED(level)) { \
            if (auto* logger = ::Ecse::System::ServiceLocator::Get<::Ecse::System::Logger>()) { \
                logger->Output(level, format, std::source_location::current(), ##__VA_ARGS__); \
            } \
        } \
    } while (0)