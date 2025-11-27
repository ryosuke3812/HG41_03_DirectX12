#ifndef __SCENE_PROCEDURAL_H__
#define __SCENE_PROCEDURAL_H__

#include "MeshBuffer.h"
#include "DescriptorHeap.h"
#include "ConstantBuffer.h"
#include "RootSignature.h"
#include "Pipeline.h"
#include "DepthStencil.h"
#include <vector>

class SceneProcedural
{
public:
	HRESULT Init();
	void Uninit();
	void Draw();

private:
	float m_time;
	float m_rad;
	bool m_isRotate;
	MeshBuffer* m_pSquare;
	MeshBuffer* m_pCube;
	DescriptorHeap* m_pShaderHeap;
	DescriptorHeap* m_pDSVHeap;
	std::vector<ConstantBuffer*> m_pWVP;
	RootSignature* m_pRootSignature;
	std::vector<Pipeline*> m_pPipeline;
	DepthStencil* m_pDSV;
};

#endif // __SCENE_PROCEDURAL_H__