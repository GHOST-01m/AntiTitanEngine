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
		//MapofGeosMeshͨ��ӳ���ҵ�MapActor�����ֶ�Ӧ��Geos�е�key
		if (it->second == name) {
			return it->first;
		}
	}

	return -1;//���ִ�е�������,Ӧ���׳�һ���쳣��ʾû�д�Map���ҵ���Ӧ���ֵļ�����
}

std::shared_ptr<RHIResource_Heap> RenderPrimitiveManager::GetHeapByName(std::string name)
{
	return mHeapsLib.at(name);
}

void RenderPrimitiveManager::InsertHeapToHeapLib(std::string heapname, std::shared_ptr<RHIResource_Heap> heap)
{
	mHeapsLib.insert(std::pair<std::string, std::shared_ptr<RHIResource_Heap>>(heapname, heap));
}
