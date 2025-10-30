#include <Windows.h>
#include <tchar.h>
#include "DirectX.h"
#include "Scene.h"
#include "Input.h"


#include <memory>
#include "DescriptorHeap.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx12.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);


std::shared_ptr<DescriptorHeap> g_pHeapIMGUI;


// ウィンドウプロシージャ
LRESULT WinProc(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	default:break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	ImGui_ImplWin32_WndProcHandler(hWnd, msg, wparam, lparam);
	return DefWindowProc(hWnd, msg, wparam, lparam);
}

void Draw()
{
#ifdef _DEBUG
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
#endif

	DrawScene();

#ifdef _DEBUG
	ImGui::Render();
	ID3D12DescriptorHeap* heap = g_pHeapIMGUI->Get();
	GetCommandList()->SetDescriptorHeaps(1, &heap);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), GetCommandList());
#endif
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	// ウィンドウクラスの生成
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = WinProc;
	wc.lpszClassName = _T("DirectX12");
	wc.hInstance = hInstance;
	RegisterClassEx(&wc);

	// ウィンドウの作成
	RECT wndRect = { 0,0,1280,720 };
	AdjustWindowRect(&wndRect, WS_OVERLAPPEDWINDOW, false);
	HWND hWnd = CreateWindow(
		wc.lpszClassName, _T("DirectX12"), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		wndRect.right - wndRect.left, wndRect.bottom - wndRect.top,
		NULL, NULL, hInstance, NULL
	);
	ShowWindow(hWnd, nCmdShow);

	MSG msg = {};
	if (FAILED(InitDirectX(hWnd, 1280, 720, false)))
	{
		MessageBox(hWnd, _T("Error"), _T("Failed [InitDirectX]."), MB_OK);
		msg.message = WM_QUIT;
	}
	if (FAILED(InitScene()))
	{
		MessageBox(hWnd, _T("Error"), _T("Failed [InitGame]."), MB_OK);
		msg.message = WM_QUIT;
	}

	{	// IMGUI用ディスクリプターヒープ
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.num = 3;
		g_pHeapIMGUI = std::make_shared<DescriptorHeap>(desc);
	}
	{	// ImGUI初期化
		if (ImGui::CreateContext() == nullptr) {
			MessageBox(hWnd, _T("Error"), _T("Failed [Imgui]."), MB_OK);
			msg.message = WM_QUIT;
		}
		else {
			if (!ImGui_ImplWin32_Init(hWnd)) {
				MessageBox(hWnd, _T("Error"), _T("Failed [Imgui]."), MB_OK);
				msg.message = WM_QUIT;
			}
			else {
				auto handle = g_pHeapIMGUI->Allocate();
				bool a = ImGui_ImplDX12_Init(GetDevice(),
					3, DXGI_FORMAT_R8G8B8A8_UNORM,
					g_pHeapIMGUI->Get(), handle.hCPU, handle.hGPU);
			}

		}
	}

	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			UpdateInput();
			float color[] = { 0.8f, 0.9f, 1.0f, 1.0f };
			DrawDirectX(Draw, color);
		}
	}

	UninitScene();
	UninitDirectX();
	UnregisterClass(wc.lpszClassName, hInstance);
	return 0;
}


