#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include "DescriptorHeap.h"

class Texture {
public:
	struct Description {
		const char* fileName;
		DescriptorHeap* pHeap;
	};

public:
	Texture(Description desc);
	~Texture();

	DescriptorHeap::Handle GetHandle();

private:
	ID3D12Resource* m_pTexture;
	DescriptorHeap::Handle m_handle;
};

#endif // __TEXTURE_H__