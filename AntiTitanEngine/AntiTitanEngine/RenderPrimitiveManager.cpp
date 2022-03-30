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

void RenderPrimitiveManager::InsertHeapToHeapLib(std::string heapname, std::shared_ptr<RHIResource_Heap> heap)
{
	mHeapsLib.insert(std::pair<std::string, std::shared_ptr<RHIResource_Heap>>(heapname, heap));
}
