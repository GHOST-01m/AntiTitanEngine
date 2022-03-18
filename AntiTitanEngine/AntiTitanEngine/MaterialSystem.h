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

private:
	std::shared_ptr<Texture> mTexture;
	int mTextureNum = 1;
};