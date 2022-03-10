#include "stdafx.h"
#include "WindowsTopApp.h"

WindowsTopApp::WindowsTopApp()
{
	mGameLogic = std::make_shared<GameLogic>();
}

void WindowsTopApp::Init(HINSTANCE hInstance)
{

	mGameLogic->LoadMap("MapActorInfo/MapActorInfo.bat");
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
