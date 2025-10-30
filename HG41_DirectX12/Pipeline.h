#ifndef __PIPELINE_H__
#define __PIPELINE_H__

#include "DirectX.h"

class Pipeline
{
public:
	struct InputLayout {
		const char* name;
		UINT index;
		DXGI_FORMAT format;
	};
	struct Description {
		ID3D12RootSignature* pRootSignature;
		const wchar_t* VSFile;
		const wchar_t* PSFile;
		InputLayout* pInputLayout;
		UINT InputLayoutNum;
		UINT RenderTargetNum;
		bool EnableDepth;
	};

public:
	Pipeline(Description desc);
	~Pipeline();
	void Bind();

private:
	ID3D12PipelineState* m_pPipeline;
};

#endif // __PIPELINE_H__