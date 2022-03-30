#include "stdafx.h"

Engine* Engine::mEngine = new Engine();
std::shared_ptr<GameTimer>          Engine::mTimer = nullptr;
std::shared_ptr<Window>             Engine::mWindow = nullptr;
std::shared_ptr<Renderer>           Engine::mRenderer = nullptr;
std::shared_ptr<AssetManager>       Engine::mAssetManager = nullptr;
std::shared_ptr<MaterialSystem>     Engine::mMaterialSystem = nullptr;

Engine::Engine()
{
	mEngine = this;
}

Engine::~Engine()
{
	OutputDebugStringA("Engine::~Engine()\n");
}

Engine* Engine::Get() {
	return mEngine;
}

void Engine::ReleaseEngine()
{
	delete mEngine;
	mEngine = nullptr;
}

std::shared_ptr<GameTimer> Engine::GetGameTimer() {
	return mTimer;
};

std::shared_ptr<Window> Engine::GetWindow() {
	return mWindow;
};

std::shared_ptr<AssetManager> Engine::GetAssetManager()
{
	return mAssetManager;
}

std::shared_ptr<Renderer> Engine::GetRenderer() {
	return mRenderer;
};

std::shared_ptr<MaterialSystem> Engine::GetMaterialSystem()
{
	return mMaterialSystem;
}

//Renderer* Engine::GetRenderer() {
//	return mRenderer;
//}

bool Engine::InitEngine(HINSTANCE hInstance) {

	if (!InitWindow( hInstance)){
		return false;
	}

	mTimer              = std::make_shared<GameTimer>();
	mRenderer           = std::make_shared<Renderer>();
	mAssetManager       = std::make_shared<AssetManager>();
	mMaterialSystem     = std::make_shared<MaterialSystem>();

	mTimer->Reset();
	gt.Reset();
	if (!InitDX()){
		return false;
	}

	return true;
};

void Engine::EngineLoop() {
	//ÒÑ·ÏÆú
	//while (mAppPaused && std::dynamic_pointer_cast<Win32Window>(mWindow)->Run())
	//{
	//	Tick();
	//}
};

void Engine::Tick()
{
	gt.Tick();
	mTimer->Tick();
	mRenderer->Update();
	mRenderer->Draw();
	//mRenderer.Update();
	//mRenderer.Draw();
	//mRenderer.CalculateFrameStats();
}

void Engine::EngineDestroy() {
	//mRenderer = nullptr;
	mWindow = nullptr;
	mTimer = nullptr;
	ReleaseEngine();

	Sleep(100);
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

	return 	mRenderer->Init();
};


bool Engine::InitWindow(HINSTANCE hInstance) {
	auto Window = std::make_shared<Win32Window>();
	mWindow = Window;
	return Window->InitWindow(hInstance);
};