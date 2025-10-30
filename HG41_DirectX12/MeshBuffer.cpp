#include "MeshBuffer.h"

MeshBuffer::MeshBuffer(Description desc)
	: m_desc(desc), m_pVtxBuf(nullptr), m_vbv{}, m_pIdxBuf(nullptr), m_ibv{}
{
	// ヒープの設定
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;						// GPUへの転送に適したCPUアクセス
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;	// typeがCUSTOM以外ならUNKNOWNを設定
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;	// typeがCUSTOM以外ならUNKNOWNを設定
	heapProp.CreationNodeMask = 1;
	heapProp.VisibleNodeMask = 1;

	// 頂点バッファリソースの設定
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;	// バッファーとして利用
	resDesc.Width = desc.vtxSize * desc.vtxCount;			// 横詰めで頂点バッファを作成
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_UNKNOWN;						// フォーマットはテクスチャのときだけ
	resDesc.SampleDesc.Count = 1;															// 0を設定するとエラー
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;	// 連続した行をメモリ内に連続して配置
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	// 頂点バッファリソースの生成
	HRESULT hr;
	hr = GetDevice()->CreateCommittedResource(
		&heapProp, D3D12_HEAP_FLAG_NONE, &resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_pVtxBuf)
	);
	if (FAILED(hr)) {
		MessageBox(NULL, L"MeshBuffer.cpp", L"Error", MB_OK);
		return;
	}

	// バッファの初期値設定
	void* pMap;
	hr = m_pVtxBuf->Map(0, nullptr, (void**)&pMap);
	if (SUCCEEDED(hr)) {
		memcpy_s(pMap, resDesc.Width, desc.pVtx, resDesc.Width);
	}
	m_pVtxBuf->Unmap(0, nullptr);

	// 頂点バッファビューの設定
	m_vbv.BufferLocation = m_pVtxBuf->GetGPUVirtualAddress();	// GPU上のバッファの仮想アドレス
	m_vbv.SizeInBytes = (UINT)resDesc.Width;				// バッファのサイズ
	m_vbv.StrideInBytes = desc.vtxSize;

	//--- インデックスのチェック
	if (desc.pIdx == nullptr) { return; }

	// リソース設定
	UINT idxSize = 0;
	switch (desc.idxSize) {
		case DXGI_FORMAT_R8_UINT: idxSize = 1; break;
		case DXGI_FORMAT_R16_UINT: idxSize = 2; break;
		case DXGI_FORMAT_R32_UINT: idxSize = 4; break;
	}
	resDesc.Width = idxSize * desc.idxCount;

	// リソースの生成
	hr = GetDevice()->CreateCommittedResource(
		&heapProp, D3D12_HEAP_FLAG_NONE, &resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_pIdxBuf)
	);
	if (FAILED(hr)) {
		MessageBox(NULL, L"MeshBuffer.cpp", L"Error", MB_OK);
		return;
	}

	// バッファの初期値設定
	hr = m_pIdxBuf->Map(0, nullptr, (void**)&pMap);
	if (SUCCEEDED(hr)) {
		memcpy_s(pMap, resDesc.Width, desc.pIdx, resDesc.Width);
	}
	m_pIdxBuf->Unmap(0, nullptr);

	// インデックスバッファビューの設定
	m_ibv.BufferLocation = m_pIdxBuf->GetGPUVirtualAddress();
	m_ibv.SizeInBytes = (UINT)resDesc.Width;
	m_ibv.Format = desc.idxSize;

}
MeshBuffer::~MeshBuffer()
{
	if (m_pIdxBuf)m_pIdxBuf->Release();
	if (m_pVtxBuf)m_pVtxBuf->Release();
}
void MeshBuffer::Draw()
{
	ID3D12GraphicsCommandList* pCommand = GetCommandList();
	pCommand->IASetPrimitiveTopology(m_desc.topology);
	pCommand->IASetVertexBuffers(0, 1, &m_vbv);
	if (m_pIdxBuf) {
		pCommand->IASetIndexBuffer(&m_ibv);
		pCommand->DrawIndexedInstanced(m_desc.idxCount, 1, 0, 0, 0);
	} else {
		pCommand->DrawInstanced(m_desc.vtxCount, 1, 0, 0);
	}
}