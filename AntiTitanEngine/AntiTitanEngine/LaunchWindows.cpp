#include "stdafx.h"

#if WIN32

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
		//WindowsApp theApp(hInstance);
		//if (!theApp.Initialize()) { return 0; };
		//return theApp.Run();
	
	WindowsTopApp app;
	app.Init(hInstance);
	app.Run();
	app.Destroy();

	return 0;
}

#endif