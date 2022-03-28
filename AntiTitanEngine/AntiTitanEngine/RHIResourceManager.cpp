#include "stdafx.h"
#include "RHIResourceManager.h"

RHIDevice::~RHIDevice()
{

}

RHIResource_VBIBBuffer::~RHIResource_VBIBBuffer()
{

}

int RHIResourceManager::GetKeyByName(std::string name)
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

std::shared_ptr<RHIResource_Heap> RHIResourceManager::GetHeapByName(std::string name)
{
	return mHeapsLib.at(name);
}
