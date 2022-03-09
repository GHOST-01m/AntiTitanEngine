#pragma once
#include "Engine.h"
#include "InputGameLogic.h"
class GameLogic
{
public:
	bool InitGameLogic();
	void Tick();

public:
	void LoadMap(std::string path);

	void MoveCamera();
};

