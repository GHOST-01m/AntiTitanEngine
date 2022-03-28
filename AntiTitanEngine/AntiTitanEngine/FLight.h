#pragma once
#include "MyStruct.h"
class FLight
{
public:
	FLight();
public:
	LightInfo mLightInfo;
	float mLightNearZ = 0.0f;
	float mLightFarZ = 0.0f;

	DirectX::XMFLOAT3 mRight = { 0.0f, -1.0f, 0.0f };
	DirectX::XMFLOAT3 mUp = { 0.0f, 0.0f, -1.0f };
	DirectX::XMFLOAT3 mLook = { -1.0f, 0.0f, 0.0f };

	DirectX::XMFLOAT4X4 mView = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 mProj = MathHelper::Identity4x4();
public:
	DirectX::BoundingSphere mSceneBounds;

public:
	void LoadLightFromBat(const std::string& filepath);
	void InitView();
	void InitProj();
	XMMATRIX GetView();
	XMMATRIX GetProj();

	//Ðý×ª
	void Pitch(float angle);
	void Yaw(float angle);
	void Roll(float angle);
};

