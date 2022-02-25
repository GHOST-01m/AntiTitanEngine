#pragma once
#include "Window.h"

class WindowsWindow : public Window {
public:
	bool InitializeMainWindow();
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	std::wstring mMainWndCaption;
	float mClientWidth;
	float mClientHeight;

	GameTimer mTimer;
	HWND      mhMainWnd = nullptr; // main window handle
	HINSTANCE mhAppInst = nullptr;

//Windows特有成员

	bool      mAppPaused = false;  // is the application paused?
	bool      mMinimized = false;  // is the application minimized?
	bool      mMaximized = false;  // is the application maximized?
	bool      mResizing = false;   // are the resize bars being dragged?
	bool      mFullscreenState = false;// fullscreen enabled

};