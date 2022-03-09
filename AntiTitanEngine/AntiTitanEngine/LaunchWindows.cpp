#include "stdafx.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	std::shared_ptr<WindowsTopApp> app=std::make_shared<WindowsTopApp>();
	app->Init(hInstance);
	app->Run();
	app->Destroy();

}