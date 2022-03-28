#pragma once
#include "Window.h"

class Win32Window :public Window
{
public:
	Win32Window();
	~Win32Window();
public:
	bool InitWindow(HINSTANCE hInstance);
	static Win32Window* mWin32Window;
	static Win32Window* Get();
	static void ReleaseWindow();
public:

	HWND GetHWND();

	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	bool Run();

	HWND mhMainWnd = nullptr;
	HINSTANCE mhAppInst = nullptr;

};

