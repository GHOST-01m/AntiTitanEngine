#include "stdafx.h"

Engine* Engine::mEngine = new Engine();
std::shared_ptr<GameTimer>   Engine::mTimer = nullptr;
std::shared_ptr<Window>      Engine::mWindow = nullptr;
std::shared_ptr<Renderer>    Engine::mRenderer = nullptr;
std::shared_ptr<Asset>       Engine::mAsset = nullptr;


Engine::Engine()
{
	mEngine = this;
}

Engine::~Engine()
{
	delete mEngine;
	OutputDebugStringA("Engine::~Engine()\n");
}

Engine* Engine::Get() {
	return mEngine;
}

std::shared_ptr<GameTimer> Engine::GetGameTimer() {
	return mTimer;
};

std::shared_ptr<Window> Engine::GetWindow() {
	return mWindow;
};

std::shared_ptr<Asset> Engine::GetAsset()
{
	return mAsset;
}

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


	if (!InitWindow( hInstance))
	{
		return false;
	}
	/*std::shared_ptr<Renderer> test = std::make_shared<Renderer>();
	bool cool = (test == nullptr);*/
	mTimer = std::make_shared<GameTimer>();
	mRenderer = std::make_shared<Renderer>();
	mAsset = std::make_shared<Asset>();
	if (!InitDX())
	{
		return false;
	}

	mTimer->Reset();

	return true;
};

void Engine::EngineLoop() {

	//while (mAppPaused && std::dynamic_pointer_cast<Win32Window>(mWindow)->Run())
	//{
	//	Tick();
	//}
};

void Engine::Tick()
{
	mTimer->Tick();
	mRenderer->Update();
	mRenderer->Draw();
	mRenderer->CalculateFrameStats();
	//mRenderer.Update();
	//mRenderer.Draw();
	//mRenderer.CalculateFrameStats();
}

void Engine::EngineDestroy() {
	mRenderer = nullptr;
	mEngine = nullptr;
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

	//Win32Window a;
	//a.InitWindow(hInstance);
	
	auto Window = std::make_shared<Win32Window>();
	mWindow = Window;
	return Window->InitWindow(hInstance);

	//return dynamic_cast<Win32Window*>(&mWindow)->InitWindow();
};


