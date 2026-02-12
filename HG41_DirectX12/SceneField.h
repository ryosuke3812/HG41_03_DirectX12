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
	// コンストラクタ
	SceneField();
	HRESULT Init();
	void Uninit();
	void Draw();

private:
	struct Vertex
	{
		float pos[3];
		float normal[3];
		float uv[2];
	};

	MeshBuffer* m_pPlane;
	MeshBuffer* m_pSphere;
	DescriptorHeap* m_pShaderHeap;
	DescriptorHeap* m_pDSVHeap;
	std::vector<ConstantBuffer*> m_pWVPs;
	std::vector<Pipeline*> m_pPipelines;
	DepthStencil* m_pDSV;

	RootSignature* m_pGroundRS;
	RootSignature* m_pWaterRS;

	// スカイスフィアの頂点バッファとルートシグネチャのメンバ変数
	//MeshBuffer* m_pSphere;
	RootSignature* m_pSkyRS;

};

#endif // __SCENE_FIELD_H__