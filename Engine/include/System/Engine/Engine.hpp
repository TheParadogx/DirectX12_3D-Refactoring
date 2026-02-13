#pragma once

#include<Utility/Export/Export.hpp>
#include<System/Service/ServiceProvider.hpp>

namespace Ecse::System
{
	class Window;
	struct EngineContext;

	/// <summary>
	/// エンジン全体の管理クラス
	/// </summary>
	class ENGINE_API Engine :public ServiceProvider<Engine>
	{
		ECSE_SERVICE_ACCESS(Engine);

	protected:
		void OnCreate()override;
	public:
		/// <summary>
		/// エンジン全体の初期化
		/// OSやGraphicsなど全て
		/// </summary>
		/// <param name="Context">初期化に必要な情報</param>
		/// <returns>true:成功</returns>
		bool Initialize(const EngineContext& Context);

		/// <summary>
		/// メインループ
		/// </summary>
		/// <returns>true:続行 false:終了</returns>
		bool Run();

		/// <summary>
		/// エンジンの終了処理
		/// </summary>
		void Shutdown();

	private:
		Window* mpWindow;

		/// <summary>
		/// 初期化を複数回通さないためのフラグ
		/// </summary>
		bool mIsInitialized;

	};
}

