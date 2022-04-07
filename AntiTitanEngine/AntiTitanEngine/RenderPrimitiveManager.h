#pragma once
#include "MyStruct.h"
#include "map"
#include "Primitive_MeshBuffer.h"
#include "Primitive_Shader.h"
#include "Primitive_Texture.h"
#include "Primitive_Heap.h"
#include "Primitive_RenderTarget.h"
#include "Primitive_Pipeline.h"
#include "Primitive_GPUResource.h"

class RenderPrimitiveManager
{
public:
	//std::vector<std::shared_ptr<Primitive_MeshBuffer>> MeshBuffers;
	//std::map<int,std::string> MeshMap;

	//std::map<std::string, std::shared_ptr<Primitive_Texture>>         mTextureLib;
	std::map<std::string, std::shared_ptr<Primitive_Heap>>            mHeapsLib;
	std::map<std::string, std::shared_ptr<Primitive_Shader>>          mShaderLib;
	std::map<std::string, std::shared_ptr<Primitive_Pipeline>>        mPipelineLib;
	std::map<std::string, std::shared_ptr<Primitive_RenderTarget>>    mRenderTargetLib;

public:
	//int GetMeshKeyByName(std::string name);
	//std::shared_ptr<Primitive_Texture>        GetTextureByName(std::string name);
	std::shared_ptr<Primitive_Heap>           GetHeapByName(std::string name);
	std::shared_ptr<Primitive_Pipeline>       GetPipelineByName(std::string name);
	std::shared_ptr<Primitive_Shader>         GetShaderByName(std::string name);
	std::shared_ptr<Primitive_RenderTarget>   GetRenderTargetByName(std::string name);

	//void InsertTextureToLib(std::string name, std::shared_ptr<Primitive_Texture>texture);
	void InsertHeapToLib(std::string, std::shared_ptr<Primitive_Heap>);
	void InsertPipelineToLib(std::string, std::shared_ptr<Primitive_Pipeline>);
	void InsertShaderToLib(std::string, std::shared_ptr<Primitive_Shader>);
	void InsertRenderTargetToLib(std::string, std::shared_ptr<Primitive_RenderTarget>);

};