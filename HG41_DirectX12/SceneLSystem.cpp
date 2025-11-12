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
	struct GenVertex {
		float x, y, z;
	};
	std::vector<GenVertex> genVtx;

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
	auto moveFunc = [&genVtx](void* arg)
		{
			std::stack<Param>* pStack = reinterpret_cast<std::stack<Param>*>(arg);
			Param& top = pStack->top();
			// 始点
			genVtx.push_back({ top.pos.x, top.pos.y, top.pos.z });
			// 進む
			top.pos.x += top.vec.x;
			top.pos.y += top.vec.y;
			top.pos.z += top.vec.z;
			// 終点
			genVtx.push_back({ top.pos.x, top.pos.y, top.pos.z });
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

	// +,- の回転はコメント化された例を参照しており、今回の実装では簡易処理として使わない（必要なら追加可能）
	SceneLSystem lsystem;
	// ルール追加
	lsystem.AddRule('F', "X[+F][-F]");
	// 文字に対応した処理の設定
	lsystem.AddBehavior('F', moveFunc);
	lsystem.AddBehavior('X', moveFunc);
	lsystem.AddBehavior('[', pushFunc);
	lsystem.AddBehavior(']', popFunc);

	// 構築済みのルール、処理に基づいてLSystem内で頂点データを生成
	lsystem.Execute(6, "F", &stack);

	// 生成頂点を MeshBuffer に渡すために頂点フォーマットを作成
	// 他のシーンに合わせて位置＋色のフォーマットを使用する（シェーダは VS_FractalObject.cso / PS_FractalObject.cso を流用）
	struct Vertex {
		float pos[3];
		float color[4];
	};
	std::vector<Vertex> vtx;
	vtx.reserve(genVtx.size());
	for (size_t i = 0; i < genVtx.size(); ++i) {
		float gray = 1.0f; // 線の色（必要なら階層や深さに応じて変える）
		vtx.push_back({ { genVtx[i].x, genVtx[i].y, genVtx[i].z }, { gray, gray, gray, 1.0f } });
	}

	// MeshBuffer 作成（LINELIST：各2点で1本の線）
	if (!vtx.empty()) {
		MeshBuffer::Description mdesc = {};
		mdesc.pVtx = vtx.data();
		mdesc.vtxSize = sizeof(Vertex);
		mdesc.vtxCount = (UINT)vtx.size();
		mdesc.pIdx = nullptr;
		mdesc.topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		// 既に作られていれば破棄してから作り直す
		if (s_pLineMesh) { delete s_pLineMesh; s_pLineMesh = nullptr; }
		s_pLineMesh = new MeshBuffer(mdesc);
	}

	// 以下は描画に必要なシェーダ／ルートシグネチャ等を作成（SceneFractal と同様の設定）
	// オブジェクト用ディスクリプターヒープ作成（定数バッファ1つ）
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.num = 1;
		if (s_pShaderHeap) { delete s_pShaderHeap; s_pShaderHeap = nullptr; }
		s_pShaderHeap = new DescriptorHeap(desc);
	}
	// 定数バッファ作成（WVP）
	{
		ConstantBuffer::Description desc = {};
		desc.pHeap = s_pShaderHeap;
		desc.size = sizeof(DirectX::XMFLOAT4X4);
		if (s_pWVP) { delete s_pWVP; s_pWVP = nullptr; }
		s_pWVP = new ConstantBuffer(desc);
	}
	// ルートシグネチャ（VS側で b0: CBV を使用）
	{
		RootSignature::Parameter param[] = {
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
		};
		RootSignature::Description desc = {};
		desc.pParam = param;
		desc.paramNum = _countof(param);
		if (s_pRootSignature) { delete s_pRootSignature; s_pRootSignature = nullptr; }
		s_pRootSignature = new RootSignature(desc);
	}
	// パイプライン作成（頂点属性は POSITION, COLOR）
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
		desc.pRootSignature = s_pRootSignature->Get();
		desc.RenderTargetNum = 1;
		desc.EnableDepth = false; // 線描画なので深度無効
		if (s_pPipeline) { delete s_pPipeline; s_pPipeline = nullptr; }
		s_pPipeline = new Pipeline(desc);
	}

	return S_OK;
}
void SceneLSystem::Uninit()
{
	// 作成したリソースを破棄
	if (s_pLineMesh) { delete s_pLineMesh; s_pLineMesh = nullptr; }
	if (s_pPipeline) { delete s_pPipeline; s_pPipeline = nullptr; }
	if (s_pRootSignature) { delete s_pRootSignature; s_pRootSignature = nullptr; }
	if (s_pWVP) { delete s_pWVP; s_pWVP = nullptr; }
	if (s_pShaderHeap) { delete s_pShaderHeap; s_pShaderHeap = nullptr; }
}

void SceneLSystem::Draw()
{
	// 必要なリソースがなければ何もしない
	if (!s_pLineMesh || !s_pPipeline || !s_pRootSignature || !s_pWVP || !s_pShaderHeap)
		return;

	// 描画先の設定
	ID3D12GraphicsCommandList* pCmdList = GetCommandList();
	D3D12_CPU_DESCRIPTOR_HANDLE hRTV[] = { GetRTV() };
	SetRenderTarget(_countof(hRTV), hRTV, D3D12_CPU_DESCRIPTOR_HANDLE{ 0 });

	// 表示領域の設定
	float width = 1280.0f;
	float height = 720.0f;
	D3D12_VIEWPORT vp = { 0, 0, width, height, 0.0f, 1.0f };
	D3D12_RECT scissor = { 0, 0, (LONG)width, (LONG)height };
	pCmdList->RSSetViewports(1, &vp);
	pCmdList->RSSetScissorRects(1, &scissor);

	// 定数バッファ（WVP）を書き込む
	// ワールドは単位行列、ビューは少し後方から見下ろす簡易カメラ、投影は透視投影
	DirectX::XMMATRIX W = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX V = DirectX::XMMatrixLookAtLH(
		DirectX::XMVectorSet(0.0f, 0.0f, -2.0f, 1.0f),
		DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
		DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(60.0f),
		width / height,
		0.1f,
		100.0f);
	DirectX::XMFLOAT4X4 fMat;
	DirectX::XMStoreFloat4x4(&fMat, DirectX::XMMatrixTranspose(W * V * P));
	s_pWVP->Write(&fMat);

	// パイプライン、ヒープの設定
	s_pPipeline->Bind();
	s_pShaderHeap->Bind();

	// 定数バッファハンドルをルートシグネチャへバインドして描画
	D3D12_GPU_DESCRIPTOR_HANDLE handle = s_pWVP->GetHandle().hGPU;
	s_pRootSignature->Bind(&handle, 1);

	// メッシュを描画（MeshBuffer が VA/VB をセットして Draw を呼ぶ）
	s_pLineMesh->Draw();
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
