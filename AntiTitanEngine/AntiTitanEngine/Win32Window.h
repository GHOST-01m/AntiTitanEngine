#pragma once
#include "Window.h"


class Win32Window :public Window
{
public:
	bool InitWindow();
	static Win32Window* mWindow;

public:

	static Win32Window* Get();

	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	HWND mhMainWnd = nullptr;
	HINSTANCE mhAppInst = nullptr;
	bool      mAppPaused = false;  // is the application paused?
	bool      mMinimized = false;  // is the application minimized?
	bool      mMaximized = false;  // is the application maximized?
	bool      mResizing = false;   // are the resize bars being dragged?
	bool      mFullscreenState = false;// fullscreen enabled


	//GameTimer mTimer;
	//Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice ;
};

