#include "stdafx.h"
#include "Engine.h"

bool Engine::InitEngine() {

	//InitDX里用到了InitWindow之后创建的（HWND mhMainWnd）这个对象，
	//所以要先InitWindow再InitDX
	if (!InitWindow())
	{
		return false;
	}
	if (!InitDX())
	{
		return false;
	}
	return true;
};

void Engine::EngineLoop() {

};

void Engine::EngineDestroy() {

};

//InitEngine
bool Engine::InitDX() {
	return 	mRenderer.InitRenderer(mhMainWnd);
};

bool Engine::InitWindow() {

	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = mhAppInst;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"MainWnd";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT R = { 0, 0, mClientWidth, mClientHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	mhMainWnd = CreateWindow(L"MainWnd", mMainWndCaption.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, mhAppInst, 0);
	if (!mhMainWnd)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(mhMainWnd, SW_SHOW);
	UpdateWindow(mhMainWnd);

	return true;
};

//EngineLoop
void Engine::RenderLoop();
void Engine::GameLoop();