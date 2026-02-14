#pragma once

#include<memory>
#include<System/Service/ServiceProvider.hpp>
#include<ImGui/imgui.h>

namespace Ecse::Graphics
{
	class GDescriptorHeap;
}

namespace Ecse::Debug
{
	class ImGuiManager : public System::ServiceProvider<ImGuiManager>
	{
		ECSE_SERVICE_ACCESS(ImGuiManager);
	public:
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
        void Render();

        /// <summary>
        /// 解放
        /// </summary>
        void Shutdown();
	private:
        /// <summary>
        /// ImGui専用のディスクリプタを管理するオブジェクト
        /// </summary>
        std::unique_ptr<Graphics::GDescriptorHeap> mHeap;

        /// <summary>
        /// ImGuiのコンテキスト
        /// </summary>
        ImGuiContext* mContext = nullptr;

        /// <summary>
        /// 初期化しているかどうか
        /// </summary>
        bool mIsInitialized = false;
	};
}