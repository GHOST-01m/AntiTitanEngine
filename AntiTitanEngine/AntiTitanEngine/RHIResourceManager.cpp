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
		//MapofGeosMeshͨ��ӳ���ҵ�MapActor�����ֶ�Ӧ��Geos�е�key
		if (it->second == name) {
			return it->first;
		}
	}

	return -1;//���ִ�е�������,Ӧ���׳�һ���쳣��ʾû�д�Map���ҵ���Ӧ���ֵļ�����
}

std::shared_ptr<RHIResource_Heap> RHIResourceManager::GetHeapByName(std::string name)
{
	return mHeapsLib.at(name);
}
