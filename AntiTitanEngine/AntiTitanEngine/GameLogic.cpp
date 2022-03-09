#include "stdafx.h"
#include "GameLogic.h"

bool GameLogic::InitGameLogic()
{
	Engine::Get()->GetRenderer()->GetCamera()->SetPosition(300.0f, -3000.0f, 1000.0f);
	Engine::Get()->GetRenderer()->GetCamera()->MoveSpeed = 1.0;

	return true;
}

void GameLogic::Tick() {


};