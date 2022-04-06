#pragma once
#include "string"
#include "vector"
#include "MyStruct.h"

class MapActorsInfo
{
public:
	std::vector<std::string> ActorNameArray;

	std::vector<std::string> MeshNameArray;

	std::vector<Transform> ActorsTransformArray;

	std::vector<Quat> ActorsQuatArray;

public:
	void SetSceneActorsInfoFromBat(const std::string& filepath);

	int Size() {
		int size;
		size = int(ActorNameArray.size());
		return size;
	}
};

