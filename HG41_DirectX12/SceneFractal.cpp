#include "SceneFractal.h"
#include "Input.h"
#include "MeshBuffer.h"
#include <DirectXMath.h>

const int MaxDepth = 1;

HRESULT SceneFractal::Init()
{
	m_rad = 0.0f;

	{	// 三角形の頂点バッファ作成
		struct Vertex {
			float pos[3];
			float color[4];
		};
		Vertex vtx[] = {
			{{ 0.0f,  0.5f, 0.0f}, {1,1,1,1} },
			{{-0.5f, -0.5f, 0.0f}, {1,1,1,1} },
			{{ 0.5f, -0.5f, 0.0f}, {1,1,1,1} },
		};
		MeshBuffer::Description desc = {};
		desc.pVtx = vtx;
		desc.vtxSize = sizeof(Vertex);
		desc.vtxCount = _countof(vtx);
		desc.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		m_pTriangle = new MeshBuffer(desc);
	}

	{	// 四角形の頂点バッファ作成
		struct Vertex {
			float pos[3];
			float color[4];
		};
		Vertex vtx[] = {
			// -Z
			{{-0.5f,  0.5f,-0.5f}, {0.8f, 0.8f, 0.8f, 1} },
			{{ 0.5f,  0.5f,-0.5f}, {0.8f, 0.8f, 0.8f, 1} },
			{{-0.5f, -0.5f,-0.5f}, {0.8f, 0.8f, 0.8f, 1} },
			{{ 0.5f, -0.5f,-0.5f}, {0.8f, 0.8f, 0.8f, 1} },
			// +X
			{{ 0.5f,  0.5f,-0.5f}, {0.6f, 0.6f, 0.6f, 1} },
			{{ 0.5f,  0.5f, 0.5f}, {0.6f, 0.6f, 0.6f, 1} },
			{{ 0.5f, -0.5f,-0.5f}, {0.6f, 0.6f, 0.6f, 1} },
			{{ 0.5f, -0.5f, 0.5f}, {0.6f, 0.6f, 0.6f, 1} },
			// +Z
			{{ 0.5f,  0.5f, 0.5f}, {0.4f, 0.4f, 0.4f, 1} },
			{{-0.5f,  0.5f, 0.5f}, {0.4f, 0.4f, 0.4f, 1} },
			{{ 0.5f, -0.5f, 0.5f}, {0.4f, 0.4f, 0.4f, 1} },
			{{-0.5f, -0.5f, 0.5f}, {0.4f, 0.4f, 0.4f, 1} },
			// -X
			{{-0.5f,  0.5f, 0.5f}, {0.2f, 0.2f, 0.2f, 1} },
			{{-0.5f,  0.5f,-0.5f}, {0.2f, 0.2f, 0.2f, 1} },
			{{-0.5f, -0.5f, 0.5f}, {0.2f, 0.2f, 0.2f, 1} },
			{{-0.5f, -0.5f,-0.5f}, {0.2f, 0.2f, 0.2f, 1} },
			// +Y
			{{-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f, 1} },
			{{ 0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f, 1} },
			{{-0.5f, 0.5f,-0.5f}, {1.0f, 1.0f, 1.0f, 1} },
			{{ 0.5f, 0.5f,-0.5f}, {1.0f, 1.0f, 1.0f, 1} },
			// -Y
			{{-0.5f,-0.5f,-0.5f}, {0.0f, 0.0f, 0.0f, 1} },
			{{ 0.5f,-0.5f,-0.5f}, {0.0f, 0.0f, 0.0f, 1} },
			{{-0.5f,-0.5f, 0.5f}, {0.0f, 0.0f, 0.0f, 1} },
			{{ 0.5f,-0.5f, 0.5f}, {0.0f, 0.0f, 0.0f, 1} },
		};
		int idx[] = {
			 0, 1, 2,  1, 3, 2,
			 4, 5, 6,  5, 7, 6,
			 8, 9,10,  9,11,10,
			12,13,14, 13,15,14,
			16,17,18, 17,19,18,
			20,21,22, 21,23,22,
		};
		MeshBuffer::Description desc = {};
		desc.pVtx = vtx;
		desc.vtxSize = sizeof(Vertex);
		desc.vtxCount = _countof(vtx);
		desc.pIdx = idx;
		desc.idxSize = DXGI_FORMAT_R32_UINT;
		desc.idxCount = _countof(idx);
		desc.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		m_pCube = new MeshBuffer(desc);
	}

	// ヒープ数の計算
	int heapNum;
	// オブジェクト用ディスクリプターヒープ作成
	{

	}

	// オブジェクト用の定数バッファ作成
	{
	}

	// ルートシグネチャ生成
	{
	}

	// パイプライン生成
	{
	}

	// DSV用のディスクリプター作成
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		desc.num = 1;
		m_pDSVHeap = new DescriptorHeap(desc);
	}
	// 深度バッファ作成
	{
		DepthStencil::Description desc = {};
		desc.width = 1280;
		desc.height = 720;
		desc.pDSVHeap = m_pDSVHeap;
		m_pDSV = new DepthStencil(desc);
	}

	return S_OK;
}
void SceneFractal::Uninit()
{
	delete m_pDSV;
	delete m_pPipeline;
	delete m_pRootSignature;
	auto it = m_pWVP.begin();
	while (it != m_pWVP.end()) {
		delete (*it);
		++it;
	}
	delete m_pDSVHeap;
	delete m_pShaderHeap;
	delete m_pTriangle;
}
void SceneFractal::Draw()
{
	m_rad += 0.01f;

	// 事前に定数バッファへ変換行列を格納
	int idx = 0;
	CalcTriangle(MaxDepth, 0.0f, 0.0f, &idx);
	CalcCube(MaxDepth, 0.0f, 0.0f, 0.0f, &idx);

	// 描画先の設定

	// 表示領域の設定

	// パイプライン、ヒープの設定

	// 定数バッファを割り当てて描画
}

/*
* depth - 再帰処理の現在の深さ
* x - 再帰処理実行中の現在の参照座標X
* y - 再帰処理実行中の現在の参照座標Y
* pIdx - 現在格納している定数バッファ配列の添え字
*/
void SceneFractal::CalcTriangle(int depth, float x, float y, int* pIdx)
{
	if (depth > 0)
	{
		// 穴あけ後の三角形の中心位置までの距離を計算
	}
	else
	{
		// 描画に必要な行列を計算
	}
}

void SceneFractal::CalcCube(int depth, float x, float y, float z, int* pIdx)
{
	if (depth > 0)
	{
		// 穴あけ後の四角形の中心位置までの距離を計算
	}
	else
	{
		// 描画に必要な行列を計算
	}
}



