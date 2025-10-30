#ifndef __DIRECTX12_H__
#define __DIRECTX12_H__

#include <d3d12.h>
#include <dxgi1_6.h>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#define SAFE_RELEASE(p) do { if(p){p->Release(); p = nullptr;} } while(0)

HRESULT InitDirectX(HWND hWnd, UINT width, UINT height, bool fullscreen);
void UninitDirectX();
void DrawDirectX(void(func)(void), const float clearColor[4]);
void SetRenderTarget(int num, D3D12_CPU_DESCRIPTOR_HANDLE * hRTVs);
void SetRenderTarget(int num, D3D12_CPU_DESCRIPTOR_HANDLE* hRTVs, D3D12_CPU_DESCRIPTOR_HANDLE hDSV);

ID3D12Device* GetDevice();
ID3D12GraphicsCommandList* GetCommandList();
D3D12_CPU_DESCRIPTOR_HANDLE GetRTV();

#endif