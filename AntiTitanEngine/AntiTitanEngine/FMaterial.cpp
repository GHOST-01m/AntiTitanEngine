#include "stdafx.h"
#include "FMaterial.h"

std::shared_ptr<Primitive_Texture> FMaterial::GetTextureByName(std::string name)
{
	return textureLib.at(name);
}

void FMaterial::InsertTextureToTexLib(std::string name, std::shared_ptr<Primitive_Texture>texture)
{
	textureLib.insert(std::pair<std::string, std::shared_ptr<Primitive_Texture>>(name, texture));
}
