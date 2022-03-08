#include "stdafx.h"

Engine* Engine::mEngine = nullptr;

Engine* Engine::Get() {
	return mEngine;
}

GameTimer* Engine::GetGameTimer() {
	return &mTimer;
};

std::shared_ptr<Window> Engine::GetWindow() {
	return mWindow;
};
//Window* Engine::GetWindow() {
//	return &mWindow;
//};


Renderer* Engine::GetRenderer() {
	return &mRenderer;
}

bool Engine::InitEngine() {

	if (mEngine == nullptr)
	{
		mEngine = new Engine;
	}

	if (!InitWindow())
	{
		return false;
	}	
	//mWindow->mMainWndCaption = mMainWndCaption;

	if (!InitDX())
	{
		return false;
	}

	return true;
};

void Engine::EngineLoop() {

	while (mAppPaused && AppRun())
	{
		Tick();
	}
};

void Engine::Tick()
{
	GameTick();
	RenderTick();
}

void Engine::EngineDestroy() {

	PostQuitMessage(0);
	delete mEngine;
};

void Engine::GuardedMain()
{
	if (!InitEngine())
	{
		return;
	}

	EngineLoop();
	EngineDestroy();
}

//InitEngine
bool Engine::InitDX() {
	return 	mRenderer.InitRenderer();
};


bool Engine::InitWindow() {


	mWindow = std::make_shared<Win32Window>();

	return std::dynamic_pointer_cast<Win32Window>(mWindow)->InitWindow();

	//return dynamic_cast<Win32Window*>(&mWindow)->InitWindow();
};

//EngineLoop
void Engine::GameTick() {
	mRenderer.Update();

};
void Engine::RenderTick() {
	mRenderer.Draw();
};


bool Engine::AppRun()
{
	bool quit = false;
	MSG msg = { 0 };

	mTimer.Reset();

		// If there are Window messages then process them.
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message==WM_QUIT)
			{
				quit = true;
			}

		}

	return !quit;
}
