#include "pch.h"
#include<Graphics/DX12/DX12.hpp>


namespace Ecse::Graphics
{
	/// <summary>
	/// 初期化（実質コンストラクタ）
	/// </summary>
	void DX12::OnCreate()
	{
		mDevice = nullptr;
		mFactory = nullptr;
		mSwapChain = nullptr;
		mCmdList = nullptr;
		mCmdQueue = nullptr;
		mDepthBuffer = nullptr;
		mRtvHeap  = nullptr;
		mDsvHeap = nullptr;
		mFence = nullptr;
		mDebugDevice = nullptr;
		mWaitForGPUEventHandle = nullptr;
		mNextFenceValue = 1;
		mFrameIndex = 0;
		mColor = Color::Gray;
		mFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	}

	/// <summary>
	/// 終了処理（実質デストラクタ）
	/// </summary>
	void DX12::OnDestroy()
	{
		//	Gpuの処理待ち
		this->WaitForGPU();

		//	ウィンドウハンドルを閉じる
		if (mWaitForGPUEventHandle != nullptr)
		{
			CloseHandle(mWaitForGPUEventHandle);
		}

		//	フレームごとのインスタンス破棄
		for (auto& frame : mFrames)
		{
			frame.BackBuffer.Reset();  // バックバッファ
			frame.Allocator.Reset();   // アロケータ
		}

		//	ビュー・ヒープ
		mDepthBuffer.Reset();
		mRtvHeap.Reset();
		mDsvHeap.Reset();

		//	コマンド・描画基盤
		mCmdList.Reset();
		mSwapChain.Reset();    // Queueより先に消す
		mCmdQueue.Reset();

		//	同期・ファクトリ
		mFence.Reset();
		mFactory.Reset();

		//	デバイスとメモリリーク
		if (mDebugDevice != nullptr)
		{
			mDebugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
			mDebugDevice.Reset();
		}
		mDevice.Reset();

		ECSE_LOG(System::ELogLevel::Log, "DX12: OnDestroy completed.");
	}

}
