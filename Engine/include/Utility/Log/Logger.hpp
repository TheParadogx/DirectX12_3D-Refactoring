#pragma once
#include<Utility/Export/Export.hpp>
#include<Utility/Singleton/Singleton.hpp>

#include<cstdint>
#include<cstdio>

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

	public:
		/// <summary>
		/// コンソールへの出力
		/// 直接呼ばずに ECSE_LOG のマクロから呼ぶことを推奨
		/// </summary>
		/// <param name="Sorcefile">__FILE__</param>
		/// <param name="Line">__LINE__</param>
		/// <param name="Level">ログの出力レベル</param>
		/// <param name="Text">出力テキスト</param>
		/// <param name="">可変引数</param>
		void Output(const char* Sourcefile, int Line, ELogLevel Level, const char* Text, ...);
	private:

		static FILE* mConOutFP;
		static FILE* mConInFP;

	};

}

/*
* 呼び出しを楽にするためのマクロ
* Loggerが登録されているときだけ処理をする
*/
#define ECSE_LOG(level, text, ...) \
    if (auto* logger = ::Ecse::Utility::Logger::Get()) { \
        logger->Output(__FILE__, __LINE__, level, text __VA_OPT__(,) __VA_ARGS__); \
    }