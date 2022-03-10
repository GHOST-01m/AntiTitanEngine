#pragma once
#include "TopAPP.h"
class WindowsTopApp:public TopAPP
{
public:
	WindowsTopApp();

public:
	void Init(HINSTANCE hInstance);
	void Run();
	void Destroy();

public:
	std::shared_ptr<GameLogic> mGameLogic;

};

