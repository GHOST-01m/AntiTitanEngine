#include "stdafx.h"

std::vector<std::unique_ptr<MeshGeometry>>* AssetManager::GetGeometryLibrary()
{
	return &Geos;
}

ActorsInfo* AssetManager::GetMapActorInfo()
{
	return &mMapActor;
}

std::map<int, std::string>* AssetManager::GetMapofGeosMesh()
{
	return &MapofGeosMesh;
}

int AssetManager::GetGeoKeyByName(std::string name)
{
	for (auto it = MapofGeosMesh.begin(); it != MapofGeosMesh.end(); it++)
	{
		//MapofGeosMeshͨ��ӳ���ҵ�MapActor�����ֶ�Ӧ��Geos�е�key
		if (it->second == name){
			return it->first;
		}
	}

	return 0;//���ִ�е�������,Ӧ���׳�һ���쳣��ʾû�д�Map���ҵ���Ӧ���ֵļ�����
}

void AssetManager::LoadExternalMapActor(std::string Path) {
	mMapActor.SetSceneActorsInfoFromBat(Path);
}

void AssetManager::LoadAsset(
	Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice,
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList)
{
	std::string StaticMeshPath;
	std::set<std::string> StaticMeshs;//����ȥ��

	StaticMesh mesh;

	Geos.resize(mMapActor.Size());

	Vertex vertice;

	UINT vbByteSize;
	UINT ibByteSize;

	//ѭ������MeshGeometry
	for (int i = 0; i < Geos.size(); i++)
	{
		std::vector<Vertex> vertices;
		std::vector<int32_t> indices;
		//��ͬ��StaticMesh�����ظ�����
		auto check = StaticMeshs.find(mMapActor.MeshNameArray[i]);
		if (check == StaticMeshs.end()) {
			StaticMeshs.insert(mMapActor.MeshNameArray[i]);
			MapofGeosMesh.insert(std::pair<int, std::string>(i, mMapActor.MeshNameArray[i]));
		}
		else { continue; }

		//��ȡmesh��Ϣ
		StaticMeshPath = "SplitMesh/" + mMapActor.MeshNameArray[i];
		//StaticMeshPath = "SplitMesh/SM_MatPreviewMesh_02";
		StaticMeshPath.erase(StaticMeshPath.length() - 1);
		StaticMeshPath += ".bat";
		mesh.LoadStaticMeshFromBat(StaticMeshPath);

		if (mesh.MeshInfo.MeshVertexInfo.size() < 3) { continue; }//û��StaticMesh�Ͳ���ȡ
		//--------------------------------------------------------------------------------

		for (int j = 0; j < mesh.MeshInfo.MeshVertexInfo.size(); j++)
		{
			vertice.Pos = mesh.MeshInfo.MeshVertexInfo[j];

			vertice.Color = {
				float(j) / mesh.MeshInfo.MeshVertexInfo.size(),
				float(j) / mesh.MeshInfo.MeshVertexInfo.size(),
				float(j) / mesh.MeshInfo.MeshVertexInfo.size(),
				1 };//��ʼ����ֵΪ�ڰ�ɫ

			vertice.Normal = mesh.MeshInfo.MeshVertexNormalInfo[j];

			vertices.push_back(vertice);
		}

		indices = mesh.MeshInfo.MeshIndexInfo;

		vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
		ibByteSize = (UINT)indices.size() * sizeof(std::uint32_t);

		Geos[i] = std::make_unique<MeshGeometry>();
		Geos[i]->Name = mesh.getMeshName();

		ThrowIfFailed(D3DCreateBlob(vbByteSize, &Geos[i]->VertexBufferCPU));
		CopyMemory(Geos[i]->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

		ThrowIfFailed(D3DCreateBlob(ibByteSize, &Geos[i]->IndexBufferCPU));
		CopyMemory(Geos[i]->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

		Geos[i]->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
			mCommandList.Get(), vertices.data(), vbByteSize, Geos[i]->VertexBufferUploader);

		Geos[i]->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
			mCommandList.Get(), indices.data(), ibByteSize, Geos[i]->IndexBufferUploader);

		Geos[i]->VertexByteStride = sizeof(Vertex);
		Geos[i]->VertexBufferByteSize = vbByteSize;
		Geos[i]->IndexFormat = DXGI_FORMAT_R32_UINT;
		Geos[i]->IndexBufferByteSize = ibByteSize;

		SubmeshGeometry submesh;
		submesh.IndexCount = (UINT)indices.size();
		submesh.StartIndexLocation = 0;
		submesh.BaseVertexLocation = 0;

		//Geos[i]->DrawArgs["box"] = submesh;
		Geos[i]->DrawArgs[mMapActor.MeshNameArray[i]] = submesh;
		//--------------------------------------------------------------------------------
	}
}