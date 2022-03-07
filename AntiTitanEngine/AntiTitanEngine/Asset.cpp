#include "stdafx.h"

void Asset::LoadExternalMapActor(std::string Path) {
	MapActor.SetSceneActorsInfoFromBat(Path);
}

void Asset::LoadAsset(
	Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice,
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList)
{
	std::string StaticMeshPath;
	std::set<std::string> StaticMeshs;//用于去重

	StaticMesh mesh;

	Geos.resize(MapActor.Size());

	Vertex vertice;

	UINT vbByteSize;
	UINT ibByteSize;

	//循环加入MeshGeometry
	for (int i = 0; i < Geos.size(); i++)
	{
		std::vector<Vertex> vertices;
		std::vector<int32_t> indices;
		//相同的StaticMesh不用重复建立
		auto check = StaticMeshs.find(MapActor.MeshNameArray[i]);
		if (check == StaticMeshs.end()) {
			StaticMeshs.insert(MapActor.MeshNameArray[i]);
			MapofGeosMesh.insert(std::pair<int, std::string>(i, MapActor.MeshNameArray[i]));
		}
		else { continue; }

		//读取mesh信息
		StaticMeshPath = "SplitMesh/" + MapActor.MeshNameArray[i];
		//StaticMeshPath = "SplitMesh/SM_MatPreviewMesh_02";
		StaticMeshPath.erase(StaticMeshPath.length() - 1);
		StaticMeshPath += ".bat";
		mesh.SetStaticMeshFromBat(StaticMeshPath);

		if (mesh.MeshInfo.MeshVertexInfo.size() < 3) { continue; }//没有StaticMesh就不读取
		//--------------------------------------------------------------------------------

		for (int j = 0; j < mesh.MeshInfo.MeshVertexInfo.size(); j++)
		{
			vertice.Pos = mesh.MeshInfo.MeshVertexInfo[j];

			vertice.Color = {
				float(j) / mesh.MeshInfo.MeshVertexInfo.size(),
				float(j) / mesh.MeshInfo.MeshVertexInfo.size(),
				float(j) / mesh.MeshInfo.MeshVertexInfo.size(),
				1 };//初始化赋值为黑白色

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
		Geos[i]->DrawArgs[MapActor.MeshNameArray[i]] = submesh;
		//--------------------------------------------------------------------------------
	}
}