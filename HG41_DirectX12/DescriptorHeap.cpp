#include "DescriptorHeap.h"

DescriptorHeap::DescriptorHeap(Description desc)
	: m_allocateCount(0), m_type()
{
	// ディスクリプターヒープ作成
	D3D12_DESCRIPTOR_HEAP_DESC heap = {};
	m_type = heap.Type = desc.heapType;
	if(desc.heapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		heap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heap.NumDescriptors = desc.num;
	heap.NodeMask = 0;

	HRESULT hr;
	hr = GetDevice()->CreateDescriptorHeap(&heap, IID_PPV_ARGS(&m_pHeap));
	if (FAILED(hr)) {
		MessageBox(NULL, L"Descriptor Heap.", L"Error", MB_OK);
	}
	return;
}
DescriptorHeap::~DescriptorHeap()
{
	if (m_pHeap) m_pHeap->Release();
}

DescriptorHeap::Handle DescriptorHeap::Allocate()
{
	D3D12_CPU_DESCRIPTOR_HANDLE hCPU =
		m_pHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE hGPU =
		m_pHeap->GetGPUDescriptorHandleForHeapStart();
	UINT increment = GetDevice()->GetDescriptorHandleIncrementSize(m_type);
	increment *= m_allocateCount;
	hCPU.ptr += increment;
	hGPU.ptr += increment;
	++m_allocateCount;

	Handle handle;
	handle.hCPU = hCPU;
	handle.hGPU = hGPU;
	return handle;
}
void DescriptorHeap::Bind()
{
	GetCommandList()->SetDescriptorHeaps(1, &m_pHeap);
}
ID3D12DescriptorHeap* DescriptorHeap::Get()
{
	return m_pHeap;
}