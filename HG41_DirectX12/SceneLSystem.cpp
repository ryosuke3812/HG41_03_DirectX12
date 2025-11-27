#include "SceneLSystem.h"
#include <functional>
#include <stack>
#include <string>
#include <map>
#include <vector>
#include <DirectXMath.h>

// ファイルスコープのレンダリングリソース（ヘッダを変更せずに管理）
static MeshBuffer*      s_pLineMesh = nullptr;
static DescriptorHeap*  s_pShaderHeap = nullptr;
static ConstantBuffer*  s_pWVP = nullptr;
static RootSignature*   s_pRootSignature = nullptr;
static Pipeline*        s_pPipeline = nullptr;

HRESULT SceneLSystem::Init()
{
	// 頂点座標（L-Systemの結果を格納する一時）
	struct Vertex {
		float x, y, z;
	};
	std::vector<Vertex> vtx;

	// スタック情報
	struct Param {
		DirectX::XMFLOAT3 pos; // 現在の位置
		DirectX::XMFLOAT3 vec; // 現在の進行方向
	};
	std::stack<Param> stack;
	// 初期値
	stack.push({ DirectX::XMFLOAT3(0,0,0), DirectX::XMFLOAT3(0,1,0) });

	//--- LSystemの作成（文字に対する動作はスタックを引数で渡す）
	// 移動処理の定義
	auto moveFunc = [&vtx](void* arg)
		{
			std::stack<Param>* pStack = reinterpret_cast<std::stack<Param>*>(arg);
			Param& top = pStack->top();
			// 始点
			vtx.push_back({ top.pos.x, top.pos.y, top.pos.z });
			// 進む
			top.pos.x += top.vec.x;
			top.pos.y += top.vec.y;
			top.pos.z += top.vec.z;
			// 終点
			vtx.push_back({ top.pos.x, top.pos.y, top.pos.z });
		};
	// スタック処理の定義
	auto pushFunc = [](void* arg)
		{
			std::stack<Param>* pStack = reinterpret_cast<std::stack<Param>*>(arg);
			pStack->push(pStack->top());
		};
	auto popFunc = [](void* arg)
		{
			std::stack<Param>* pStack = reinterpret_cast<std::stack<Param>*>(arg);
			pStack->pop();
		};

	// 他のルールを参考に+,-の回転処理ルールを実装
	// 厳密な回転でなくてもスタック中の進行方向を（うまいこと）変えるだけでもそれっぽくなる
	// 回転角度（例: 25度）
	const float kAngle = DirectX::XMConvertToRadians(10.0f);

	// + (時計回り) の回転
	auto rotPlusFunc = [=](void* arg)
		{
			std::stack<Param>* pStack = reinterpret_cast<std::stack<Param>*>(arg);
			Param& top = pStack->top();

			DirectX::XMMATRIX R = DirectX::XMMatrixRotationZ(-kAngle); // -25度
			DirectX::XMVECTOR vec = DirectX::XMLoadFloat3(&top.vec);
			vec = DirectX::XMVector3Transform(vec, R);
			DirectX::XMStoreFloat3(&top.vec, vec);
		};

	// - (反時計回り) の回転
	auto rotMinusFunc = [=](void* arg)
		{
			std::stack<Param>* pStack = reinterpret_cast<std::stack<Param>*>(arg);
			Param& top = pStack->top();

			DirectX::XMMATRIX R = DirectX::XMMatrixRotationZ(kAngle); // +25度
			DirectX::XMVECTOR vec = DirectX::XMLoadFloat3(&top.vec);
			vec = DirectX::XMVector3Transform(vec, R);
			DirectX::XMStoreFloat3(&top.vec, vec);
		};

	SceneLSystem lsystem;
	// ルール追加
	lsystem.AddRule('F', "X[+F][-F]");
	// 文字に対応した処理の設定
	lsystem.AddBehavior('F', moveFunc);
	lsystem.AddBehavior('X', moveFunc);
	lsystem.AddBehavior('[', pushFunc);
	lsystem.AddBehavior(']', popFunc);
	// +,-の回転処理ルールを実装
	lsystem.AddBehavior('+', rotPlusFunc);
	lsystem.AddBehavior('-', rotMinusFunc);

	// 構築済みのルール、処理に基づいてLSystem内で頂点データを生成
	lsystem.Execute(6, "F", &stack);

	// 頂点バッファの生成
	{
		MeshBuffer::Description desc = {};
		desc.pVtx = vtx.data();
		desc.vtxSize = sizeof(Vertex);
		desc.vtxCount = vtx.size();
		desc.topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		m_pTree = new MeshBuffer(desc);
	}

	// オブジェクト用ディスクリプターヒープ作成
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.num = 1;
		m_pShaderHeap = new DescriptorHeap(desc);
	}
	// オブジェクト用の定数バッファ作成
	{
		ConstantBuffer::Description desc = {};
		desc.pHeap = m_pShaderHeap;
		desc.size = sizeof(DirectX::XMFLOAT4X4) * 3;
		m_pWVP = new ConstantBuffer(desc);
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
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT},
			{"UV", 0, DXGI_FORMAT_R32G32_FLOAT},
		};
		Pipeline::Description desc = {};
		desc.pInputLayout = layout;
		desc.InputLayoutNum = _countof(layout);
		desc.VSFile = L"VS_LSystem.cso";
		desc.PSFile = L"PS_LSystem.cso";
		desc.pRootSignature = m_pRootSignature->Get();
		desc.RenderTargetNum = 1;
		desc.EnableDepth = true;
		m_pPipeline = new Pipeline(desc);
	}

	// 深度バッファ用のディスクリプター作成
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
void SceneLSystem::Uninit()
{
	delete m_pDSV;
	delete m_pPipeline;
	delete m_pRootSignature;
	delete m_pWVP;
	delete m_pDSVHeap;
	delete m_pShaderHeap;
	delete m_pTree;
}

void SceneLSystem::Draw()
{
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

	// パイプライン、ヒープ設定
	m_pPipeline->Bind();
	m_pShaderHeap->Bind();

	// 変換行列の計算
	DirectX::XMMATRIX mat[3];
	static float rad = 0.0f;
	mat[0] = DirectX::XMMatrixIdentity();
	mat[1] = DirectX::XMMatrixLookAtLH(
		DirectX::XMVectorSet(cosf(rad) * 30.0f, 30.0f, sinf(rad) * 30.0f, 0.0),
		DirectX::XMVectorSet(0.0f, 10.0f, 0.0f, 0.0),
		DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0)
	);
	mat[2] = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(60.0f), 16.f / 9.f, 0.1f, 100.0f
	);
	rad += 0.003f;

	// 定数バッファへ設定
	DirectX::XMFLOAT4X4 fMat[3];
	for (int i = 0; i < 3; ++i)
		DirectX::XMStoreFloat4x4(&fMat[i], DirectX::XMMatrixTranspose(mat[i]));
	m_pWVP->Write(&fMat);

	// 描画
	D3D12_GPU_DESCRIPTOR_HANDLE handle[] = {
		m_pWVP->GetHandle().hGPU
	};
	m_pRootSignature->Bind(handle, 1);
	m_pTree->Draw();
}

void SceneLSystem::AddRule(char key, const char* str)
{
	m_rule.insert(std::pair<char, std::string>(key, str));
}

void SceneLSystem::AddBehavior(char key, std::function<void(void*)> func)
{
	m_behavior.insert(std::pair<char, std::function<void(void*)>>(key, func));
}

void SceneLSystem::Execute(int iteration, const char* initValue, void* arg)
{
	std::string::iterator it;
	std::string base; // 変換前文字列
	std::string str = initValue; // 変換後文字列
	// イテレーション回数分、変換処理を実行
	for (int i = 0; i < iteration; ++i) {
		base = str; // 変換済みの文字列を再度変換
		str = ""; // 変換後の文字を格納できるよう初期化
		it = base.begin();
		while (it != base.end()) {
			auto rule = m_rule.find(*it); // 変換ルールがあるか探索
			if (rule != m_rule.end())
				str += rule->second; // 変換後の文字を追加
			else
				str += *it; // 変換ルールがないので、現在の文字をそのまま追加
			++it;
		}
	}
	// 変換後の文字に対応する処理を実行
	it = str.begin();
	while (it != str.end()) {
		auto behavior = m_behavior.find(*it);
		if (behavior != m_behavior.end())
			behavior->second(arg);
		++it;
	}
}
