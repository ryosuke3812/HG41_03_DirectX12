#include "ConstantBuffer.h"


ConstantBuffer::ConstantBuffer(Description desc)
	: m_handle{}, m_size(0), m_pBuf(nullptr), m_cbv{}, m_pPtr(nullptr)
{
	HRESULT hr;
	ID3D12Device* pDevice = GetDevice();

	// 定数バッファリソースのヒープ設定
	D3D12_HEAP_PROPERTIES prop = {};
	prop.Type = D3D12_HEAP_TYPE_UPLOAD;						// GPUへの転送に適したCPUアクセス
	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;	// typeがCUSTOM以外ならUNKNOWNを設定
	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;	// typeがCUSTOM以外ならUNKNOWNを設定
	prop.CreationNodeMask = 1;
	prop.VisibleNodeMask = 1;

	// 定数バッファリソースの設定
	D3D12_RESOURCE_DESC res = {};
	res.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;								// バッファーとして利用
	res.Width = (desc.size + 0xff) & ~0xff;	// 横詰めで頂点バッファを作成
	res.Height = 1;
	res.DepthOrArraySize = 1;
	res.MipLevels = 1;
	res.Format = DXGI_FORMAT_UNKNOWN;				// フォーマットはテクスチャのときだけ
	res.SampleDesc.Count = 1;						// 0を設定するとエラー
	res.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;	// 連続した行をメモリ内に連続して配置
	res.Flags = D3D12_RESOURCE_FLAG_NONE;

	// 定数バッファリソースの生成
	hr = pDevice->CreateCommittedResource(
		&prop, D3D12_HEAP_FLAG_NONE, &res,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_pBuf)
	);
	if (FAILED(hr)) {
		MessageBox(NULL, L"Constant Buffer.", L"Error", MB_OK);
		return;
	}

	// 定数バッファービューの設定
	m_cbv.BufferLocation = m_pBuf->GetGPUVirtualAddress();	// GPU上のバッファの仮想アドレス
	m_cbv.SizeInBytes = static_cast<UINT>(res.Width);
	m_handle = desc.pHeap->Allocate();
	pDevice->CreateConstantBufferView(&m_cbv, m_handle.hCPU);

	// マッピング
	hr = m_pBuf->Map(0, nullptr, reinterpret_cast<void**>(&m_pPtr));
	if (FAILED(hr)) { return; }
	ZeroMemory(m_pPtr, desc.size);
	m_size = desc.size;

}
ConstantBuffer::~ConstantBuffer()
{
	if (m_pBuf) m_pBuf->Release();
}

void ConstantBuffer::Write(const void* data)
{
	memcpy_s(m_pPtr, m_size, data, m_size);
}
DescriptorHeap::Handle ConstantBuffer::GetHandle()
{
	return m_handle;
}