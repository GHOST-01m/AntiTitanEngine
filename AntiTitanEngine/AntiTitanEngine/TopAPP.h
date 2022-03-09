#pragma once
#include "Engine.h"
#include "GameLogic.h"
class TopAPP
{
	
public:
	void Init();
	void Run();
	void Destroy();

	Engine mEngine;
	GameLogic mGameLogic;
};