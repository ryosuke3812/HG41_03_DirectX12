#include "SceneProcedural.h"
#include <functional>
#include <stack>
#include <string>
#include <map>
#include <vector>
#include <DirectXMath.h>

const int ObjNum = 7;

HRESULT SceneProcedural::Init()
{
	m_time = 0.0f;
	m_rad = 0.0f;
	m_isRotate = false;

	struct Vertex {
		float pos[3];
		float uv[2];
	};

	{	// 四角形の頂点バッファ作成
		Vertex vtx[] = {
			{{-0.5f,  0.5f, 0.0f}, {0,0} },
			{{ 0.5f,  0.5f, 0.0f}, {1,0} },
			{{-0.5f, -0.5f, 0.0f}, {0,1} },
			{{ 0.5f, -0.5f, 0.0f}, {1,1} },
		};
		MeshBuffer::Description desc = {};
		desc.pVtx = vtx;
		desc.vtxSize = sizeof(Vertex);
		desc.vtxCount = _countof(vtx);
		desc.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		m_pSquare = new MeshBuffer(desc);
	}

	{	// 立方体の頂点バッファ作成
		Vertex vtx[] = {
			// -Z
			{{-0.5f,  0.5f,-0.5f}, {0,0} },
			{{ 0.5f,  0.5f,-0.5f}, {1,0} },
			{{-0.5f, -0.5f,-0.5f}, {0,1} },
			{{ 0.5f, -0.5f,-0.5f}, {1,1} },
			// +X
			{{ 0.5f,  0.5f,-0.5f}, {0,0} },
			{{ 0.5f,  0.5f, 0.5f}, {1,0} },
			{{ 0.5f, -0.5f,-0.5f}, {0,1} },
			{{ 0.5f, -0.5f, 0.5f}, {1,1} },
			// +Z
			{{ 0.5f,  0.5f, 0.5f}, {0,0} },
			{{-0.5f,  0.5f, 0.5f}, {1,0} },
			{{ 0.5f, -0.5f, 0.5f}, {0,1} },
			{{-0.5f, -0.5f, 0.5f}, {1,1} },
			// -X
			{{-0.5f,  0.5f, 0.5f}, {0,0} },
			{{-0.5f,  0.5f,-0.5f}, {1,0} },
			{{-0.5f, -0.5f, 0.5f}, {0,1} },
			{{-0.5f, -0.5f,-0.5f}, {1,1} },
			// +Y
			{{-0.5f, 0.5f, 0.5f}, {0,0} },
			{{ 0.5f, 0.5f, 0.5f}, {1,0} },
			{{-0.5f, 0.5f,-0.5f}, {0,1} },
			{{ 0.5f, 0.5f,-0.5f}, {1,1} },
			// -Y
			{{-0.5f,-0.5f,-0.5f}, {0,0} },
			{{ 0.5f,-0.5f,-0.5f}, {1,0} },
			{{-0.5f,-0.5f, 0.5f}, {0,1} },
			{{ 0.5f,-0.5f, 0.5f}, {1,1} },
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

	// オブジェクト用ディスクリプターヒープ作成
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.num = ObjNum;
		m_pShaderHeap = new DescriptorHeap(desc);
	}
	// オブジェクト用の定数バッファ作成
	{
		ConstantBuffer::Description desc = {};
		desc.pHeap = m_pShaderHeap;
		desc.size = sizeof(DirectX::XMFLOAT4X4) * 3 + sizeof(DirectX::XMFLOAT4);
		for (int i = 0; i < ObjNum; ++i)
			m_pWVP.push_back(new ConstantBuffer(desc));
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
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT},
		};
		Pipeline::Description desc = {};
		desc.pInputLayout = layout;
		desc.InputLayoutNum = _countof(layout);
		desc.VSFile = L"VS_Procedural.cso";
		desc.PSFile = L"PS_Procedural.cso";
		desc.pRootSignature = m_pRootSignature->Get();
		desc.RenderTargetNum = 1;
		desc.EnableDepth = true;

		const wchar_t* File[] = {
			L"PS_WhiteNoise.cso",
			L"PS_BlockNoise.cso",
			L"PS_ValueNoise.cso",
			L"PS_PerlinNoise.cso",
			L"PS_CellularNoise.cso",
			L"PS_fBM.cso",
			L"PS_NoiseFire.cso",
		};
		for (int i = 0; i < ObjNum; ++i)
		{
			desc.PSFile = File[i];
			m_pPipeline.push_back(new Pipeline(desc));
		}
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
void SceneProcedural::Uninit()
{
	auto wvpIt = m_pWVP.begin();
	while (wvpIt != m_pWVP.end())
	{
		delete* wvpIt;
		++wvpIt;
	}
	auto pipelineIt = m_pPipeline.begin();
	while (pipelineIt != m_pPipeline.end())
	{
		delete* pipelineIt;
		++pipelineIt;
	}
}
void SceneProcedural::Draw()
{
	m_time += 1.0f / 60;
	if (GetAsyncKeyState(VK_RETURN) & 0x0001)
		m_isRotate ^= true;
	if (m_isRotate)
		m_rad += 0.01f;

	ID3D12GraphicsCommandList* pCmdList = GetCommandList();

	// レンダーターゲット設定
	D3D12_CPU_DESCRIPTOR_HANDLE hRTV[] = { GetRTV() };
	D3D12_CPU_DESCRIPTOR_HANDLE hDSV = m_pDSV->GetHandleDSV().hCPU;
	SetRenderTarget(_countof(hRTV), hRTV, hDSV);
	m_pDSV->Clear();

	// 表示領域の設定
	float width = 1280.0f;
	float height = 720.0f;
	D3D12_VIEWPORT vp = { 0, 0, width, height, 0.0f, 1.0f };
	D3D12_RECT scissor = { 0, 0, (LONG)width, (LONG)height };
	pCmdList->RSSetViewports(1, &vp);
	pCmdList->RSSetScissorRects(1, &scissor);

	// ヒープ設定
	m_pShaderHeap->Bind();

	// 変換行列の計算
	DirectX::XMMATRIX mat[3];
	mat[0] = DirectX::XMMatrixIdentity();
	mat[1] = DirectX::XMMatrixLookAtLH(
		DirectX::XMVectorSet(sinf(m_rad) * 5.0f, 0.0f, cosf(m_rad) * 5.0f, 0.0),
		DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0),
		DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0)
	);
	mat[2] = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(60.0f), 16.f / 9.f, 0.1f, 100.0f
	);

	// オブジェクト数分描画
	struct Param {
		DirectX::XMFLOAT4X4 mat[3];
		DirectX::XMFLOAT4 time;
	};
	Param param;
	DirectX::XMStoreFloat4x4(&param.mat[1], DirectX::XMMatrixTranspose(mat[1]));
	DirectX::XMStoreFloat4x4(&param.mat[2], DirectX::XMMatrixTranspose(mat[2]));
	param.time.x = m_time;
	for (int i = 0; i < ObjNum; ++i)
	{
		// 定数バッファへ設定
		mat[0] = DirectX::XMMatrixTranslation((i - ObjNum / 2) * 1.05f, 0.0f, 0.0f);
		DirectX::XMStoreFloat4x4(&param.mat[0], DirectX::XMMatrixTranspose(mat[0]));
		m_pWVP[i]->Write(&param);

		// 描画
		D3D12_GPU_DESCRIPTOR_HANDLE handle[] = {
			m_pWVP[i]->GetHandle().hGPU
		};
		m_pPipeline[i]->Bind();
		m_pRootSignature->Bind(handle, 1);
		m_pSquare->Draw();
	}
}