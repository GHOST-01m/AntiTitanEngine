#pragma once
#include "string"
#include "vector"
#include "MyStruct.h"
#include "StaticMesh.h"
class Actor
{
public:
	std::string actorName;
	std::string staticmeshName;
	Transform transform;
	Quat quat;

public:
	int CBVoffset = -1;
};