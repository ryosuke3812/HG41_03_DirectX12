#ifndef __SCENE_PROCEDURAL_H__
#define __SCENE_PROCEDURAL_H__

#include "MeshBuffer.h"
#include "DescriptorHeap.h"
#include "ConstantBuffer.h"
#include "RootSignature.h"
#include "Pipeline.h"
#include "DepthStencil.h"
#include <vector>

class SceneProcedural
{
public:
	HRESULT Init();
	void Uninit();
	void Draw();

private:
};

#endif // __SCENE_PROCEDURAL_H__