#include "stdafx.h"
#include "WindowsTopApp.h"

WindowsTopApp::WindowsTopApp()
{
	mEngine = std::make_shared<Engine>();
	mGameLogic= std::make_shared<GameLogic>();
}

void WindowsTopApp::Init(HINSTANCE hInstance)
{
	if (!mEngine->InitEngine(hInstance))
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
	while (mEngine->mAppPaused && std::dynamic_pointer_cast<Win32Window>(mEngine->mWindow)->Run())
	{
		mEngine->Tick();
		mGameLogic->Tick();
	}
}

void WindowsTopApp::Destroy()
{
	mEngine->EngineDestroy();
}
