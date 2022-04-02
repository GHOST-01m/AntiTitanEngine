#include "stdafx.h"
#include "RenderPrimitiveManager.h"

Primitive_MeshBuffer::~Primitive_MeshBuffer()
{

}

int RenderPrimitiveManager::GetMeshKeyByName(std::string name)
{
	for (auto it = MeshMap.begin(); it != MeshMap.end(); it++)
	{
		//MapofGeosMeshͨ��ӳ���ҵ�MapActor�����ֶ�Ӧ��Geos�е�key
		if (it->second == name) {
			return it->first;
		}
	}

	return -1;//���ִ�е�������,Ӧ���׳�һ���쳣��ʾû�д�Map���ҵ���Ӧ���ֵļ�����
}

std::shared_ptr<Primitive_Heap> RenderPrimitiveManager::GetHeapByName(std::string name)
{
	return mHeapsLib.at(name);
}

std::shared_ptr<Primitive_Texture> RenderPrimitiveManager::GetTextureByName(std::string name)
{
	return mTextureLib.at(name);
}

std::shared_ptr<Primitive_Pipeline> RenderPrimitiveManager::GetPipelineByName(std::string name)
{
	return mPipelineLib.at(name);
}

std::shared_ptr<Primitive_Shader> RenderPrimitiveManager::GetShaderByName(std::string name)
{
	return mShaderLib.at(name);

}

std::shared_ptr<Primitive_RenderTarget> RenderPrimitiveManager::GetRenderTargetByName(std::string name)
{
	return mRenderTargetLib.at(name);
}

void RenderPrimitiveManager::InsertTextureToLib(std::string name , std::shared_ptr<Primitive_Texture>texture)
{
	mTextureLib.insert(std::pair<std::string, std::shared_ptr<Primitive_Texture>>(name, texture));

}

void RenderPrimitiveManager::InsertHeapToLib(std::string heapname, std::shared_ptr<Primitive_Heap> heap)
{
	mHeapsLib.insert(std::pair<std::string, std::shared_ptr<Primitive_Heap>>(heapname, heap));
}

void RenderPrimitiveManager::InsertPipelineToLib(std::string name, std::shared_ptr<Primitive_Pipeline>pipeline)
{
	mPipelineLib.insert(std::pair<std::string, std::shared_ptr<Primitive_Pipeline>>(name, pipeline));
}

void RenderPrimitiveManager::InsertShaderToLib(std::string name, std::shared_ptr<Primitive_Shader> shader)
{
	mShaderLib.insert(std::pair<std::string, std::shared_ptr<Primitive_Shader>>(name, shader));
}

void RenderPrimitiveManager::InsertRenderTargetToLib(std::string name, std::shared_ptr<Primitive_RenderTarget>RenderTarget)
{
	mRenderTargetLib.insert(std::pair<std::string, std::shared_ptr<Primitive_RenderTarget>>(name, RenderTarget));
}
