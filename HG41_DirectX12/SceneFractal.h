#ifndef __SCENE_FRACTAL_H__
#define __SCENE_FRACTAL_H__

#include "MeshBuffer.h"
#include "DescriptorHeap.h"
#include "ConstantBuffer.h"
#include "RootSignature.h"
#include "Pipeline.h"
#include "DepthStencil.h"
#include <vector>

class SceneFractal
{
public:
	HRESULT Init();
	void Uninit();
	void Draw();

private:
	void CalcTriangle(int depth, float x, float y, int* pIdx);
	void CalcCube(int depth, float x, float y, float z, int* pIdx);

private:
	MeshBuffer* m_pTriangle;
	MeshBuffer* m_pCube;
	DescriptorHeap* m_pShaderHeap;
	DescriptorHeap* m_pDSVHeap;
	std::vector<ConstantBuffer*> m_pWVP;
	RootSignature* m_pRootSignature;
	Pipeline* m_pPipeline;
	DepthStencil* m_pDSV;
	float m_rad;
};

#endif // __SCENE_FRACTAL_H__