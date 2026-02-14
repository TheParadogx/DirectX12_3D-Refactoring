#pragma once

#include<memory>
#include<System/Service/ServiceProvider.hpp>
#include<ImGui/imgui.h>
#include<Utility/Export/Export.hpp>
#include<vector>
#include<functional>

namespace Ecse::Graphics
{
	class GDescriptorHeap;
}

namespace Ecse::Debug
{
	/// <summary>
	/// ImGuiの一括管理
	/// </summary>
	class ENGINE_API ImGuiManager : public System::ServiceProvider<ImGuiManager>
	{
		ECSE_SERVICE_ACCESS(ImGuiManager);
	public:
        ImGuiManager();
        virtual ~ImGuiManager();

        /// <summary>
        /// 初期化
        /// </summary>
        /// <returns>true:成功</returns>
        bool Initialize();

        /// <summary>
        ///フレーム開始 
        /// </summary>
        void NewFrame();

        /// <summary>
        /// フレームの終了・描画
        /// </summary>
        void EndFrame();

        /// <summary>
        /// 解放
        /// </summary>
        void Shutdown();

        /// <summary>
        /// 外部からデバッグUI関数を登録する
        /// </summary>
        /// <param name="guiFunc"></param>
        void AddDebugUI(std::function<void()> guiFunc);

	private:
        /// <summary>
        /// デバックUI用の処理を呼び出す関数群
        /// </summary>
        std::vector<std::function<void()>> mDebugUIFunctions;

        /// <summary>
        /// ImGui専用のディスクリプタを管理するオブジェクト
        /// </summary>
        std::unique_ptr<Graphics::GDescriptorHeap> mHeap;

        /// <summary>
        /// ImGuiのコンテキスト
        /// </summary>
        ImGuiContext* mContext;

        /// <summary>
        /// 初期化しているかどうか
        /// </summary>
        bool mIsInitialized;
	};
}