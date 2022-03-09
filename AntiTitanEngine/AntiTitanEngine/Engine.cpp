#include "stdafx.h"

Engine* Engine::mEngine = nullptr;
std::shared_ptr<Window> Engine::mWindow = nullptr;
std::shared_ptr<Renderer> Engine::mRenderer = nullptr;

Engine::Engine()
{
//	assert(mEngine == nullptr);
//	mEngine = this;
}

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

std::shared_ptr<Renderer> Engine::GetRenderer() {
	return mRenderer;
};

//Renderer* Engine::GetRenderer() {
//	return mRenderer;
//}

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
	mRenderer = std::make_shared<Renderer>();

	if (!InitDX())
	{
		return false;
	}

	mTimer.Reset();

	return true;
};

void Engine::EngineLoop() {

	while (mAppPaused && std::dynamic_pointer_cast<Win32Window>(mWindow)->Run())
	{
		Tick();
	}
};

void Engine::Tick()
{
	mTimer.Tick();
	mRenderer->Update();
	mRenderer->Draw();
	mRenderer->CalculateFrameStats();
	//mRenderer.Update();
	//mRenderer.Draw();
	//mRenderer.CalculateFrameStats();
}

void Engine::EngineDestroy() {
	delete mEngine;
	PostQuitMessage(0);
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

	//Renderer r;
	//GameTimer gt;
	//mTimer = gt;
	//mRenderer = new Renderer;
	return 	mRenderer->InitRenderer();

};


bool Engine::InitWindow(HINSTANCE hInstance) {

	Win32Window a;
	//a.InitWindow(hInstance);
	
	mWindow = std::make_shared<Win32Window>(a);
	
	return std::dynamic_pointer_cast<Win32Window>(mWindow)->InitWindow(hInstance);

	//return dynamic_cast<Win32Window*>(&mWindow)->InitWindow();
};


