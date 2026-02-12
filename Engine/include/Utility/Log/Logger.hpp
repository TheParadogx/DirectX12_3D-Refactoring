#pragma once
#include<Utility/Export/Export.hpp>
#include<Utility/Singleton/Singleton.hpp>

#include<cstdint>
#include<cstdio>
#include<string>
#include<string_view>


namespace Ecse::Utility
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
	class ENGINE_API Logger : public Singleton<Logger>
	{
		GENERATE_SINGLETON_BODY(Logger);
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
		void Output(const char* File, int Line, ELogLevel Level, std::string_view Fmt, Args&&... args)
		{
			try
			{
				std::string message = std::vformat(Fmt, std::make_format_args(args...));
				LogInternal(File, Line, Level, message);
			}
			catch (const std::format_error& e)
			{
				LogInternal(File, Line, ELogLevel::Error, std::string("Format Error: ") + e.what());
			}
		}
	};

}

/*
* 呼び出しを楽にするためのマクロ
* Loggerが登録されているときだけ処理をする
*/
#ifdef _DEBUG
// デバッグビルド時：すべて出す
#define ECSE_LOG(level, fmt, ...) \
        if (auto* logger = ::Ecse::Utility::Logger::Get()) { \
            logger->Output(__FILE__, __LINE__, level, fmt __VA_OPT__(,) __VA_ARGS__); \
        }
#else
// リリースビルド時：Error と Fatal だけ出す（それ以外はコードごと消滅させる）
#define ECSE_LOG(level, fmt, ...) \
        if (level >= ::Ecse::Utility::ELogLevel::Error) { \
            if (auto* logger = ::Ecse::Utility::Logger::Get()) { \
                logger->Output(__FILE__, __LINE__, level, fmt __VA_OPT__(,) __VA_ARGS__); \
            } \
        }
#endif