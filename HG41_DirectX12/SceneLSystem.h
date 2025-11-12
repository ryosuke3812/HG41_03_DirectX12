#ifndef __SCENE_LSYSTEM_H__
#define __SCENE_LSYSTEM_H__

#include "MeshBuffer.h"
#include "DescriptorHeap.h"
#include "ConstantBuffer.h"
#include "RootSignature.h"
#include "Pipeline.h"
#include "DepthStencil.h"
#include <functional>
#include <stack>
#include <string>
#include "DirectXMath.h"
#include <map>



class SceneLSystem {
public:
	SceneLSystem() {}
	~SceneLSystem() {}
	
	HRESULT Init();
	void Uninit();
	void Draw();

	void Execute(int iteration, const char* initValue, void* arg = nullptr);
	// •ÏŠ·ƒ‹[ƒ‹‚Ì’Ç‰Á
	void AddRule(char key, const char* str) {
		m_rule.insert(std::pair<char, std::string>(key, str));
	}
	// Àsˆ—‚Ì’Ç‰Á
	void AddBehavior(char key, std::function<void(void*)> func) {
		m_behavior.insert(std::pair<char, std::function<void(void*)>>(key, func));
	}
private:
	std::map<char, std::string> m_rule; // •ÏŠ·ƒ‹[ƒ‹
	std::map<char, std::function<void(void*)>> m_behavior; // •¶š‚²‚Æ‚Ì“®ì
};

#endif // __SCENE_LSYSTEM_H__