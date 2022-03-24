#include "stdafx.h"
#include "FLight.h"

FLight::FLight()
{
	mSceneBounds.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
	mSceneBounds.Radius = sqrtf(1000.0f * 1000.0f + 500.0f * 500.0f);
}

void FLight::LoadLightFromBat(const std::string& filepath)
{
	std::ifstream BatFile(filepath, std::ios::in | std::ios::binary);
	int32_t DataLength;

	BatFile.read((char*)&DataLength, sizeof(int32_t));//第一个是Light数量
	BatFile.read((char*)&DataLength, sizeof(int32_t));
	mLightInfo.ActorName.resize(DataLength);
	BatFile.read((char*)mLightInfo.ActorName.data(), sizeof(char) * DataLength);
	BatFile.read((char*)&mLightInfo.mTransform, sizeof(Transform) );
	BatFile.read((char*)&mLightInfo.Rotation, sizeof(Float3));
	BatFile.read((char*)&mLightInfo.Intensity, sizeof(float));
	BatFile.read((char*)&mLightInfo.Direction, sizeof(FVector));
	BatFile.read((char*)&mLightInfo.LinearColor, sizeof(Color));//Mesh索引长度

	BatFile.close();
}
