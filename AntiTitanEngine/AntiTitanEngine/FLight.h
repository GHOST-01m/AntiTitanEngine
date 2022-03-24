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

public:
	DirectX::BoundingSphere mSceneBounds;

public:
	void LoadLightFromBat(const std::string& filepath);
};

