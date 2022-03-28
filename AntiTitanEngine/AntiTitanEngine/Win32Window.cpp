#include "stdafx.h"
#include "Win32Window.h"

Win32Window* Win32Window::mWin32Window = nullptr;

LRESULT CALLBACK
Main_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Forward hwnd on because we can get messages (e.g., WM_CREATE)
	// before CreateWindow returns, and thus before mhMainWnd is valid.
	return Win32Window::Get()->MsgProc(hwnd, msg, wParam, lParam);
}

Win32Window* Win32Window::Get() {
	return mWin32Window;
}

void Win32Window::ReleaseWindow()
{
	delete mWin32Window;
	mWin32Window = NULL;
}

HWND Win32Window::GetHWND() {
	return mhMainWnd;
};

LRESULT Win32Window::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (!mWin32Window)
	{
		return DefWindowProc(hwnd, msg, wParam, lParam);

	}
	switch (msg)
	{
		// WM_ACTIVATE is sent when the window is activated or deactivated.  
		// We pause the game when the window is deactivated and unpause it 
		// when it becomes active.  
	//case WM_ACTIVATE:
	//	if (LOWORD(wParam) == WA_INACTIVE)
	//	{
	//		Engine::Get()->mAppPaused = true;
	//		Engine::Get()->GetGameTimer()->Stop();
	//	}
	//	else
	//	{
	//		Engine::Get()->mAppPaused = false;
	//		Engine::Get()->GetGameTimer()->Start();
	//	}
	//	return 0;

	//	// WM_SIZE is sent when the user resizes the window.  
	//case WM_SIZE:
	//	// Save the new client area dimensions.
	//	mClientWidth = LOWORD(lParam);
	//	mClientHeight = HIWORD(lParam);
	//	if (Engine::Get()->GetRenderer()->Getd3dDevice())
	//	//if (Engine::Get()->GetRenderer()->IsDeviceValid())
	//	//if (true)
	//	{
	//		if (wParam == SIZE_MINIMIZED)
	//		{
	//			Engine::Get()->mAppPaused = true;
	//			Engine::Get()->mMinimized = true;
	//			Engine::Get()->mMaximized = false;
	//		}
	//		else if (wParam == SIZE_MAXIMIZED)
	//		{
	//			Engine::Get()->mAppPaused = false;
	//			Engine::Get()->mMinimized = false;
	//			Engine::Get()->mMaximized = true;
	//			Engine::Get()->GetRenderer()->OnResize();
	//		}
	//		else if (wParam == SIZE_RESTORED)
	//		{

	//			// Restoring from minimized state?
	//			if (Engine::Get()->mMinimized)
	//			{
	//				Engine::Get()->mAppPaused = false;
	//				Engine::Get()->mMinimized = false;
	//				Engine::Get()->GetRenderer()->OnResize();
	//			}

	//			// Restoring from maximized state?
	//			else if (Engine::Get()->mMaximized)
	//			{
	//				Engine::Get()->mAppPaused = false;
	//				Engine::Get()->mMaximized = false;
	//				Engine::Get()->GetRenderer()->OnResize();
	//			}
	//			else if (Engine::Get()->mResizing)
	//			{
	//				// If user is dragging the resize bars, we do not resize 
	//				// the buffers here because as the user continuously 
	//				// drags the resize bars, a stream of WM_SIZE messages are
	//				// sent to the window, and it would be pointless (and slow)
	//				// to resize for each WM_SIZE message received from dragging
	//				// the resize bars.  So instead, we reset after the user is 
	//				// done resizing the window and releases the resize bars, which 
	//				// sends a WM_EXITSIZEMOVE message.
	//			}
	//			else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
	//			{
	//				Engine::Get()->GetRenderer()->OnResize();
	//			}
	//		}
	//	}
	//	return 0;

	//	// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	//case WM_ENTERSIZEMOVE:
	//	Engine::Get()->mAppPaused = true;
	//	Engine::Get()->mResizing = true;
	//	Engine::Get()->GetGameTimer()->Stop();

	//	return 0;

	//	// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
	//	// Here we reset everything based on the new window dimensions.
	//case WM_EXITSIZEMOVE:
	//	Engine::Get()->mAppPaused = false;
	//	Engine::Get()->mResizing = false;
	//	Engine::Get()->GetGameTimer()->Start();
	//	Engine::Get()->GetRenderer()->OnResize();
	//	return 0;

		// WM_DESTROY is sent when the window is being destroyed.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		// The WM_MENUCHAR message is sent when a menu is active and the user presses 
		// a key that does not correspond to any mnemonic or accelerator key. 
	//case WM_MENUCHAR:
	//	// Don't beep when we alt-enter.
	//	return MAKELRESULT(0, MNC_CLOSE);

		// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		std::dynamic_pointer_cast<DXRHI>(Engine::Get()->GetRenderer()->GetRHI())->OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		std::dynamic_pointer_cast<DXRHI>(Engine::Get()->GetRenderer()->GetRHI())->OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		std::dynamic_pointer_cast<DXRHI>(Engine::Get()->GetRenderer()->GetRHI())->OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_KEYUP:
		if (wParam == VK_ESCAPE)
		{
			PostQuitMessage(0);
			return 0;
		}
		else if ((int)wParam == VK_F2)
			Engine::Get()->GetRenderer()->Set4xMsaaState(!Engine::Get()->GetRenderer()->m4xMsaaState);
		return 0;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hwnd, &ps);
			EndPaint(hwnd, &ps);
		}

		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
};

Win32Window::Win32Window()
{

}

Win32Window::~Win32Window()
{

}

bool Win32Window::InitWindow(HINSTANCE hInstance) {

	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = Main_WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
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


	if (mWin32Window == nullptr)
	{
		mWin32Window = new Win32Window;
	}
	mWin32Window->mhAppInst = hInstance;

	return true;
}


bool Win32Window::Run()
{
	bool quit = false;

	MSG msg = { 0 };

	Engine::Get()->GetGameTimer()->Reset();

	// If there are Window messages then process them.
	while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		if (msg.hwnd== mhMainWnd)
		{
			Main_WndProc(msg.hwnd, msg.message, msg.wParam, msg.lParam);
		}
		else 
		{
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
		{
			quit = true;
		}
	}
	return !quit;
}