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

	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };//这个实际上没用上，漫反射直接用原本的颜色
	DirectX::XMFLOAT3 FresnelR0 = { 0.17f, 0.17f, 0.17f };
	float Roughness = 0.5f;
};

