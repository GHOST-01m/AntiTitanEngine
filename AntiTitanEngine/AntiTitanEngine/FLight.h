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

	DirectX::XMFLOAT3 mRight = { -1.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 mUp = { 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT3 mLook = { 0.0f, 1.0f, 0.0f };

	DirectX::XMFLOAT4X4 mView = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 mProj = MathHelper::Identity4x4();
public:
	DirectX::BoundingSphere mSceneBounds;

public:
	void LoadLightFromBat(const std::string& filepath);
	XMMATRIX GetView();
	XMMATRIX GetProj(XMVECTOR targetPos);
};

