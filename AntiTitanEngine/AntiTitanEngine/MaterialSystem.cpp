#include "stdafx.h"
#include "MaterialSystem.h"
#include "Common/DDSTextureLoader.h"

int MaterialSystem::GetTextureNum()
{
	return mTextureNum;
}

std::shared_ptr<Texture> MaterialSystem::GetTexture()
{
	return mTexture;
}

void MaterialSystem::LoadTexture()
{
	mTexture = std::make_shared<Texture>();
	mTexture->Name = "TestTexture";
	mTexture->Filename = L"Texture/SkyShpere.dds";
	//testTexture->Filename = L"E:/DX12Homework/AntiTitanEngine/AntiTitanEngine/AntiTitanEngine/AntiTitanEngine/TextureddsTest.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(Engine::Get()->GetRenderer()->md3dDevice.Get(),
		Engine::Get()->GetRenderer()->mCommandList.Get(), mTexture->Filename.c_str(),
		mTexture->Resource, mTexture->UploadHeap));
}
