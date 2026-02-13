#pragma once

#include<System/Service/ServiceProvider.hpp>
#include<Utility/Types/EcseTypes.hpp>

#include<array>

namespace Ecse::Graphics
{
	class DX12 : public System::ServiceProvider<DX12>
	{
		ECSE_SERVICE_ACCESS(DX12);
	public:

		/// <summary>
		/// バッファの数
		/// 今回はトリプルバッファで作成。２でもいい
		/// </summary>
		static constexpr int FRAME_COUNT = 3;
	private:

		struct FrameResrouce
		{
			/// <summary>
			/// コマンドリストが記録に使うメモリ領域。実行中はリセット不可（すると落ちる）
			/// </summary>
			CmdAlloc Allocator;
			/// <summary>
			/// 実際に色が書き込まれるテクスチャ本体
			/// </summary>
			Resource BackBuffer;
			/// <summary>
			/// このフレームがGPUで完了したかを確認するためのフェンス値
			/// </summary>
			UINT64 FenceValue = 0;
		};

		/// <summary>
		/// GPUとの通信窓口的な
		/// </summary>
		Device mDevice;
		/// <summary>
		/// スワップチェインやアダプタ作成とか
		/// </summary>
		Factory mFactory;
		/// <summary>
		/// フロントとバックのバッファ入れ替え
		/// </summary>
		SwapChain mSwapChain;
		/// <summary>
		/// GPUへの命令を書き込む記録帳的な
		/// </summary>
		CmdList mCmdList;
		/// <summary>
		/// 書き終えたコマンドリストをGPUへ送り出す
		/// </summary>
		CmdQueue mCmdQueue;

		/// <summary>
		/// 各フレームごとのリソース
		/// </summary>
		std::array<FrameResrouce, FRAME_COUNT> mFrames;

		/// <summary>
		/// 深度バッファリソース（描画時の前後関係の判定）
		/// </summary>
		Resource mDepthBuffer;
		/// <summary>
		/// RTV用ディスクリプタヒープ (バックバッファの設定情報を格納)
		/// </summary>
		Heap mRtvHeap;
		/// <summary>
		/// DSV用ディスクリプタヒープ (深度バッファの設定情報を格納)
		/// </summary>
		Heap mDsvHeap;
		/// <summary>
		/// フェンス（CPUとGPUの同期をとるため）
		/// </summary>
		Fence mFence;
		/// <summary>
		/// リソース漏れ検知
		/// </summary>
		DebugDevice mDebugDevice;

		/// <summary>
		/// GPU待ちイベントハンドル
		/// </summary>
		HANDLE mWaitForGPUEventHandle;
		/// <summary>
		/// 次にSignalする値
		/// </summary>
		UINT64 mNextFenceValue;
		/// <summary>
		/// 今のフレームのインデックス
		/// </summary>
		UINT mFrameIndex;


		// ここに背景色を追加

		/// <summary>
		/// フォーマット
		/// </summary>
		DXGI_FORMAT mFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	};

}


