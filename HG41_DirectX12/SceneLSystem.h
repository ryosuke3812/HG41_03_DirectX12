#ifndef __SCENE_LSYSTEM_H__
#define __SCENE_LSYSTEM_H__

#include "MeshBuffer.h"
#include "DescriptorHeap.h"
#include "ConstantBuffer.h"
#include "RootSignature.h"
#include "Pipeline.h"
#include "DepthStencil.h"
#include <functional>
#include <map>
#include <string>

class SceneLSystem
{
public:
	HRESULT Init();
	void Uninit();
	void Draw();

	void AddRule(char key, const char* str);
	// é¿çsèàóùÇÃí«â¡
	void AddBehavior(char key, std::function<void(void*)> func);
	void Execute(int iteration, const char* initValue, void* arg);

private:
	MeshBuffer* m_pTree;
	DescriptorHeap* m_pShaderHeap;
	DescriptorHeap* m_pDSVHeap;
	ConstantBuffer* m_pWVP;
	RootSignature* m_pRootSignature;
	Pipeline* m_pPipeline;
	DepthStencil* m_pDSV;

	std::map<char, std::string> m_rule;
	std::map<char, std::function<void(void*)>> m_behavior;
};

#endif // __SCENE_LSYSTEM_H__