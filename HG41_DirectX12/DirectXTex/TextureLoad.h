#ifndef __TEXTURE_LOAD_H__
#define __TEXTURE_LOAD_H__

#include "DirectXTex.h"

#ifdef _DEBUG
#if (_MSC_VER >= 1920)
#ifdef _WIN64
#pragma comment(lib, "DirectXTex/DirectXTexD_x64_vs2019.lib")
#else
#pragma comment(lib, "DirectXTex/DirectXTexD_x86_vs2019.lib")
#endif
#elif (_MSC_VER >= 1910)
#ifdef _WIN64
#pragma comment(lib, "DirectXTex/DirectXTexD_x64_vs2017.lib")
#else
#pragma comment(lib, "DirectXTex/DirectXTexD_x86_vs2017.lib")
#endif
#endif
#else
#pragma comment(lib, "DirectXTex/DirectXTex.lib")
#endif

inline HRESULT LoadTexture(const char* fileName, DirectX::TexMetadata* pInfo, DirectX::ScratchImage* pImage)
{
	HRESULT hr;

	wchar_t wPath[MAX_PATH];
	size_t wLen = 0;
	MultiByteToWideChar(0, 0, fileName, -1, wPath, MAX_PATH);

	// hdr‘Î‰ž
	if (strstr(fileName, ".hdr")) {
		hr = DirectX::LoadFromHDRFile(wPath, pInfo, *pImage);
	}
	else {
		hr = DirectX::LoadFromWICFile(wPath, DirectX::WIC_FLAGS_NONE, pInfo, *pImage);
	}

	return hr;
}

#endif // __TEXTURE_LOAD_H__
