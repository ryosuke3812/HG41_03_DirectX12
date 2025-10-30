#ifndef __DEPTH_STENCIL_H__
#define __DEPTH_STENCIL_H__

#include "DescriptorHeap.h"

class DepthStencil
{
public:
	struct Description
	{
		UINT width;
		UINT height;
		DescriptorHeap* pDSVHeap;
	};

public:
	DepthStencil(Description desc);
	~DepthStencil();

	void Clear();
	DescriptorHeap::Handle GetHandleDSV();

private:
	ID3D12Resource* m_pDepthStencil;
	DescriptorHeap::Handle m_hDSV;
};



#endif // __DEPTH_STENCIL_H__