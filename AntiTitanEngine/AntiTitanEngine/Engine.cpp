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

bool Engine::InitEngine(HINSTANCE hInstance) {

	if (mEngine == nullptr)
	{
		mEngine = new Engine;
	}

	if (!InitWindow( hInstance))
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

void Engine::GuardedMain(HINSTANCE hInstance)
{
	if (!InitEngine(hInstance))
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


bool Engine::InitWindow(HINSTANCE hInstance) {

	Win32Window a;
	a.InitWindow(hInstance);
	mWindow = std::make_shared<Win32Window>(a);
	
	return std::dynamic_pointer_cast<Win32Window>(mWindow)->InitWindow(hInstance);

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
