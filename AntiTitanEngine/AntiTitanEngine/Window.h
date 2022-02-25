#pragma once
#include "GameTimer.h"
#include <crtdbg.h>
#include <windowsx.h>

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

class Window {

public:
	virtual bool InitializeMainWindow() = 0;

protected:
	std::wstring mMainWndCaption;
	float mClientWidth;
	float mClientHeight;

	GameTimer mTimer;
};