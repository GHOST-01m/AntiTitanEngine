#pragma once
#include "Renderer.h"


class Engine
{
	bool InitEngine();
	//InitEngine��ʹ�õ��ĺ���
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

