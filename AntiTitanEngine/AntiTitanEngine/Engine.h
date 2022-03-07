#pragma once
#include "Renderer.h"


class Engine
{
	bool InitEngine();
	//InitEngine内使用到的函数
		bool InitWindow();
		HWND mhMainWnd = nullptr;
		HINSTANCE mhAppInst = nullptr;
		bool InitDX();

	void EngineLoop();
	//EngineLoop
		void RenderLoop();
		void GameLoop();

	void EngineDestroy();






		Renderer mRenderer;
};

