#pragma once
#include "MyStruct.h"
class FLight
{
public:
	LightInfo mLightInfo;
public:
	void LoadLightFromBat(const std::string& filepath);
};

