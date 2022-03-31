#include "stdafx.h"
#include "RenderPrimitiveManager.h"

RHIDevice::~RHIDevice()
{

}

RHIResource_MeshBuffer::~RHIResource_MeshBuffer()
{

}

int RenderPrimitiveManager::GetMeshKeyByName(std::string name)
{
	for (auto it = MeshMap.begin(); it != MeshMap.end(); it++)
	{
		//MapofGeosMesh通过映射找到MapActor的名字对应的Geos中的key
		if (it->second == name) {
			return it->first;
		}
	}

	return -1;//如果执行到这里了,应该抛出一个异常表示没有从Map中找到对应名字的几何体
}

std::shared_ptr<RHIResource_Heap> RenderPrimitiveManager::GetHeapByName(std::string name)
{
	return mHeapsLib.at(name);
}

std::shared_ptr<RHIResource_Texture> RenderPrimitiveManager::GetTextureByName(std::string name)
{
	return mTextureLib.at(name);
}

std::shared_ptr<RHIResource_Pipeline> RenderPrimitiveManager::GetPipelineByName(std::string name)
{
	return mPipelineLib.at(name);
}

std::shared_ptr<RHIResource_Shader> RenderPrimitiveManager::GetShaderByName(std::string name)
{
	return mShaderLib.at(name);

}

void RenderPrimitiveManager::InsertTextureToLib(std::string name , std::shared_ptr<RHIResource_Texture>texture)
{
	mTextureLib.insert(std::pair<std::string, std::shared_ptr<RHIResource_Texture>>(name, texture));

}

void RenderPrimitiveManager::InsertHeapToLib(std::string heapname, std::shared_ptr<RHIResource_Heap> heap)
{
	mHeapsLib.insert(std::pair<std::string, std::shared_ptr<RHIResource_Heap>>(heapname, heap));
}

void RenderPrimitiveManager::InsertPipelineToLib(std::string name, std::shared_ptr<RHIResource_Pipeline>pipeline)
{
	mPipelineLib.insert(std::pair<std::string, std::shared_ptr<RHIResource_Pipeline>>(name, pipeline));
}

void RenderPrimitiveManager::InsertShaderToLib(std::string name, std::shared_ptr<RHIResource_Shader> shader)
{
	mShaderLib.insert(std::pair<std::string, std::shared_ptr<RHIResource_Shader>>(name, shader));
}
