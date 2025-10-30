#include "RenderTarget.h"


RenderTarget::RenderTarget(Description desc)
{
	// レンダーターゲットの作成
	D3D12_HEAP_PROPERTIES rtvProp = {};
	rtvProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	rtvProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	rtvProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	// リソース設定
	D3D12_RESOURCE_DESC rtvResourceDesc = {};
	rtvResourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvResourceDesc.Width = desc.width;
	rtvResourceDesc.Height = desc.height;
	rtvResourceDesc.DepthOrArraySize = 1;
	rtvResourceDesc.SampleDesc.Count = 1;
	rtvResourceDesc.SampleDesc.Quality = 0;
	rtvResourceDesc.MipLevels = 1;
	rtvResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	rtvResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	rtvResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	// リソース作成
	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = rtvResourceDesc.Format;
	clearValue.Color[0] = clearValue.Color[1] = clearValue.Color[2] = clearValue.Color[3] = 0.0f;
	HRESULT hr = GetDevice()->CreateCommittedResource(
		&rtvProp, D3D12_HEAP_FLAG_NONE, &rtvResourceDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clearValue, IID_PPV_ARGS(&m_pRenderTarget)
	);
	if (FAILED(hr)) {
		MessageBox(NULL, L"RenderTarget.cpp", L"Error", MB_OK);
		return;
	}

	// ビューの作成
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = rtvResourceDesc.Format;
	m_hRTV = desc.pRTVHeap->Allocate();
	GetDevice()->CreateRenderTargetView(m_pRenderTarget, &rtvDesc, m_hRTV.hCPU);

	// シェーダーリソースビュー
	D3D12_SHADER_RESOURCE_VIEW_DESC rtvsrvDesc = {};
	rtvsrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	rtvsrvDesc.Format = rtvDesc.Format;
	rtvsrvDesc.Texture2D.MipLevels = 1;
	rtvsrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	m_hSRV = desc.pSRVHeap->Allocate();
	GetDevice()->CreateShaderResourceView(m_pRenderTarget, &rtvsrvDesc, m_hSRV.hCPU);

}
RenderTarget::~RenderTarget()
{
	if (m_pRenderTarget) m_pRenderTarget->Release();
}

void RenderTarget::ResourceBarrier(D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
	D3D12_RESOURCE_BARRIER barrierDesc = {};
	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrierDesc.Transition.pResource = m_pRenderTarget;
	barrierDesc.Transition.Subresource = 0;
	barrierDesc.Transition.StateBefore = before;
	barrierDesc.Transition.StateAfter = after;
	GetCommandList()->ResourceBarrier(1, &barrierDesc);
}

void RenderTarget::Clear()
{
	const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	GetCommandList()->ClearRenderTargetView(m_hRTV.hCPU, clearColor, 0, nullptr);
}
DescriptorHeap::Handle RenderTarget::GetHandleRTV()
{
	return m_hRTV;
}

DescriptorHeap::Handle RenderTarget::GetHandleSRV()
{
	return m_hSRV;
}