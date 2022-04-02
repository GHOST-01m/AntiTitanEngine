#pragma once
#include "MyStruct.h"
#include "map"
#include "RHIFactory.h"
#include "RHIDevice.h"
#include "RHIResource_MeshBuffer.h"
#include "RHIResource_Shader.h"
#include "RHIResource_Texture.h"
#include "RHIResource_ShadowMap.h"
#include "RHIResource_Heap.h"
#include "RHIResource_RenderTarget.h"
#include "RHIResource_Pipeline.h"
#include "RHIResource_GPUResource.h"

class RenderPrimitiveManager
{
public:
	std::shared_ptr<RHIFactory> mRHIFactory;
	std::shared_ptr<RHIDevice> mRHIDevice;

public:
	std::vector<std::shared_ptr<RHIResource_MeshBuffer>> VBIBBuffers;
	std::map<int,std::string> MeshMap;
	std::shared_ptr<RHIResource_Shader> mShader;

	std::map<std::string, std::shared_ptr<RHIResource_Texture>>         mTextureLib;
	std::map<std::string, std::shared_ptr<RHIResource_Heap>>            mHeapsLib;
	std::map<std::string, std::shared_ptr<RHIResource_Shader>>          mShaderLib;
	std::map<std::string, std::shared_ptr<RHIResource_Pipeline>>        mPipelineLib;
	std::map<std::string, std::shared_ptr<RHIResource_RenderTarget>>    mRenderTargetLib;
	
	//std::shared_ptr<RHIResource_ShadowMap> mShadowMap;
	//std::shared_ptr<RHIResource_RenderTarget> mRenderTarget;

public:
	int GetMeshKeyByName(std::string name);
	std::shared_ptr<RHIResource_Heap>           GetHeapByName(std::string name);
	std::shared_ptr<RHIResource_Texture>        GetTextureByName(std::string name);
	std::shared_ptr<RHIResource_Pipeline>       GetPipelineByName(std::string name);
	std::shared_ptr<RHIResource_Shader>         GetShaderByName(std::string name);
	std::shared_ptr<RHIResource_RenderTarget>   GetRenderTargetByName(std::string name);


	void InsertTextureToLib(std::string name, std::shared_ptr<RHIResource_Texture>texture);
	void InsertHeapToLib(std::string, std::shared_ptr<RHIResource_Heap>);
	void InsertPipelineToLib(std::string, std::shared_ptr<RHIResource_Pipeline>);
	void InsertShaderToLib(std::string, std::shared_ptr<RHIResource_Shader>);
	void InsertRenderTargetToLib(std::string, std::shared_ptr<RHIResource_RenderTarget>);

};