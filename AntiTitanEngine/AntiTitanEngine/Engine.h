#pragma once
#include "Window.h"
#include "Win32Window.h"
#include "GameTimer.h"
#include "Renderer.h"
#include "FSingleton.h"


class Engine
{	
	Engine();
public:
	~Engine();
	Engine(const Engine& rhs) = delete;
	Engine& operator=(const Engine& rhs) = delete;

public:
	static Engine* mEngine;
	static Engine* Get();

	std::shared_ptr<GameTimer> GetGameTimer();
	//GameTimer* GetGameTimer();
	std::shared_ptr<Window> GetWindow();
	std::shared_ptr<Asset> GetAsset();
	std::shared_ptr <Renderer> GetRenderer();

	bool InitEngine(HINSTANCE hInstance);	//InitEngine内使用到的函数
		bool InitWindow(HINSTANCE hInstance);
		bool InitDX();
	void EngineLoop();	//EngineLoop内的函数
		void Tick();
	void EngineDestroy();
	void GuardedMain(HINSTANCE hInstance);

private:
	static std::shared_ptr<GameTimer>   mTimer;
	static std::shared_ptr<Renderer>    mRenderer;
	static std::shared_ptr<Window>      mWindow;
	static std::shared_ptr<Asset>       mAsset;
public:
	GameTimer  gt;
public:
	float mTotalTime;

public:
	bool      mAppPaused = true;  // is the application paused?
	bool      mMinimized = false;  // is the application minimized?
	bool      mMaximized = false;  // is the application maximized?
	bool      mResizing = false;   // are the resize bars being dragged?
	bool      mFullscreenState = false;// fullscreen enabled

public:
	std::wstring mMainWndCaption = L"AntiTitanEngine";
	std::string MapPath="MapActorInfo / MapActorInfo.bat";
};

