#include "stdafx.h"
#include "ActorsInfo.h"

void ActorsInfo::SetSceneActorsInfoFromBat(const std::string& StaticMeshPath) {

		std::ifstream BatFile(StaticMeshPath, std::ios::in | std::ios::binary);
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
		pos.push_back(BatFile.tellg());

		//MeshNameArray
		BatFile.read((char*)&DataLength, sizeof(int32_t));
		MeshNameArray.resize(DataLength);
		for (int i = 0; i < DataLength; i++)
		{
			BatFile.read((char*)&DataStringLength, sizeof(int32_t));
			MeshNameArray[i].resize(DataStringLength);
			BatFile.read((char*)MeshNameArray[i].data(), sizeof(char) * DataStringLength);
		}
		pos.push_back(BatFile.tellg());


		//ActorsTransformArray
		BatFile.read((char*)&DataLength, sizeof(int32_t));
		ActorsTransformArray.resize(DataLength);
		BatFile.read((char*)ActorsTransformArray.data(), sizeof(Transform) * DataLength);
		pos.push_back(BatFile.tellg());


		//ActorsQuatArray
		BatFile.read((char*)&DataLength, sizeof(int32_t));
		ActorsQuatArray.resize(DataLength);
		BatFile.read((char*)ActorsQuatArray.data(), sizeof(Quat) * DataLength);
		pos.push_back(BatFile.tellg());

		BatFile.close();
}