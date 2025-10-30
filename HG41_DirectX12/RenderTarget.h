#ifndef __RENDER_TARGET_H__
#define __RENDER_TARGET_H__

#include "DescriptorHeap.h"

class RenderTarget
{
public:
	struct Description
	{
		UINT width;
		UINT height;
		DescriptorHeap* pRTVHeap;
		DescriptorHeap* pSRVHeap;
	};
public:
	RenderTarget(Description desc);
	~RenderTarget();

	void ResourceBarrier(D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);
	void Clear();
	DescriptorHeap::Handle GetHandleRTV();
	DescriptorHeap::Handle GetHandleSRV();

private:
	ID3D12Resource* m_pRenderTarget;
	DescriptorHeap::Handle m_hRTV;
	DescriptorHeap::Handle m_hSRV;
};

#endif // __RENDER_TARGET_H__