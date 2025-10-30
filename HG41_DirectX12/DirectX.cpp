#include "DirectX.h"

// グローバル変数
ID3D12Device* g_pDevice;
IDXGIFactory6* g_pFactory;
ID3D12CommandAllocator* g_pCmdAllocator;
ID3D12GraphicsCommandList* g_pCmdList;
ID3D12CommandQueue* g_pCmdQueue;
IDXGISwapChain3* g_pSwapChain;
ID3D12DescriptorHeap* g_pRTVHeap;
ID3D12Resource** g_ppBackBuf;
UINT64 g_fenceLevel;
ID3D12Fence* g_pFence;
D3D12_VIEWPORT g_viewPort;
D3D12_RECT g_scissor;

HRESULT InitDirectX(HWND hWnd, UINT width, UINT height, bool fullscreen)
{
	HRESULT hr;

#ifdef _DEBUG
	ID3D12Debug* pDebug;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pDebug)))) {
		pDebug->EnableDebugLayer();
	}
#endif

	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
	};
	for (auto lv : featureLevels) {
		hr = D3D12CreateDevice(nullptr, lv, IID_PPV_ARGS(&g_pDevice));
		if (SUCCEEDED(hr)) { break; }
	}
	if (FAILED(hr)) { return hr; }

	// コマンドアロケーター
	D3D12_COMMAND_LIST_TYPE cmdListType = D3D12_COMMAND_LIST_TYPE_DIRECT;
	hr = g_pDevice->CreateCommandAllocator(cmdListType, IID_PPV_ARGS(&g_pCmdAllocator));
	if (FAILED(hr)) { return hr; }
	// コマンドリスト
	hr = g_pDevice->CreateCommandList(0, cmdListType, g_pCmdAllocator, nullptr, IID_PPV_ARGS(&g_pCmdList));
	if (FAILED(hr)) { return hr; }
	// コマンドキュー
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Type = cmdListType;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cmdQueueDesc.NodeMask = 0;
	hr = g_pDevice->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&g_pCmdQueue));
	if (FAILED(hr)) { return hr; }

	// スワップチェーン
#ifdef _DEBUG
	hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&g_pFactory));
#else
	hr = CreateDXGIFactory1(IID_PPV_ARGS(&g_pFactory));
#endif
	if (FAILED(hr)) { return hr; }

	DXGI_SWAP_CHAIN_DESC1 scDesc = {};
	scDesc.Width = width;
	scDesc.Height = height;
	scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.Stereo = false;
	scDesc.SampleDesc.Count = 1;
	scDesc.SampleDesc.Quality = 0;
	scDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	scDesc.BufferCount = 2;
	scDesc.Scaling = DXGI_SCALING_STRETCH;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	hr = g_pFactory->CreateSwapChainForHwnd(
		g_pCmdQueue, hWnd, &scDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(&g_pSwapChain)
	);
	if (FAILED(hr)) { return hr; }

	//--- レンダーターゲット
	// ディスクリプタヒープ
	D3D12_DESCRIPTOR_HEAP_DESC rtvDHDesc = {};
	rtvDHDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDHDesc.NumDescriptors = 2;
	rtvDHDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvDHDesc.NodeMask = 0;
	hr = g_pDevice->CreateDescriptorHeap(&rtvDHDesc, IID_PPV_ARGS(&g_pRTVHeap));
	if (FAILED(hr)) { return hr; }
	// レンダーターゲット
	g_ppBackBuf = new ID3D12Resource*[scDesc.BufferCount];
	D3D12_CPU_DESCRIPTOR_HANDLE handle = g_pRTVHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < scDesc.BufferCount; ++i)
	{
		hr = g_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&g_ppBackBuf[i]));
		g_pDevice->CreateRenderTargetView(g_ppBackBuf[i], nullptr, handle);
		handle.ptr += g_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	g_fenceLevel = 0;
	hr = g_pDevice->CreateFence(g_fenceLevel, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_pFence));
	if (FAILED(hr)) { return hr; }

	return S_OK;
}
void UninitDirectX()
{
	DXGI_SWAP_CHAIN_DESC1 scDesc;
	g_pSwapChain->GetDesc1(&scDesc);
	for (UINT i = 0; i < scDesc.BufferCount; ++i) {
		SAFE_RELEASE(g_ppBackBuf[i]);
	}
	delete[] g_ppBackBuf;
	g_ppBackBuf = nullptr;
	SAFE_RELEASE(g_pRTVHeap);
	SAFE_RELEASE(g_pCmdQueue);
	SAFE_RELEASE(g_pCmdList);
	SAFE_RELEASE(g_pCmdAllocator);
	SAFE_RELEASE(g_pDevice);
}
void DrawDirectX(void(func)(void), const float clearColor[4])
{
	// 描画準備
	UINT bbIdx = g_pSwapChain->GetCurrentBackBufferIndex();
	g_pCmdAllocator->Reset();
	g_pCmdList->Reset(g_pCmdAllocator, nullptr);

	// バックバッファをレンダーターゲットとして利用
	D3D12_RESOURCE_BARRIER barrierDesc = {};
	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrierDesc.Transition.pResource = g_ppBackBuf[bbIdx];
	barrierDesc.Transition.Subresource = 0;
	barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	g_pCmdList->ResourceBarrier(1, &barrierDesc);

	// レンダーターゲット設定
	D3D12_CPU_DESCRIPTOR_HANDLE hRTV;
	hRTV = g_pRTVHeap->GetCPUDescriptorHandleForHeapStart();
	hRTV.ptr += bbIdx * g_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	g_pCmdList->OMSetRenderTargets(1, &hRTV, false, nullptr);
	g_pCmdList->ClearRenderTargetView(hRTV, clearColor, 0, nullptr);

	// 描画
	ID3D12CommandList* pCmdList[] = { g_pCmdList };
	if (func) { func(); }


	// 画面出力準備
	barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	g_pCmdList->ResourceBarrier(1, &barrierDesc);

	// 描画命令発光停止
	g_pCmdList->Close();
	g_pCmdQueue->ExecuteCommandLists(1, pCmdList);

	// 画面出力
	g_pSwapChain->Present(1, 0);

	// コマンドリストの完了を✓
	g_pCmdQueue->Signal(g_pFence, ++g_fenceLevel);
	if (g_pFence->GetCompletedValue() != g_fenceLevel)
	{
		auto hWait = CreateEvent(nullptr, false, false, nullptr);
		g_pFence->SetEventOnCompletion(g_fenceLevel, hWait);
		WaitForSingleObject(hWait, INFINITE);
		CloseHandle(hWait);
	}
}

void SetRenderTarget(int num, D3D12_CPU_DESCRIPTOR_HANDLE* hRTVs)
{
	g_pCmdList->OMSetRenderTargets(num, hRTVs, FALSE, nullptr);
}
void SetRenderTarget(int num, D3D12_CPU_DESCRIPTOR_HANDLE* hRTVs, D3D12_CPU_DESCRIPTOR_HANDLE hDSV)
{
	g_pCmdList->OMSetRenderTargets(num, hRTVs, FALSE, hDSV.ptr ? &hDSV : nullptr);
}

ID3D12Device* GetDevice()
{
	return g_pDevice;
}
ID3D12GraphicsCommandList* GetCommandList()
{
	return g_pCmdList;
}
D3D12_CPU_DESCRIPTOR_HANDLE GetRTV()
{
	UINT bbIdx = g_pSwapChain->GetCurrentBackBufferIndex();
	D3D12_CPU_DESCRIPTOR_HANDLE hRTV;
	hRTV = g_pRTVHeap->GetCPUDescriptorHandleForHeapStart();
	hRTV.ptr += bbIdx * g_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	return hRTV;
}