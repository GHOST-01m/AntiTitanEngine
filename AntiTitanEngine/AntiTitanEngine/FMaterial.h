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

	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 FresnelR0 = { 0.09f, 0.09f, 0.09f };
	float Roughness = 0.1f;
};

