#include "stdafx.h"

void StaticMesh::SetStaticMeshFromBat(const std::string& StaticMeshPath) {
	std::ifstream BatFile(StaticMeshPath, std::ios::in | std::ios::binary);
	int32_t DataLength;
	BatFile.read((char*)&DataLength, sizeof(int32_t));//Mesh名字长度
	MeshInfo.MeshName.resize(DataLength);
	BatFile.read((char*)MeshInfo.MeshName.data(), sizeof(char)*DataLength);//MeshName值
	BatFile.read((char*)&DataLength, sizeof(int32_t));//Mesh路径长度
	MeshInfo.MeshPath.resize(DataLength);
	BatFile.read((char*)MeshInfo.MeshPath.data(), sizeof(char) * DataLength);//Mesh路径值
	BatFile.read((char*)&MeshInfo.MeshVerticesNum, sizeof(int32_t));//Mesh顶点数量
	BatFile.read((char*)&MeshInfo.MeshTrianglesNum, sizeof(int32_t));//Meh三角面数量
	BatFile.read((char*)&DataLength, sizeof(int32_t));//Mesh顶点信息长度
	MeshInfo.MeshVertexInfo.resize(DataLength);
	BatFile.read((char*)MeshInfo.MeshVertexInfo.data(), sizeof(FVector) * DataLength);//Mesh顶点
	BatFile.read((char*)&DataLength, sizeof(int32_t));//Mesh索引长度
	MeshInfo.MeshIndexInfo.resize(DataLength);
	BatFile.read((char*)MeshInfo.MeshIndexInfo.data(), sizeof(int32_t)*DataLength);//Mesh索引


	BatFile.close();
}

void StaticMesh::BuildStaticMeshGeometry() {

}