#include "pch.h"
#include<Graphics/DX12/DX12.hpp>
#include<System/EngineConfig.hpp>

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

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="WindowHandle">ウィンドウのハンドル</param>
	/// <param name="Width">スクリーン横幅</param>
	/// <param name="Height">スクリーン縦幅</param>
	/// <returns>true:成功</returns>
	bool DX12::Initialize(HWND WindowHandle, UINT Width, UINT Height)
	{
#if defined(_DEBUG) || defined(ECSE_DEV_TOOL_ENABLED)
		//	デバック時だけリソース検知などを有効に
		DebugLayerOn();
#endif

		//	ファクトリー初期化
		if (InitializeFactory() == false)
		{
			ECSE_LOG(System::ELogLevel::Fatal, "Failed InitializeFactory.");
			return false;
		}

		//	デバイス初期化
		if (InitializeDevice() == false)
		{
			ECSE_LOG(System::ELogLevel::Fatal, "Failed InitializeDevice.");
			return false;

		}

		//	スワップチェイン初期化
		if (InitializeSwapChain(WindowHandle, Width, Height) == false)
		{
			ECSE_LOG(System::ELogLevel::Fatal, "Failed InitializeSwapChain.");
			return false;

		}

		//	バックバッファ用ヒープ初期化
		if (InitializeBackBufferHeap() == false)
		{
			ECSE_LOG(System::ELogLevel::Fatal, "Failed InitializeBackBufferHeap.");
			return false;

		}

		//	深度バッファ用ヒープ初期化
		if (InitializeDepthHeap() == false)
		{
			ECSE_LOG(System::ELogLevel::Fatal, "Failed InitializeDepthHeap.");
			return false;

		}

		//	レンダーターゲット初期化
		if (InitializeRenderTarget(Width, Height) == false)
		{
			ECSE_LOG(System::ELogLevel::Fatal, "Failed InitializeRenderTarget.");
			return false;

		}

		//	フェンスの初期化
		if (InitializeFence() == false)
		{
			ECSE_LOG(System::ELogLevel::Fatal, "Failed InitializeFence.");
			return false;

		}
		return true;

	}

	/// <summary>
	/// デバイスの初期化
	/// </summary>
	/// <returns>true:成功</returns>
	bool DX12::InitializeDevice()
	{
		// IDXGIFactory7の機能を使って、処理能力が高い順にGPUをリストアップ
		Adapter adapter;
		for (UINT i = 0; mFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND; i++)
		{
			DXGI_ADAPTER_DESC3 desc;
			adapter->GetDesc3(&desc);

			//	ソフトウェアレンダラは対象外（Microsoft Basic Render Driverなど）
			if (desc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE) continue;

			// デバイスの作成を試みる
			// 最新の機能を使いたいので、D3D_FEATURE_LEVEL_12_1 以上
			HRESULT hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&mDevice));

			if (SUCCEEDED(hr))
			{
				ECSE_LOG(System::ELogLevel::Log, "Success CreateDevice.");
				break;
			}
		}

		// デバイスが作れなかったら終了
		if (mDevice == nullptr)
		{
			ECSE_LOG(System::ELogLevel::Fatal, "Failed CreateDevice. No compatible GPU found.");
			return false;
		}

		//	Debugデバイスの取得
#if defined(_DEBUG) || ECSE_DEV_TOOL_ENABLED
		if (FAILED(mDevice.As(&mDebugDevice)))
		{
			ECSE_LOG(System::ELogLevel::Warning, "DirectX12: Failed to get DebugDevice interface.");
		}
#endif
		return true;
	}

	/// <summary>
	/// ファクトリーの初期化
	/// </summary>
	/// <returns>true:成功</returns>
	bool DX12::InitializeFactory()
	{
		UINT factoryFlags = 0;

#if defined(_DEBUG) || defined(ECSE_DEV_TOOL_ENABLED)
		factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
		const HRESULT hr = CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&mFactory));
		if (FAILED(hr))
		{
			ECSE_LOG(System::ELogLevel::Fatal, "DirectX12: Failed to create DXGI Factory. HRESULT: 0x{:X}", (uint32_t)hr);
			return false;
		}

		return true;
	}

}
