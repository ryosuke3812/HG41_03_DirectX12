#ifndef __CONSTANT_BUFFER_H__
#define __CONSTANT_BUFFER_H__

#include "DescriptorHeap.h"

class ConstantBuffer
{
public:
	struct Description
	{
		DescriptorHeap* pHeap;
		UINT size;
	};

public:
	ConstantBuffer(Description desc);
	~ConstantBuffer();

	void Write(const void* data);
	DescriptorHeap::Handle GetHandle();

private:
	DescriptorHeap::Handle m_handle;
	UINT m_size;
	ID3D12Resource* m_pBuf;
	D3D12_CONSTANT_BUFFER_VIEW_DESC m_cbv;
	void* m_pPtr;
};

#endif // __CONSTANT_BUFFER_H__