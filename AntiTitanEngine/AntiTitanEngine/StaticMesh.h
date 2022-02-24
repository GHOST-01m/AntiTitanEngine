#pragma once
#include "stdafx.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

struct FVector
{
	float X;
	float Y;
	float Z;
};

struct StaticMeshInfo
{
public:
	std::string MeshName;
	std::string MeshPath;
	int MeshVerticesNum;
	int MeshTrianglesNum;
	std::vector<FVector> MeshVertexInfo;
	std::vector<int32_t> MeshIndexInfo;

};

class StaticMesh 
{
//New
public:
	StaticMeshInfo MeshInfo;

public:
	void SetStaticMeshFromBat(const std::string& filepath);
	void BuildStaticMeshGeometry();
};