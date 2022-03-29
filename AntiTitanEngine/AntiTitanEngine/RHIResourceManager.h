#pragma once
#include "MyStruct.h"
#include "map"
#include "RHIFactory.h"
#include "RHIDevice.h"
#include "RHIResource_VBIBBuffer.h"
#include "RHIResource_Shader.h"
#include "RHIResource_Texture.h"
#include "RHIResource_ShadowMap.h"
#include "RHIResource_Heap.h"

class RHIResourceManager//������RendererPrimitiveManager �������Ա��RHI���õ�Renderer��
{
public:
	std::shared_ptr<RHIFactory> mRHIFactory;
	std::shared_ptr<RHIDevice> mRHIDevice;

public:
	std::vector<std::shared_ptr<RHIResource_VBIBBuffer>> VBIBBuffers;
	std::map<int,std::string> MeshMap;
	std::vector<std::shared_ptr<RHIResource_Texture>> mTextures;
	std::map<int, std::string> TextureMap;
	std::shared_ptr<RHIResource_Shader> mShader;
	std::shared_ptr<RHIResource_ShadowMap> mShadowMap;
	std::map<std::string,std::shared_ptr<RHIResource_Heap>> mHeapsLib;

public:
	int GetKeyByName(std::string name);
	std::shared_ptr<RHIResource_Heap> GetHeapByName(std::string name);
};