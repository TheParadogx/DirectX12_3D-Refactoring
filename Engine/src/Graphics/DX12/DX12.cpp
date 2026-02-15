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
		mMACmdAlloc = nullptr;
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
			frame.UploadPool.Reset();	//	プール
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

		mMACmdAlloc.Reset();

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
#if defined(_DEBUG) || ECSE_DEV_TOOL_ENABLED 
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
		if (InitializeDepthHeap(Width, Height) == false)
		{
			ECSE_LOG(System::ELogLevel::Fatal, "Failed InitializeDepthHeap.");
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
	/// 描画開始
	/// </summary>
	void DX12::BegineRendering()
	{
		//	次の描画するべきバックバッファのインデックス
		mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();

		//	次に使うフレームのリソースが解放されているかどうかを見る。ストール防止
		if (mFence->GetCompletedValue() < mFrames[mFrameIndex].FenceValue)
		{
			mFence->SetEventOnCompletion(mFrames[mFrameIndex].FenceValue, mWaitForGPUEventHandle);
			WaitForSingleObject(mWaitForGPUEventHandle, INFINITE);
		}
		//	コマンド記録の準備
		mFrames[mFrameIndex].Allocator->Reset();
		mCmdList->Reset(mFrames[mFrameIndex].Allocator.Get(), nullptr);

		//	リソースバリア
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = mFrames[mFrameIndex].BackBuffer.Get();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		mCmdList->ResourceBarrier(1, &barrier);

		//	レンダーターゲットの設定
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = mRtvHeap->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.ptr += mFrameIndex * mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = mDsvHeap->GetCPUDescriptorHandleForHeapStart();
		mCmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

		//	レンダーターゲットのクリア
		mCmdList->ClearRenderTargetView(rtvHandle, mColor.GetRawPointer(), 0, nullptr);
		mCmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	}

	/// <summary>
	/// 画面のフリップ
	/// </summary>
	void DX12::Flip()
	{
		//	リソースバリア
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = mFrames[mFrameIndex].BackBuffer.Get();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		mCmdList->ResourceBarrier(1, &barrier);

		//	命令の確定と送信
		mCmdList->Close();
		ID3D12CommandList* ppCommandLists[] = { mCmdList.Get() };
		mCmdQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		//	画面の切り替え
		mSwapChain->Present(1, 0);

		//	現在のフレームに完了すべきフェンス値を入れる
		mNextFenceValue++;
		mFrames[mFrameIndex].FenceValue = mNextFenceValue;
		mCmdQueue->Signal(mFence.Get(), mNextFenceValue);
	}

	/// <summary>
	/// GPUの処理待ち
	/// </summary>
	void DX12::WaitForGPU()
	{
		//	現在のフェンス値でSignalを送ってその値まで待機する
		mNextFenceValue++;
		if (FAILED(mCmdQueue->Signal(mFence.Get(), mNextFenceValue)))
		{
			return;
		}

		//	GPUがこの値になるまでCPUをブロック
		if (mFence->GetCompletedValue() < mNextFenceValue)
		{
			mFence->SetEventOnCompletion(mNextFenceValue, mWaitForGPUEventHandle);
			WaitForSingleObject(mWaitForGPUEventHandle, INFINITE);
		}

	}

	/// <summary>
	/// ビューポートとシザー矩形の設定
	/// </summary>
	void DX12::SetViewPort(float Width, float Height, float x, float y)
	{
		D3D12_VIEWPORT viewport = { x, y, Width, Height, 0.0f, 1.0f };
		D3D12_RECT scissor = { (LONG)x, (LONG)y, (LONG)(x + Width), (LONG)(y + Height) };

		mCmdList->RSSetViewports(1, &viewport);
		mCmdList->RSSetScissorRects(1, &scissor);
	}

	/// <summary>
	/// Dx12デバイスの取得
	/// </summary>
	/// <returns></returns>
	ID3D12Device* DX12::GetDevice()
	{
		return mDevice.Get();
	}

	/// <summary>
	/// Dx12コマンドリストの取得
	/// </summary>
	/// <returns></returns>
	ID3D12GraphicsCommandList* DX12::GetCommandList()
	{
		return mCmdList.Get();
	}

	/// <summary>
	/// Dx12コマンドアロケーターの取得
	/// </summary>
	/// <returns></returns>
	ID3D12CommandAllocator* DX12::GetCommandAllocator()
	{
		return mFrames[mFrameIndex].Allocator.Get();
	}

	/// <summary>
	/// Dx12コマンドキューの取得
	/// </summary>
	/// <returns></returns>
	ID3D12CommandQueue* DX12::GetCommandQueue()
	{
		return mCmdQueue.Get();
	}

	/// <summary>
	/// D3D12MAアロケーターの取得
	/// </summary>
	/// <returns></returns>
	D3D12MA::Allocator* DX12::GetMAAllocator()
	{
		return mMACmdAlloc.Get();
	}

	/// <summary>
	/// デバッグレイヤーの起動
	/// </summary>
	void DX12::DebugLayerOn()
	{
		// Debug5のインターフェースで起動
		Debug5 debugLayer = nullptr;

		HRESULT hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer));
		if (SUCCEEDED(hr))
		{
			debugLayer->EnableDebugLayer();
			debugLayer->SetEnableAutoName(TRUE);
			ECSE_LOG(System::ELogLevel::Log, "EnableDebugLayer(ID3D12Debug5).");
		}
		//	D3D12GetDebugInterfaceで失敗する可能性があるらしいので一応復旧処理も入れておきます。
		else
		{
			ComPtr<ID3D12Debug> debugBasic;
			hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debugBasic));
			if (SUCCEEDED(hr))
			{
				debugBasic->EnableDebugLayer();
				ECSE_LOG(System::ELogLevel::Log, "EnableDebugLayer(Basic).");
			}
		}

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

				// D3D12Maアロケーターの作成
				D3D12MA::ALLOCATOR_DESC allocatorDesc = {};
				allocatorDesc.pDevice = mDevice.Get();
				allocatorDesc.pAdapter = adapter.Get();

				hr = D3D12MA::CreateAllocator(&allocatorDesc, &mMACmdAlloc);
				if (FAILED(hr))
				{
					ECSE_LOG(System::ELogLevel::Fatal, "Failed D3D12MA::CreateAllocator.");
					return false;
				}

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

		//	デバイス作成時やリソース解放時の詳細なエラーチェックが可能にするための機能の有効化
#if defined(_DEBUG) || ECSE_DEV_TOOL_ENABLED 
		factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
		// CreateDXGIFactory2を使用することで、デバッグフラグを指定した生成が可能
		const HRESULT hr = CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&mFactory));
		if (FAILED(hr))
		{
			ECSE_LOG(System::ELogLevel::Fatal, "Failed CreateDXGIFactory2.");
			return false;
		}

		return true;
	}

	/// <summary>
	/// スワップチェインの初期化
	/// </summary>
	/// <param name="WindowHandle"></param>
	/// <param name="Width"></param>
	/// <param name="Height"></param>
	/// <returns>true:成功</returns>
	bool DX12::InitializeSwapChain(HWND WindowHandle, UINT Width, UINT Height)
	{
		HRESULT hr = S_OK;

		//	コマンドキューの作成
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.NodeMask = 0;
		hr = mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCmdQueue));
		if (FAILED(hr))
		{
			ECSE_LOG(System::ELogLevel::Fatal, "Failed CreateCommandQueue.");
			return false;
		}

		//	各フレームのコマンドアロケーターの作成
		for (int i = 0; i < FRAME_COUNT; i++)
		{
			hr = mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mFrames[i].Allocator));
			if (FAILED(hr))
			{
				ECSE_LOG(System::ELogLevel::Fatal, "Failed CreateCommandAllocator." + std::to_string(i));
				return false;
			}

			//	D3D12MA Poolの作成
			D3D12MA::POOL_DESC poolDecs = {};
			poolDecs.HeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
			poolDecs.Flags = D3D12MA::POOL_FLAG_ALGORITHM_LINEAR;
			hr = mMACmdAlloc->CreatePool(&poolDecs, &mFrames[i].UploadPool);
			if (FAILED(hr))
			{
				ECSE_LOG(System::ELogLevel::Fatal, "Failed Create D3D12MA Pool for Frame: " + std::to_string(i));
				return false;
			}
		}

		//	コマンドリストの作成
		hr = mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mFrames[0].Allocator.Get(), nullptr, IID_PPV_ARGS(&mCmdList));
		if (FAILED(hr))
		{
			ECSE_LOG(System::ELogLevel::Fatal, "Failed CreateCommandList.");
			return false;
		}

		// 最初はClose状態から。
		mCmdList->Close();

		//	スワップチェインの設定
		DXGI_SWAP_CHAIN_DESC1 scDesc = {};
		scDesc.Width = Width;
		scDesc.Height = Height;
		scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scDesc.Stereo = FALSE;
		scDesc.SampleDesc.Count = 1; // マルチサンプルはOFF
		scDesc.SampleDesc.Quality = 0;
		scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scDesc.BufferCount = FRAME_COUNT; // トリプルバッファ
		scDesc.Scaling = DXGI_SCALING_STRETCH;
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		//	スワップチェインの作成
		ComPtr<IDXGISwapChain1> swapChain1 = nullptr;
		hr = mFactory->CreateSwapChainForHwnd(
			mCmdQueue.Get(),
			WindowHandle,
			&scDesc,
			nullptr,
			nullptr,
			&swapChain1
		);

		if (FAILED(hr))
		{
			ECSE_LOG(System::ELogLevel::Fatal, "Failed CreateSwapChainForHwnd.");
			return false;
		}

		hr = swapChain1.As(&mSwapChain);
		if (FAILED(hr))
		{
			return false;
		}

		return true;
	}

	/// <summary>
	/// バックバッファ用ディスクリプタヒープの初期化
	/// </summary>
	/// <returns>true:成功</returns>
	bool DX12::InitializeBackBufferHeap()
	{
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.NumDescriptors = FRAME_COUNT;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtvHeapDesc.NodeMask = 0;

		//	RTV用のヒープ作成
		HRESULT hr = mDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mRtvHeap));
		if (FAILED(hr))
		{
			ECSE_LOG(System::ELogLevel::Fatal, "CreateDescriptorHeap (RTV)");
			return false;
		}

		//	ハンドルとインクリメントサイズの取得
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = mRtvHeap->GetCPUDescriptorHandleForHeapStart();
		const UINT rtvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		//	スワップチェインのバッファをフレーム構造体に紐づけ
		for (UINT i = 0; i < FRAME_COUNT; ++i)
		{

			// バックバッファリソースの取得
			hr = mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mFrames[i].BackBuffer));
			if (FAILED(hr))
			{
				ECSE_LOG(System::ELogLevel::Fatal, "Failed GetBackBuffer.");
				return false;
			}

			// RTVの作成
			mDevice->CreateRenderTargetView(mFrames[i].BackBuffer.Get(), nullptr, rtvHandle);

			// 次の枠へアドレスをずらす
			rtvHandle.ptr += rtvDescriptorSize;
		}

		return true;
	}

	/// <summary>
	/// 深度バッファ用ディスクリプタヒープの初期化
	/// </summary>
	/// <returns>true:成功</returns>
	bool DX12::InitializeDepthHeap(UINT Width, UINT Height)
	{

		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		dsvHeapDesc.NodeMask = 0;

		//	DSV用のヒープ作成
		HRESULT hr = mDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&mDsvHeap));
		if (FAILED(hr))
		{
			ECSE_LOG(System::ELogLevel::Fatal, "CreateDescriptorHeap (DSV)");
			return false;
		}

		//	震度バッファの設定
		D3D12_RESOURCE_DESC depthDesc = {};
		depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depthDesc.Width = static_cast<UINT64>(Width);
		depthDesc.Height = Height;
		depthDesc.DepthOrArraySize = 1;
		depthDesc.MipLevels = 1;
		depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthDesc.SampleDesc.Count = 1;
		depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		//	深度値用。GPU専用のVRAM上に確保
		D3D12_HEAP_PROPERTIES heapProp = {};
		heapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		//	クリア用の初期値
		D3D12_CLEAR_VALUE clearValue = {};
		clearValue.Format = DXGI_FORMAT_D32_FLOAT;
		clearValue.DepthStencil.Depth = 1.0f;

		//	深度バッファの作成
		hr = mDevice->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&depthDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&clearValue,
			IID_PPV_ARGS(&mDepthBuffer)
		);
		if (FAILED(hr))
		{
			ECSE_LOG(System::ELogLevel::Fatal, "Failed CreateDepthBuffer.");
			return false;
		}

		//	DSVの作成
		mDevice->CreateDepthStencilView(mDepthBuffer.Get(), nullptr, mDsvHeap->GetCPUDescriptorHandleForHeapStart());

		return true;
	}

	/// <summary>
	/// フェンスの初期化
	/// </summary>
	/// <returns>true:成功</returns>
	bool DX12::InitializeFence()
	{
		HRESULT hr = S_OK;

		//	フェンスオブジェクトの作成
		hr = mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence));
		if (FAILED(hr))
		{
			ECSE_LOG(System::ELogLevel::Fatal, "Failed CreateFence.");
			return false;
		}

		//	フェンス更新用のカウンタを初期化
		mNextFenceValue = 1;

		//	イベントハンドルの作成
		mWaitForGPUEventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (mWaitForGPUEventHandle == nullptr)
		{
			return false;
		}

		//	一応ここでも初期化
		for (UINT i = 0; i < FRAME_COUNT; ++i)
		{
			mFrames[i].FenceValue = 0;
		}

		return true;
	}


}
