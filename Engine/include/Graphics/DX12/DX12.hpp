#pragma once

#include<Utility/Export/Export.hpp>
#include<System/Service/ServiceProvider.hpp>
#include<Utility/Types/EcseTypes.hpp>
#include<Graphics/Color/Color.hpp>

#include<array>

namespace Ecse::Graphics
{

	/// <summary>
	/// DX12でのレンダリングまでの基盤部分の一括管理
	/// </summary>
	class ENGINE_API DX12 : public System::ServiceProvider<DX12>
	{
		ECSE_SERVICE_ACCESS(DX12);

	protected:
		/// <summary>
		/// 初期化（実質コンストラクタ）
		/// </summary>
		void OnCreate()override;

		/// <summary>
		/// 終了処理（実質デストラクタ）
		/// </summary>
		void OnDestroy()override;

	public:

		/// <summary>
		/// 初期化
		/// </summary>
		/// <param name="WindowHandle">ウィンドウのハンドル</param>
		/// <param name="Width">スクリーン横幅</param>
		/// <param name="Height">スクリーン縦幅</param>
		/// <returns>true:成功</returns>
		bool Initialize(HWND WindowHandle, UINT Width, UINT Height);

		/// <summary>
		/// 描画開始
		/// </summary>
		void BegineRendering();

		/// <summary>
		/// 画面のフリップ
		/// </summary>
		void Flip();

		/// <summary>
		/// GPUの処理待ち
		/// </summary>
		void WaitForGPU();

		/// <summary>
		/// ビューポートとシザー矩形の設定
		/// </summary>
		void SetViewPort(float Width, float Height, float x = 0.0f, float y = 0.0f);

		/// <summary>
		/// Dx12デバイスの取得
		/// </summary>
		/// <returns></returns>
		ID3D12Device* GetDevice();

		/// <summary>
		/// Dx12コマンドリストの取得
		/// </summary>
		/// <returns></returns>
		ID3D12GraphicsCommandList* GetCommandList();

		/// <summary>
		/// Dx12コマンドアロケーターの取得
		/// </summary>
		/// <returns></returns>
		ID3D12CommandAllocator* GetCommandAllocator();

		/// <summary>
		/// Dx12コマンドキューの取得
		/// </summary>
		/// <returns></returns>
		ID3D12CommandQueue* GetCommandQueue();

	private:
		/// <summary>
		/// デバッグレイヤーの起動
		/// </summary>
		void DebugLayerOn();

		/// <summary>
		/// デバイスの初期化
		/// </summary>
		/// <returns>true:成功</returns>
		bool InitializeDevice();

		/// <summary>
		/// ファクトリーの初期化
		/// </summary>
		/// <returns>true:成功</returns>
		bool InitializeFactory();

		/// <summary>
		/// スワップチェインの初期化
		/// </summary>
		/// <param name="WindowHandle"></param>
		/// <param name="Width"></param>
		/// <param name="Height"></param>
		/// <returns>true:成功</returns>
		bool InitializeSwapChain(HWND WindowHandle, UINT Width, UINT Height);

		/// <summary>
		/// バックバッファ用ディスクリプタヒープの初期化
		/// </summary>
		/// <returns>true:成功</returns>
		bool InitializeBackBufferHeap();

		/// <summary>
		/// 深度バッファ用ディスクリプタヒープの初期化
		/// </summary>
		/// <returns>true:成功</returns>
		bool InitializeDepthHeap(UINT Width, UINT Height);

		/// <summary>
		/// フェンスの初期化
		/// </summary>
		/// <returns>true:成功</returns>
		bool InitializeFence();

	private:
		/// <summary>
		/// バッファの数
		/// 今回はトリプルバッファで作成。２でもいい
		/// </summary>
		static constexpr int FRAME_COUNT = 3;

		/// <summary>
		/// 各フレームごとに必要な情報をまとめた構造体
		/// </summary>
		struct FrameResrouce
		{
			/// <summary>
			/// コマンドリストが記録に使うメモリ領域。実行中はリセット不可（すると落ちる）
			/// </summary>
			CmdAlloc Allocator = nullptr;
			/// <summary>
			/// 実際に色が書き込まれるテクスチャ本体
			/// </summary>
			Resource BackBuffer = nullptr;
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

		/// <summary>
		/// 背景色
		/// </summary>
		Color mColor;

		/// <summary>
		/// フォーマット
		/// </summary>
		DXGI_FORMAT mFormat;

	};

}


