#include "stdafx.h"
#include "Engine.h"

bool Engine::InitEngine() {

	mTimer = mWindow.mTimer;

	if (!InitWindow())
	{
		return false;
	}
	if (!InitDX())
	{
		return false;
	}




	return true;
};

void Engine::EngineLoop() {

};

void Engine::EngineDestroy() {

};

//InitEngine
bool Engine::InitDX() {
	return 	mRenderer.InitRenderer(mWindow.mhMainWnd);
};



bool Engine::InitWindow() {
	return mWindow.InitWindow();
};

//EngineLoop
void Engine::RenderLoop();
void Engine::GameLoop();