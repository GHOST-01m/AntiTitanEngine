#include "stdafx.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
//	// Enable run-time memory check for debug builds.
//#if defined(DEBUG) | defined(_DEBUG)
//	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
//#endif
//
//	try
//	{
//		WindowsApp theApp(hInstance);
//		//BoxApp theApp(hInstance);
//		if (!theApp.Initialize())
//			return 0;
//
//		//theApp.LogAdapters();
//		return theApp.Run();
//	}
//	catch (DxException& e)
//	{
//		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
//		return 0;
//	}

WindowsApp theApp(hInstance);
//BoxApp theApp(hInstance);
if (!theApp.Initialize())
return 0;

//theApp.LogAdapters();
return theApp.Run();
}