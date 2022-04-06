#include "stdafx.h"
void StaticMesh::LoadStaticMeshFromBat(const std::string& StaticMeshPath) {

	std::ifstream BatFile(StaticMeshPath, std::ios::in | std::ios::binary);
	int32_t DataLength;

	BatFile.read((char*)&DataLength, sizeof(int32_t));//Mesh���ֳ���
	MeshInfo.MeshName.resize(DataLength);
	BatFile.read((char*)MeshInfo.MeshName.data(), sizeof(char)*DataLength);//MeshNameֵ
	BatFile.read((char*)&DataLength, sizeof(int32_t));//Mesh·������
	MeshInfo.MeshPath.resize(DataLength);
	BatFile.read((char*)MeshInfo.MeshPath.data(), sizeof(char) * DataLength);//Mesh·��ֵ
	BatFile.read((char*)&MeshInfo.MeshVerticesNum, sizeof(int32_t));//Mesh��������
	BatFile.read((char*)&MeshInfo.MeshTrianglesNum, sizeof(int32_t));//Meh����������

	BatFile.read((char*)&DataLength, sizeof(int32_t));//Mesh������Ϣ����
	MeshInfo.MeshVertexInfo.resize(DataLength);
	BatFile.read((char*)MeshInfo.MeshVertexInfo.data(), sizeof(FVector) * DataLength);//Mesh����

	BatFile.read((char*)&DataLength, sizeof(int32_t));//Mesh��������
	MeshInfo.MeshIndexInfo.resize(DataLength);
	BatFile.read((char*)MeshInfo.MeshIndexInfo.data(), sizeof(int32_t)*DataLength);//Mesh����

	BatFile.read((char*)&DataLength, sizeof(int32_t));//MeshNormal����
	MeshInfo.MeshVertexNormalInfo.resize(DataLength);
	BatFile.read((char*)MeshInfo.MeshVertexNormalInfo.data(), sizeof(FVector4) * DataLength);//MeshNormal

	BatFile.read((char*)&DataLength, sizeof(int32_t));//MeshTexCoord����(UV)
	MeshInfo.MeshTexCoord.resize(DataLength);
	BatFile.read((char*)MeshInfo.MeshTexCoord.data(), sizeof(FVector2D) * DataLength);//MeshTexCoord

	BatFile.close();
}

void StaticMesh::SetMeshBuffer(std::shared_ptr<Primitive_MeshBuffer> buffer)
{
	meshBuffer = buffer;
}

std::string StaticMesh::GetMeshName() {
	return MeshInfo.MeshName;
}