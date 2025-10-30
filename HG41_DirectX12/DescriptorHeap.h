#ifndef __DESCRIPTOR_HEAP_H__
#define __DESCRIPTOR_HEAP_H__

#include "DirectX.h"

class DescriptorHeap
{
public:
	struct Description {
		D3D12_DESCRIPTOR_HEAP_TYPE heapType;
		UINT num;
	};
	struct Handle {
		D3D12_CPU_DESCRIPTOR_HANDLE hCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE hGPU;
	};

public:
	DescriptorHeap(Description desc);
	~DescriptorHeap();

	Handle Allocate();
	void Bind();
	ID3D12DescriptorHeap* Get();

private:
	ID3D12DescriptorHeap* m_pHeap;
	D3D12_DESCRIPTOR_HEAP_TYPE m_type;
	UINT m_allocateCount;
};

#endif // __DESCRIPTOR_HEAP_H__