#pragma once
#include "Primitive_MeshBuffer.h"
#include "MyStruct.h"
#include "FMaterial.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

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
	std::vector<FVector2D> MeshTexCoord;
};

class StaticMesh 
{
public:
	StaticMeshInfo MeshInfo;
	std::shared_ptr<Primitive_MeshBuffer> meshBuffer;
	std::shared_ptr<FMaterial> material;

public:
	void LoadStaticMeshFromBat(const std::string& filepath);
	void SetMeshBuffer(std::shared_ptr<Primitive_MeshBuffer>);

public:

public:
	std::string GetMeshName();
};