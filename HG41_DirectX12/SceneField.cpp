#include "SceneField.h"
#include <functional>
#include <stack>
#include <string>
#include <map>
#include <vector>
#include <DirectXMath.h>

const int MaxObjects = 3; // オブジェクト数 (地面、水面、大気)
const int MaxConstBufNum = 3; // 定数バッファ (地面・水面行列、パラメータ、大気行列)

SceneField::SceneField()
	: m_pPlane(nullptr),
	m_pSphere(nullptr),
	m_pShaderHeap(nullptr),
	m_pDSVHeap(nullptr),
	m_pDSV(nullptr),
	m_pGroundRS(nullptr),
	m_pWaterRS(nullptr),
	m_pSkyRS(nullptr)
{
}

HRESULT SceneField::Init()
{
	// ==========================================
	// 1. メッシュバッファの生成 (ジオメトリ)
	// ==========================================

	// ～～ 地面用頂点データ作成 ～～
	const float maxSize = 20.0f;
	const int GridNum = 500;
	const float planeSpace = maxSize / (GridNum - 1);
	std::vector<Vertex> planeVtx;
	for (int j = 0; j < GridNum; ++j) {
		for (int i = 0; i < GridNum; ++i) {
			planeVtx.push_back({
				{i * planeSpace - maxSize * 0.5f, 0.0f, j * planeSpace - maxSize * 0.5f},
				{0.0f, 1.0f, 0.0f},
				{ i / (GridNum - 1.0f), j / (GridNum - 1.0f) }
				});
		}
	}
	std::vector<DWORD> planeIdx;
	for (int j = 0; j < GridNum - 1; ++j) {
		for (int i = 0; i < GridNum - 1; ++i) {
			planeIdx.push_back(GridNum * j + i);
			planeIdx.push_back(GridNum * j + i + 1);
			planeIdx.push_back(GridNum * (j + 1) + i);
			planeIdx.push_back(GridNum * (j + 1) + i);
			planeIdx.push_back(GridNum * j + i + 1);
			planeIdx.push_back(GridNum * (j + 1) + i + 1);
		}
	}
	{
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
	{
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

	// ==========================================
	// 2. ディスクリプタヒープと定数バッファの生成
	// ==========================================
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.num = MaxConstBufNum;
		m_pShaderHeap = new DescriptorHeap(desc);
	}
	{
		ConstantBuffer::Description desc = {};
		desc.pHeap = m_pShaderHeap;

		// [0] 変換行列 (地面・水面)
		desc.size = sizeof(DirectX::XMFLOAT4X4) * 3;
		m_pWVPs.push_back(new ConstantBuffer(desc));

		// [1] カメラ、時間 (水面・大気パラメータ用)
		desc.size = sizeof(DirectX::XMFLOAT4X4);
		m_pWVPs.push_back(new ConstantBuffer(desc));

		// [2] 変換行列 (スカイスフィア用)
		desc.size = sizeof(DirectX::XMFLOAT4X4) * 3;
		m_pWVPs.push_back(new ConstantBuffer(desc));
	}

	// ==========================================
	// 3. ルートシグネチャの生成
	// ==========================================
	{	// 地面
		RootSignature::Parameter param[] = {
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
		};
		RootSignature::Description desc = {};
		desc.pParam = param;
		desc.paramNum = _countof(param);
		m_pGroundRS = new RootSignature(desc);
	}
	{	// 水面
		RootSignature::Parameter param[] = {
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, D3D12_SHADER_VISIBILITY_PIXEL},
		};
		RootSignature::Description desc = {};
		desc.pParam = param;
		desc.paramNum = _countof(param);
		m_pWaterRS = new RootSignature(desc);
	}
	{ 	// 大気 (スカイスフィア)
		RootSignature::Parameter param[] = {
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, D3D12_SHADER_VISIBILITY_PIXEL}, // 1に修正
		};
		RootSignature::Description desc = {};
		desc.pParam = param;
		desc.paramNum = _countof(param);
		m_pSkyRS = new RootSignature(desc);
	}

	// ==========================================
	// 4. パイプラインの生成
	// ==========================================
	{
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

		// [0] 地面
		desc.pRootSignature = m_pGroundRS->Get();
		desc.VSFile = L"VS_Ground.cso";
		desc.PSFile = L"PS_Ground.cso";
		m_pPipelines.push_back(new Pipeline(desc));

		// [1] 水面
		desc.pRootSignature = m_pWaterRS->Get();
		desc.VSFile = L"VS_Water.cso";
		desc.PSFile = L"PS_Water.cso";
		m_pPipelines.push_back(new Pipeline(desc));

		// [2] 大気
		desc.pRootSignature = m_pSkyRS->Get();
		desc.VSFile = L"VS_Atomosphere.cso";
		desc.PSFile = L"PS_Atomosphere.cso";
		m_pPipelines.push_back(new Pipeline(desc));
	}

	// ==========================================
	// 5. 深度バッファ生成
	// ==========================================
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		desc.num = 1;
		m_pDSVHeap = new DescriptorHeap(desc);
	}
	{
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
	// 作成したポインタをすべて解放する
	delete m_pPlane;
	delete m_pSphere;
	delete m_pGroundRS;
	delete m_pWaterRS;
	delete m_pSkyRS;
	delete m_pDSVHeap;
	delete m_pDSV;
	delete m_pShaderHeap;

	for (auto p : m_pWVPs) {
		delete p;
	}
	m_pWVPs.clear();

	for (auto p : m_pPipelines) {
		delete p;
	}
	m_pPipelines.clear();
}

void SceneField::Draw()
{
	ID3D12GraphicsCommandList* pCmdList = GetCommandList();
	D3D12_CPU_DESCRIPTOR_HANDLE hRTV[] = { GetRTV() };
	auto hDSV = m_pDSV->GetHandleDSV().hCPU;
	SetRenderTarget(_countof(hRTV), hRTV, hDSV);
	m_pDSV->Clear();

	float width = 1280.0f;
	float height = 720.0f;
	D3D12_VIEWPORT vp = { 0, 0, width, height, 0.0f, 1.0f };
	D3D12_RECT scissor = { 0, 0, (LONG)width, (LONG)height };
	pCmdList->RSSetViewports(1, &vp);
	pCmdList->RSSetScissorRects(1, &scissor);

	m_pShaderHeap->Bind();

	// カメラ設定
	DirectX::XMFLOAT3 camPos = { -10.0, 5.0f, -10 };
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

	// 時間・カメラ位置用バッファ (b1)
	static float time = 0.0f;
	DirectX::XMFLOAT4 param = {
		camPos.x, camPos.y, camPos.z, time
	};
	time += 2.0f / 60.0f;
	m_pWVPs[1]->Write(&param);

	// ===================================
	// 1. スカイスフィア (大気) の描画
	// ===================================
	// スカイスフィアを常にカメラの中心に移動させる
	mat[0] = DirectX::XMMatrixTranslation(camPos.x, camPos.y, camPos.z);
	DirectX::XMStoreFloat4x4(&fMat[0], DirectX::XMMatrixTranspose(mat[0]));
	m_pWVPs[2]->Write(&fMat);

	// 深度バッファをオフにして一番奥の背景として描画
	SetRenderTarget(_countof(hRTV), hRTV);
	m_pPipelines[2]->Bind();
	D3D12_GPU_DESCRIPTOR_HANDLE hSky[] = {
		m_pWVPs[2]->GetHandle().hGPU, // b0: 変換行列
		m_pWVPs[1]->GetHandle().hGPU  // b1: カメラ位置と時間
	};
	m_pSkyRS->Bind(hSky, 2);
	m_pSphere->Draw();

	// ===================================
	// 2. 地面・水面の描画
	// ===================================
	// 深度バッファを再び有効化
	SetRenderTarget(_countof(hRTV), hRTV, hDSV);

	m_pPipelines[0]->Bind();
	D3D12_GPU_DESCRIPTOR_HANDLE hGround[] = {
		m_pWVPs[0]->GetHandle().hGPU
	};
	m_pGroundRS->Bind(hGround, 1);
	m_pPlane->Draw();

	m_pPipelines[1]->Bind();
	D3D12_GPU_DESCRIPTOR_HANDLE hWater[] = {
		m_pWVPs[0]->GetHandle().hGPU,
		m_pWVPs[1]->GetHandle().hGPU
	};
	m_pWaterRS->Bind(hWater, 2);
	m_pPlane->Draw();
}