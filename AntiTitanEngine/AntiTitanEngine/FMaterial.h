#pragma once
#include "MyStruct.h"
#include "Primitive_Texture.h"
#include "map"

class FMaterial
{
public:
	std::map<std::string,std::shared_ptr<Primitive_Texture>> textureLib;
	std::shared_ptr<Primitive_Texture> GetTextureByName(std::string);
	void InsertTextureToTexLib(std::string, std::shared_ptr<Primitive_Texture>);


};

