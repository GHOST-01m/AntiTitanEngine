#include "stdafx.h"
#include "GameLogic.h"

bool GameLogic::InitGameLogic()
{
	Engine::Get()->GetRenderer()->GetCamera()->SetPosition(300.0f, -3000.0f, 1000.0f);
	Engine::Get()->GetRenderer()->GetCamera()->MoveSpeed = 2;

	return true;
}

void GameLogic::Tick() {

	MoveCamera();
};

void GameLogic::LoadMap(std::string path)
{
	Engine::Get()->MapPath = path;
}

void GameLogic::MoveCamera()
{
		const float dt = 5;

		if (GetAsyncKeyState('W') & 0x8000) {
			Engine::Get()->GetRenderer()->GetCamera()->Walk(Engine::Get()->GetRenderer()->GetCamera()->MoveSpeed * dt);
		}
		if (GetAsyncKeyState('S') & 0x8000) {
			Engine::Get()->GetRenderer()->GetCamera()->Walk(-(Engine::Get()->GetRenderer()->GetCamera()->MoveSpeed) * dt);
		}
		if (GetAsyncKeyState('A') & 0x8000) {
			Engine::Get()->GetRenderer()->GetCamera()->Strafe(-(Engine::Get()->GetRenderer()->GetCamera()->MoveSpeed) * dt);
		}
		if (GetAsyncKeyState('D') & 0x8000) {
			Engine::Get()->GetRenderer()->GetCamera()->Strafe(Engine::Get()->GetRenderer()->GetCamera()->MoveSpeed * dt);
		}
		if (GetAsyncKeyState('Q') & 0x8000) {
			Engine::Get()->GetRenderer()->GetCamera()->Fly(-(Engine::Get()->GetRenderer()->GetCamera()->MoveSpeed) * dt);
		}
		if (GetAsyncKeyState('E') & 0x8000) {
			Engine::Get()->GetRenderer()->GetCamera()->Fly(Engine::Get()->GetRenderer()->GetCamera()->MoveSpeed * dt);
		}
		Engine::Get()->GetRenderer()->GetCamera()->UpdateViewMatrix();

}
