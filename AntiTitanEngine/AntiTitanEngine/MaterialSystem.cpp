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
	
	//ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(Engine::Get()->GetRenderer()->md3dDevice.Get(),
	//	Engine::Get()->GetRenderer()->mCommandList.Get(), mTexture->Filename.c_str(),
	//	mTexture->Resource, mTexture->UploadHeap));

	auto device=std::dynamic_pointer_cast<DXRHI>(Engine::Get()->GetRenderer()->GetRHI())->Getd3dDevice();
	auto commandlist = std::dynamic_pointer_cast<DXRHI>(Engine::Get()->GetRenderer()->GetRHI())->GetCommandList();

	
		//xie dao RHI li mian!!!!!!!!!!!!!!!!!!!!!!!!!
		//xie dao RHI li mian!!!!!!!!!!!!!!!!!!!!!!!!!
		//xie dao RHI li mian!!!!!!!!!!!!!!!!!!!!!!!!!
		//xie dao RHI li mian!!!!!!!!!!!!!!!!!!!!!!!!!

	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(device.Get(),
		commandlist.Get(), mTexture->Filename.c_str(),
		mTexture->Resource, mTexture->UploadHeap));
}
