#include "SceneField.h"
#include <functional>
#include <stack>
#include <string>
#include <map>
#include <vector>
#include <DirectXMath.h>

const int MaxObjects = 2; // オブジェクト数
const int MaxConstBufNum = 2; // 定数バッファ

HRESULT SceneField::Init()
{
	// ～～ 頂点データ作成 ～～
	struct Vertex
	{
		float pos[3];
		float normal[3];
		float uv[2];
	};
	const float maxSize = 20.0f;
	const int GridNum = 500;
	const float planeSpace = maxSize / (GridNum - 1);
	// 頂点生成
	std::vector<Vertex> planeVtx;
	for (int j = 0; j < GridNum; ++j)
	{
		for (int i = 0; i < GridNum; ++i)
		{
			planeVtx.push_back(
				{
					{i * planeSpace - maxSize * 0.5f, 0.0f, j * planeSpace - maxSize * 0.5f},
					{0.0f, 1.0f, 0.0f},
					{ i / (GridNum - 1.0f), j / (GridNum - 1.0f) }
				}
			);
		}
	}
	// インデックス生成
	std::vector<DWORD> planeIdx;
	for (int j = 0; j < GridNum - 1; ++j)
	{
		for (int i = 0; i < GridNum - 1; ++i)
		{
			planeIdx.push_back(GridNum * j + i);
			planeIdx.push_back(GridNum * j + i + 1);
			planeIdx.push_back(GridNum * (j + 1) + i);
			planeIdx.push_back(GridNum * (j + 1) + i);
			planeIdx.push_back(GridNum * j + i + 1);
			planeIdx.push_back(GridNum * (j + 1) + i + 1);
		}
	}
	// ～～～　構築後、頂点データに基づいて頂点バッファを生成　～～～
	{	// 頂点バッファの生成
		MeshBuffer::Description desc = {};
		desc.pVtx = planeVtx.data();
		desc.vtxSize = sizeof(Vertex);
		desc.vtxCount = planeVtx.size();
		desc.pIdx = planeIdx.data();
		desc.idxSize = DXGI_FORMAT_R32_UINT;
		desc.idxCount = planeIdx.size();
		desc.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		m_pPlane = new MeshBuffer(desc);
	}


	// ～～ スカイスフィア用頂点データ作成 ～～
	const int SPHERE_HORIZONTAL = 17;
	const int SPHERE_VERTICAL = 9;
	const float SPHERE_SIZE = 10.0f;
	std::vector<Vertex> sphereVtx;
	for (int j = 0; j < SPHERE_VERTICAL; ++j) {
		float radY = DirectX::XMConvertToRadians(180.0f * j / (SPHERE_VERTICAL - 1));
		float sinY = sinf(radY);
		float cosY = cosf(radY);
		for (int i = 0; i < SPHERE_HORIZONTAL; ++i) {
			float rad = DirectX::XMConvertToRadians(360.0f * i / (SPHERE_HORIZONTAL - 1));
			sphereVtx.push_back({
			  {sinY * sinf(rad) * SPHERE_SIZE, cosY * SPHERE_SIZE, sinY * cosf(rad) * SPHERE_SIZE },
			  {sinY * sinf(rad), cosY, sinY * cosf(rad) },
			  {i / (SPHERE_HORIZONTAL - 1.0f), j / (SPHERE_VERTICAL - 1.0f)} });
		}
	}
	// インデックス
	std::vector<unsigned long> sphereIdx;
	for (int j = 0; j < SPHERE_VERTICAL - 1; ++j) {
		for (int i = 0; i < SPHERE_HORIZONTAL - 1; ++i) {
			sphereIdx.push_back((j + 1) * SPHERE_HORIZONTAL + i);
			sphereIdx.push_back(j * SPHERE_HORIZONTAL + i);
			sphereIdx.push_back(j * SPHERE_HORIZONTAL + i + 1);
			sphereIdx.push_back(j * SPHERE_HORIZONTAL + i + 1);
			sphereIdx.push_back((j + 1) * SPHERE_HORIZONTAL + i + 1);
			sphereIdx.push_back((j + 1) * SPHERE_HORIZONTAL + i);
		}
	}
	{	// 頂点バッファの生成
		MeshBuffer::Description desc = {};
		desc.pVtx = sphereVtx.data();
		desc.vtxCount = sphereVtx.size();
		desc.vtxSize = sizeof(Vertex);
		desc.pIdx = sphereIdx.data();
		desc.idxCount = sphereIdx.size();
		desc.idxSize = DXGI_FORMAT_R32_UINT;
		desc.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		m_pSphere = new MeshBuffer(desc);
	}


	{	// オブジェクト用ディスクリプターヒープ作成
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.num = MaxConstBufNum;
		m_pShaderHeap = new DescriptorHeap(desc);
	}
	{	// オブジェクト用の定数バッファ作成
		ConstantBuffer::Description desc = {};
		desc.pHeap = m_pShaderHeap;
		desc.size = sizeof(DirectX::XMFLOAT4X4) * 3;
		m_pWVPs.resize(MaxConstBufNum);
		// 変換行列
		m_pWVPs[0] = new ConstantBuffer(desc);
		// カメラ、時間(水面用)
		desc.size = sizeof(DirectX::XMFLOAT4X4);
		m_pWVPs[1] = new ConstantBuffer(desc);
	}
	{	// ルートシグネチャ生成
		RootSignature::Parameter param[] = {
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
		};
		RootSignature::Description desc = {};
		desc.pParam = param;
		desc.paramNum = _countof(param);
		m_pGroundRS = new RootSignature(desc);
	}

	{	// ルートシグネチャ生成(水面)
		RootSignature::Parameter param[] = {
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, D3D12_SHADER_VISIBILITY_PIXEL},
		};
		RootSignature::Description desc = {};
		desc.pParam = param;
		desc.paramNum = _countof(param);
		m_pWaterRS = new RootSignature(desc);
	}
	{	// パイプライン生成
		Pipeline::InputLayout layout[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT},
		};
		Pipeline::Description desc = {};
		desc.pInputLayout = layout;
		desc.InputLayoutNum = _countof(layout);
		desc.RenderTargetNum = 1;
		desc.EnableDepth = TRUE;
		m_pPipelines.resize(MaxObjects);
		// フィールド
		desc.pRootSignature = m_pGroundRS->Get();
		desc.VSFile = L"VS_Ground.cso";
		desc.PSFile = L"PS_Ground.cso";
		m_pPipelines[0] = new Pipeline(desc);
		// 水面
		desc.pRootSignature = m_pWaterRS->Get();
		desc.VSFile = L"VS_Water.cso";
		desc.PSFile = L"PS_Water.cso";
		m_pPipelines[1] = new Pipeline(desc);
	}
	{	// DSV用のディスクリプター作成
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		desc.num = 1;
		m_pDSVHeap = new DescriptorHeap(desc);
	}
	{	// 深度バッファ作成
		DepthStencil::Description desc = {};
		desc.width = 1280;
		desc.height = 720;
		desc.pDSVHeap = m_pDSVHeap;
		m_pDSV = new DepthStencil(desc);
	}
	return S_OK;
}
void SceneField::Uninit()
{
	delete m_pPlane;
	delete m_pGroundRS;
	delete m_pDSVHeap;
	delete m_pDSV;
}
void SceneField::Draw()
{
	// 描画準備
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

	m_pShaderHeap->Bind();

	// カメラ位置
	static float rad = 0.0f;
	rad += 0.005f;
	DirectX::XMFLOAT3 camPos = { -10.0, 5.0f, -10 };

	// 変換行列作成
	DirectX::XMFLOAT4X4 fMat[3];
	DirectX::XMMATRIX mat[3];
	mat[0] = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	mat[1] = DirectX::XMMatrixLookAtLH(
		DirectX::XMVectorSet(camPos.x, camPos.y, camPos.z, 0.0),
		DirectX::XMVectorSet(10.0f, 0.0f, 10.0f, 0.0),
		DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0)
	);
	mat[2] = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(60.0f), 16.f / 9.f, 0.1f, 500.0f
	);
	for (int i = 0; i < 3; ++i) {
		DirectX::XMStoreFloat4x4(&fMat[i], DirectX::XMMatrixTranspose(mat[i]));
	}
	m_pWVPs[0]->Write(&fMat);

	// カメラ位置や時間の定数バッファ
	static float time = 0.0f;
	DirectX::XMFLOAT4 param = {
		camPos.x, camPos.y, camPos.z, time
	};
	time += 1.0f / 60.0f;
	m_pWVPs[1]->Write(&param);

	// 地形
	m_pPipelines[0]->Bind();
	D3D12_GPU_DESCRIPTOR_HANDLE handle[] = {
		m_pWVPs[0]->GetHandle().hGPU
	};
	m_pGroundRS->Bind(handle, 1);
	m_pPlane->Draw();

	// 水面
	m_pPipelines[1]->Bind();
	D3D12_GPU_DESCRIPTOR_HANDLE hWater[] = {
		m_pWVPs[0]->GetHandle().hGPU,
		m_pWVPs[1]->GetHandle().hGPU
	};
	m_pWaterRS->Bind(hWater, 2);
	m_pPlane->Draw();
}