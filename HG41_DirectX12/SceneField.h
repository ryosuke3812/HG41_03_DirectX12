#ifndef __SCENE_FIELD_H__
#define __SCENE_FIELD_H__

#include "MeshBuffer.h"
#include "DescriptorHeap.h"
#include "ConstantBuffer.h"
#include "RootSignature.h"
#include "Pipeline.h"
#include "DepthStencil.h"
#include <vector>

class SceneField
{
public:
	HRESULT Init();
	void Uninit();
	void Draw();

private:
	MeshBuffer* m_pPlane;
	MeshBuffer* m_pSphere;
	DescriptorHeap* m_pShaderHeap;
	DescriptorHeap* m_pDSVHeap;
	std::vector<ConstantBuffer*> m_pWVPs;
	std::vector<Pipeline*> m_pPipelines;
	DepthStencil* m_pDSV;

	RootSignature* m_pGroundRS;
	RootSignature* m_pWaterRS;
};

#endif // __SCENE_FIELD_H__