#ifndef __MESH_BUFFER_H__
#define __MESH_BUFFER_H__

#include "DirectX.h"

class MeshBuffer
{
public:
	struct Description {
		const void* pVtx;
		UINT vtxSize;
		UINT vtxCount;
		const void* pIdx;
		DXGI_FORMAT idxSize;
		UINT idxCount;
		D3D12_PRIMITIVE_TOPOLOGY topology;
	};

public:
	MeshBuffer(Description desc);
	~MeshBuffer();
	void Draw();

private:
	Description m_desc;
	ID3D12Resource* m_pVtxBuf;
	D3D12_VERTEX_BUFFER_VIEW m_vbv;
	ID3D12Resource* m_pIdxBuf;
	D3D12_INDEX_BUFFER_VIEW m_ibv;
};

#endif // __MESH_BUFFER_H__