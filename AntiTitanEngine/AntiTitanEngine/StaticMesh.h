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

	operator XMFLOAT3() const {
		return { X,Y,Z };
	}
};
struct FVector4
{
	float X;
	float Y;
	float Z;
	float W;

	operator XMFLOAT4() const {
		return { X,Y,Z,W };
	}

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
	std::vector<FVector4> MeshVertexNormalInfo;
};

class StaticMesh 
{
//New
public:
	StaticMeshInfo MeshInfo;

public:
	void SetStaticMeshFromBat(const std::string& filepath);
	void SetNull();
public:
	std::string getMeshName();
};