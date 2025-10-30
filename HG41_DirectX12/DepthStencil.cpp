#include "DepthStencil.h"


DepthStencil::DepthStencil(Description desc)
	: m_pDepthStencil(nullptr)
	, m_hDSV{}
{
	// レンダーターゲットの作成
	D3D12_HEAP_PROPERTIES prop = {};
	prop.Type = D3D12_HEAP_TYPE_DEFAULT;
	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	prop.CreationNodeMask = 1;
	prop.VisibleNodeMask = 1;

	// リソース設定
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Format = DXGI_FORMAT_D32_FLOAT;	// 深度バッファフォーマット
	resDesc.Width = desc.width;
	resDesc.Height = desc.height;
	resDesc.DepthOrArraySize = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.MipLevels = 1;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // 深度バッファの指定

	// リソース作成
	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = resDesc.Format;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;
	HRESULT hr = GetDevice()->CreateCommittedResource(
		&prop, D3D12_HEAP_FLAG_NONE, &resDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(&m_pDepthStencil)
	);
	if (FAILED(hr)) {
		MessageBox(NULL, L"DepthStencil.cpp", L"Error", MB_OK);
		return;
	}


	// ビューの作成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = resDesc.Format;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	m_hDSV = desc.pDSVHeap->Allocate();
	GetDevice()->CreateDepthStencilView(m_pDepthStencil, &dsvDesc, m_hDSV.hCPU);
}
DepthStencil::~DepthStencil()
{
}

void DepthStencil::Clear()
{
	GetCommandList()->ClearDepthStencilView(m_hDSV.hCPU, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}
DescriptorHeap::Handle DepthStencil::GetHandleDSV()
{
	return m_hDSV;
}