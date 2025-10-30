#include "Pipeline.h"
#include <vector>
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

Pipeline::Pipeline(Description desc)
	: m_pPipeline(nullptr)
{
	// ラスタライザステートの設定
	D3D12_RASTERIZER_DESC rasterDesc = {};
	rasterDesc.MultisampleEnable = false;
	rasterDesc.CullMode = D3D12_CULL_MODE_NONE;
	rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterDesc.DepthClipEnable = false; // 深度クリッピング

	// ブレンドの設定
	D3D12_BLEND_DESC blendDesc;
	blendDesc.AlphaToCoverageEnable = false; // サンプリングを考慮した中間αのテスト設定
	blendDesc.IndependentBlendEnable = false;	//レンダ―ターゲットごとに個別にパラメータを設定するか
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].LogicOpEnable = false;
	blendDesc.RenderTarget[0].RenderTargetWriteMask
		= D3D12_COLOR_WRITE_ENABLE_ALL;

	// 深度ステートの設定
	D3D12_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = desc.EnableDepth ? TRUE : FALSE;
	dsDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	dsDesc.StencilEnable = FALSE;

	// シェーダーの読み込み
	HRESULT hr;
	ID3DBlob *pVS, *pPS;
	hr = D3DReadFileToBlob(desc.VSFile, &pVS);		// 4x4の行列を使用した頂点シェーダ（後述
	if (FAILED(hr)) {
		MessageBox(NULL, L"VS error", L"error", MB_OK);
		return;
	}
	hr = D3DReadFileToBlob(desc.PSFile, &pPS);		// 直接色を出力するピクセルシェーダ（後述
	if (FAILED(hr)) {
		MessageBox(NULL, L"PS error", L"error", MB_OK);
		return;
	}

	// インプットレイアウト
	std::vector<D3D12_INPUT_ELEMENT_DESC> element;
	element.resize(desc.InputLayoutNum);
	for (int i = 0; i < desc.InputLayoutNum; ++i)
	{
		element[i].SemanticName = desc.pInputLayout[i].name;
		element[i].SemanticIndex = desc.pInputLayout[i].index;
		element[i].Format = desc.pInputLayout[i].format;
		element[i].InputSlot = 0;
		element[i].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		element[i].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		element[i].InstanceDataStepRate = 0;
	}

	// パイプラインステートの設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc = {};
	// ルートシグネチャ
	pipelineDesc.pRootSignature = desc.pRootSignature;
	// シェーダ
	pipelineDesc.VS = { pVS->GetBufferPointer(), pVS->GetBufferSize() };
	pipelineDesc.PS = { pPS->GetBufferPointer(), pPS->GetBufferSize() };
	// 頂点レイアウト
	pipelineDesc.InputLayout = { element.data(), desc.InputLayoutNum };
	// サンプリング
	pipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	pipelineDesc.SampleDesc.Count = 1;
	pipelineDesc.SampleDesc.Quality = 0;
	// ラスタライザ
	pipelineDesc.RasterizerState = rasterDesc;
	// ブレンド
	pipelineDesc.BlendState = blendDesc;
	// プリミティブ
	pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;	// 構成要素（三角形
	// レンダーターゲット
	pipelineDesc.NumRenderTargets = desc.RenderTargetNum;
	for(int i = 0; i < desc.RenderTargetNum; ++ i)
		pipelineDesc.RTVFormats[i] = DXGI_FORMAT_R8G8B8A8_UNORM;
	pipelineDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
	// 深度バッファ
	pipelineDesc.DepthStencilState = dsDesc;
	pipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	// パイプラインの生成
	hr = GetDevice()->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&m_pPipeline));
	if (FAILED(hr)) {
		MessageBox(NULL, L"Pipeline.cpp", L"error", MB_OK);
		return;
	}
}
Pipeline::~Pipeline()
{
	if (m_pPipeline) m_pPipeline->Release();
}
void Pipeline::Bind()
{
	GetCommandList()->SetPipelineState(m_pPipeline);
}