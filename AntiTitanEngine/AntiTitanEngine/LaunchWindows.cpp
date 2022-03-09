#include "stdafx.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{

	
	WindowsTopApp app;
	app.Init(hInstance);
	app.Run();
	app.Destroy();

	return 0;
}