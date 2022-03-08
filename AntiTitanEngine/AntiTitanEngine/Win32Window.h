#pragma once
#include "Window.h"

class Win32Window :public Window
{
public:
	bool InitWindow();
	static Win32Window* mWin32Window;
	static Win32Window* Get();

public:

	HWND GetHWND();

	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	HWND mhMainWnd = nullptr;
	HINSTANCE mhAppInst = nullptr;


};

