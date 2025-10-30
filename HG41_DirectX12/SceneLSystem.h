#ifndef __SCENE_LSYSTEM_H__
#define __SCENE_LSYSTEM_H__

#include "MeshBuffer.h"
#include "DescriptorHeap.h"
#include "ConstantBuffer.h"
#include "RootSignature.h"
#include "Pipeline.h"
#include "DepthStencil.h"

class SceneLSystem
{
public:
	HRESULT Init();
	void Uninit();
	void Draw();

private:
};

#endif // __SCENE_LSYSTEM_H__