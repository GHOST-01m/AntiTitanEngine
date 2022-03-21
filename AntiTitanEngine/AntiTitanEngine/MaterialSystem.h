#pragma once
#include "MyStruct.h"
#include "Engine.h"
#include "Common/DDSTextureLoader.h"

class MaterialSystem
{
public:
	int GetTextureNum();

public:

	std::shared_ptr<Texture> GetTexture();
	void LoadTexture();

	std::shared_ptr<Texture> mTexture;
	int mTextureNum = 1;
	std::shared_ptr<Material> mMaterial;
};