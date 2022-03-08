#pragma once
#include "Window.h"
#include "Win32Window.h"
#include "GameTimer.h"
#include "Renderer.h"


class Engine
{
public:
	static Engine* mEngine;
	static Engine* Get();
	GameTimer* GetGameTimer();
	std::shared_ptr<Window> GetWindow();
	//Window* GetWindow();
	Renderer* GetRenderer();

	bool InitEngine(HINSTANCE hInstance);	//InitEngine内使用到的函数
		bool InitWindow(HINSTANCE hInstance);
		bool InitDX();

	void EngineLoop();	//EngineLoop内的函数
		bool AppRun();
		void Tick();
			void GameTick();
			void RenderTick();


	void EngineDestroy();

	void GuardedMain(HINSTANCE hInstance);

public:
	GameTimer mTimer;
	Renderer mRenderer;
	std::shared_ptr<Window> mWindow;
	//Window  mWindow;
public:
	bool      mAppPaused = false;  // is the application paused?
	bool      mMinimized = false;  // is the application minimized?
	bool      mMaximized = false;  // is the application maximized?
	bool      mResizing = false;   // are the resize bars being dragged?
	bool      mFullscreenState = false;// fullscreen enabled

public:
	std::wstring mMainWndCaption = L"AntiTitanEngine";
	int mClientWidth = 1920;
	int mClientHeight = 1080;
};

