#include "stdafx.h"
#include "WindowsTopApp.h"

WindowsTopApp::WindowsTopApp()
{
	mGameLogic = std::make_shared<GameLogic>();
}

void WindowsTopApp::Init(HINSTANCE hInstance)
{

	//mGameLogic->LoadMap("MapActorInfo / MapActorInfo.bat");
	//Engine 

	if (!Engine::Get()->InitEngine(hInstance))
	{
		return;
	}
	if (!mGameLogic->InitGameLogic())
	{
		return;
	}
}

void WindowsTopApp::Run()
{
	auto t0 = Engine::Get();
	auto t = Engine::Get()->GetWindow();
	auto t1 = Engine::Get()->mAppPaused;

	while (Engine::Get()->mAppPaused && std::dynamic_pointer_cast<Win32Window>(Engine::Get()->GetWindow())->Run())
	{
		Engine::Get()->Tick();
		mGameLogic->Tick();
	}
}

void WindowsTopApp::Destroy()
{
	Engine::Get()->EngineDestroy();

}
