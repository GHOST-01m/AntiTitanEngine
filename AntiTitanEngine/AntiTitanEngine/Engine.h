#pragma once
#include "Renderer.h"
#include "Win32Window.h"

class Engine
{
	bool InitEngine();
	//InitEngine内使用到的函数

		bool InitWindow();
		bool InitDX();

	void EngineLoop();
	//EngineLoop内的函数
		void RenderLoop();
		void GameLoop();

	void EngineDestroy();


public:
	GameTimer mTimer;
	Renderer mRenderer;
	Win32Window mWindow;//

public:
	std::wstring mMainWndCaption = L"AntiTitanEngine";
	int mClientWidth = 1920;
	int mClientHeight = 1080;
};

