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
	//int heapNum = MaxDepthに応じた三角形と四角形に必要なヒープ数を計算;
	int heapNum = (int)(powf(3.0f, MaxDepth) + powf(20.0f, MaxDepth));
	// オブジェクト用ディスクリプターヒープ作成
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.num = heapNum;
		m_pShaderHeap = new DescriptorHeap(desc);
	}

	// オブジェクト用の定数バッファ作成
	{
		ConstantBuffer::Description desc = {};
		desc.pHeap = m_pShaderHeap;
		desc.size = sizeof(DirectX::XMFLOAT4X4);
		m_pWVP.resize(heapNum);
		for (int i = 0; i < heapNum; ++i)
			m_pWVP[i] = new ConstantBuffer(desc);
	}

	// ルートシグネチャ生成
	{
		RootSignature::Parameter param[] = {
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
		};
		RootSignature::Description desc = {};
		desc.pParam = param;
		desc.paramNum = _countof(param);
		m_pRootSignature = new RootSignature(desc);
	}

	// パイプライン生成
	{
		Pipeline::InputLayout layout[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT},
		};
		Pipeline::Description desc = {};
		desc.pInputLayout = layout;
		desc.InputLayoutNum = _countof(layout);
		desc.VSFile = L"VS_FractalObject.cso";
		desc.PSFile = L"PS_FractalObject.cso";
		desc.pRootSignature = m_pRootSignature->Get();
		desc.RenderTargetNum = 1;
		desc.EnableDepth = true;
		m_pPipeline = new Pipeline(desc);
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
	ID3D12GraphicsCommandList* pCmdList = GetCommandList();
	D3D12_CPU_DESCRIPTOR_HANDLE hRTV[] = { GetRTV() };
	auto hDSV = m_pDSV->GetHandleDSV().hCPU;
	SetRenderTarget(_countof(hRTV), hRTV, hDSV);
	m_pDSV->Clear();

	// 表示領域の設定
	float width = 1280.0f;
	float height = 720.0f;
	D3D12_VIEWPORT vp = { 0, 0, width, height, 0.0f, 1.0f };
	D3D12_RECT scissor = { 0, 0, (LONG)width, (LONG)height };
	pCmdList->RSSetViewports(1, &vp);
	pCmdList->RSSetScissorRects(1, &scissor);

	// パイプライン、ヒープの設定
	m_pPipeline->Bind();
	m_pShaderHeap->Bind();

	// 定数バッファを割り当てて描画
	D3D12_GPU_DESCRIPTOR_HANDLE handle;
	for (int i = 0; i < m_pWVP.size(); ++i) {
		handle = m_pWVP[i]->GetHandle().hGPU;
		m_pRootSignature->Bind(&handle, 1);
		// 定数バッファに三角形、四角形が両方格納されているので、三角形の表示個数を元に描画判定
		if (i < powf(3.0f, MaxDepth))
			m_pTriangle->Draw();
		else
			m_pCube->Draw();
	}

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
		// 再帰の度合いに応じて移動距離が変化するので、MaxDepthやdepthを組み合わせて移動距離を計算
		// 1回目の再帰処理は1/4,2回目の再帰処理は1/8,3回目の再帰処理は1/16…
		float dist = 0.5f * powf(0.5f, (MaxDepth - depth));
		CalcTriangle(depth - 1, x + 0.0f, y + dist, pIdx); // 上に移動
		CalcTriangle(depth - 1, x - dist, y - dist, pIdx); // 左下に移動
		CalcTriangle(depth - 1, x + dist, y - dist, pIdx); // 右下に移動

	}
	else
	{
		// 描画に必要な行列を計算
		//float s = 描画する三角形の大きさをMaxDepthを使用して計算（MaxDepth = 1なら1 / 2, MaxDepth = 2なら1 / 4…）;
		float s = 1.0f * powf(0.5f, MaxDepth);
		DirectX::XMMATRIX W = DirectX::XMMatrixScaling(s, s, 1.0f) * DirectX::XMMatrixTranslation(x, y, 0.0f);
		DirectX::XMMATRIX V = DirectX::XMMatrixLookAtLH(
			DirectX::XMVectorSet(0.0f, 0.0f, -2.0f, 1.0f),
			DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
			DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
		//DirectX::XMMATRIX P = プロジェクション行列;
		DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(
			DirectX::XMConvertToRadians(60.0f),
			1280.0f / 720.0f,
			0.1f,
			100.0f);

		// 書き込み用のデータを計算
		DirectX::XMFLOAT4X4 fMat;
		DirectX::XMStoreFloat4x4(&fMat, DirectX::XMMatrixTranspose(W * V * P));
		// 定数バッファへ適宜書き込み
		m_pWVP[*pIdx]->Write(&fMat);
		*pIdx = (*pIdx) + 1;

	}
}

void SceneFractal::CalcCube(int depth, float x, float y, float z, int* pIdx)
{
	if (depth > 0)
	{
		// 穴あけ後の四角形の中心位置までの距離を計算
		// 3x3x3に分割した立方体のうち、表示が必要な箇所を1に変更する
		int idx[] = {
		0,0,0, 0,0,0, 0,0,0, // 上段
		0,0,0, 0,0,0, 0,0,0, // 中段
		0,0,0, 0,0,0, 0,0,0, // 下段
		};

		// 再帰の度合いに応じて移動距離が変化するので、MaxDepthやdepthを組み合わせて移動距離を計算
		// 1回目の再帰処理は1/3,2回目の再帰処理は1/9,3回目の再帰処理は1/27…
		float dist = 1.0f * powf(1.0f / 3.0f, (MaxDepth - depth));
		for (int i = 0; i < _countof(idx); ++i)
		{
			if (!idx[i]) { continue; }
			CalcCube(depth - 1,
				((i % 3) - 1) * dist + x, // iからX座標を計算
				((i % 9) / 3 - 1) * dist + y, // iからY座標を計算
				(i / 9 - 1) * dist + z, // iからZ座標を計算
				pIdx);
		}
	}
	else
	{
		// 描画に必要な行列を計算
		//float s = 描画する四角形の大きさをMaxDepthを使用して計算（MaxDepth = 1なら1 / 3, MaxDepth = 2なら1 / 9…）;
		float s = 1.0f * powf(1.0f / 3.0f, MaxDepth);
		DirectX::XMMATRIX W =
			DirectX::XMMatrixScaling(s, s, s) *
			DirectX::XMMatrixRotationY(m_rad) *
			DirectX::XMMatrixTranslation(x, y, z);
		DirectX::XMMATRIX V = DirectX::XMMatrixLookAtLH(
			DirectX::XMVectorSet(0.0f, 0.0f, -5.0f, 1.0f),
			DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
			DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
		DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(
			DirectX::XMConvertToRadians(60.0f),
			1280.0f / 720.0f,
			0.1f,
			100.0f);

		// 書き込み用のデータを計算
		DirectX::XMFLOAT4X4 fMat;
		DirectX::XMStoreFloat4x4(&fMat, DirectX::XMMatrixTranspose(W * V * P));
		// 定数バッファへ適宜書き込み
		m_pWVP[*pIdx]->Write(&fMat);
		*pIdx = (*pIdx) + 1;
	}
}



