#ifndef __ROOT_SIGNATURE_H__
#define __ROOT_SIGNATURE_H__

#include "DirectX.h"

class RootSignature
{
public:
	struct Parameter {
		D3D12_DESCRIPTOR_RANGE_TYPE type;
		UINT slot;
		UINT num;
		D3D12_SHADER_VISIBILITY shader;
	};
	struct Description {
		Parameter* pParam;
		UINT paramNum;
	};

public:
	RootSignature(Description desc);
	~RootSignature();
	ID3D12RootSignature* Get();
	void Bind(D3D12_GPU_DESCRIPTOR_HANDLE* handle, UINT num);

private:
	ID3D12RootSignature* m_pRootSignature;
};

#endif // __ROOT_SIGNATURE_H__