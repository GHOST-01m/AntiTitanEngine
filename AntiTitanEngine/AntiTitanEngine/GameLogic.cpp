#include "stdafx.h"
#include "GameLogic.h"

bool GameLogic::InitGameLogic()
{
	Engine::Get()->GetRenderer()->GetCamera()->SetPosition(300.0f, -3000.0f, 1000.0f);
	Engine::Get()->GetRenderer()->GetCamera()->MoveSpeed = 0.5;

	//Engine::Get()->GetAssetManager()->LoadExternalMapActor("MapActorInfo/MapActorInfo.bat");
	Engine::Get()->GetAssetManager()->mLight = std::make_shared<FLight>();
	Engine::Get()->GetAssetManager()->mLight->LoadLightFromBat("MapLightInfo/MapLightInfo.bat");
	Engine::Get()->GetAssetManager()->mLight->InitView();
	Engine::Get()->GetAssetManager()->mLight->InitProj();

	return true;
}

void GameLogic::Tick() {
	//Engine::Get()->GetAssetManager()->mLight->Yaw(0.0005f);
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
		if (GetAsyncKeyState(0x31) & 0x8000) {
			Engine::Get()->GetRenderer()->GetCamera()->MoveSpeed += 0.01f;
		}
		if (GetAsyncKeyState(0x32) & 0x8000) {
			Engine::Get()->GetRenderer()->GetCamera()->MoveSpeed -= 0.01f;
		}
		if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
			Engine::Get()->GetAssetManager()->mLight->Yaw(0.002f);
		}
		Engine::Get()->GetRenderer()->GetCamera()->UpdateViewMatrix();
		//MOUSEEVENTF_MOVE
}
