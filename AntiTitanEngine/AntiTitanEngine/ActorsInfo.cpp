#include "stdafx.h"
#include "ActorsInfo.h"

void ActorsInfo::SetSceneActorsInfoFromBat(const std::string& StaticMeshPath) {

	//std::string test = "E:/DX12Homework/AntiTitanEngine/AntiTitanEngine/AntiTitanEngine/AntiTitanEngine/MapActorInfo/MapActorInfo.bat";
	std::string test = "MapActorInfo/MapActorInfo.bat";
		
	std::ifstream BatFile(StaticMeshPath, std::ios::in | std::ios::binary);

	//std::ifstream BatFile(test, std::ios::in | std::ios::binary);

	if (BatFile.is_open())
	{
		int a=0;
	}

	int32_t DataLength;
	int32_t DataStringLength;

	std::vector<int32_t> pos;

	//ActorNameArray
	BatFile.read((char*)&DataLength, sizeof(int32_t));
	ActorNameArray.resize(DataLength);
	for (int i=0;i< DataLength;i++)
	{
		BatFile.read((char*)&DataStringLength, sizeof(int32_t));
		ActorNameArray[i].resize(DataStringLength);
		BatFile.read((char*)ActorNameArray[i].data(), sizeof(char) * DataStringLength);
	}
	pos.push_back(static_cast<unsigned int>(BatFile.tellg()));

	//MeshNameArray
	BatFile.read((char*)&DataLength, sizeof(int32_t));
	MeshNameArray.resize(DataLength);
	for (int i = 0; i < DataLength; i++)
	{
		BatFile.read((char*)&DataStringLength, sizeof(int32_t));
		MeshNameArray[i].resize(DataStringLength);
		BatFile.read((char*)MeshNameArray[i].data(), sizeof(char) * DataStringLength);
	}
	pos.push_back(static_cast<unsigned int>(BatFile.tellg()));

	//ActorsTransformArray
	BatFile.read((char*)&DataLength, sizeof(int32_t));
	ActorsTransformArray.resize(DataLength);
	BatFile.read((char*)ActorsTransformArray.data(), sizeof(Transform) * DataLength);
	pos.push_back(static_cast<unsigned int>(BatFile.tellg()));

	//ActorsQuatArray
	BatFile.read((char*)&DataLength, sizeof(int32_t));
	ActorsQuatArray.resize(DataLength);
	BatFile.read((char*)ActorsQuatArray.data(), sizeof(Quat) * DataLength);
	pos.push_back(static_cast<unsigned int>(BatFile.tellg()));

	BatFile.close();
}