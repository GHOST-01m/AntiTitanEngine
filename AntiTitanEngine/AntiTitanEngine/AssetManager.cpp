#include "stdafx.h"

std::shared_ptr<StaticMesh> AssetManager::GetStaticMeshByName(std::string name)
{
	return StaticMeshLib.at(name);
}

void AssetManager::InsertStaticMeshToLib(std::string name, std::shared_ptr<StaticMesh> staticMesh)
{
	StaticMeshLib.insert(std::pair<std::string, std::shared_ptr<StaticMesh>>(name, staticMesh));
}

std::shared_ptr<Scene> AssetManager::GetScene()
{
	return mScene;
}

std::vector<std::unique_ptr<MeshGeometry>>* AssetManager::GetGeometryLibrary()
{
	return &Geos;
}

MapActorsInfo* AssetManager::GetMapActorInfo()
{
	return &mMapActor;
}
//
std::map<int, std::string>* AssetManager::GetMapofGeosMesh()
{
	return &MapofGeosMesh;
}

int AssetManager::GetGeoKeyByName(std::string name)
{
	for (auto it = MapofGeosMesh.begin(); it != MapofGeosMesh.end(); it++)
	{
		//MapofGeosMesh通过映射找到MapActor的名字对应的Geos中的key
		if (it->second == name){
			return it->first;
		}
	}

	return 0;//如果执行到这里了,应该抛出一个异常表示没有从Map中找到对应名字的几何体
}

std::shared_ptr<FLight> AssetManager::GetLight()
{
	return mLight;
}

void AssetManager::LoadExternalMapActor(std::string Path) {
	mMapActor.LoadSceneActorsInfoFromBat(Path);
	mScene = std::make_shared<Scene>();
	for (auto index=0;index< mMapActor.Size();index++)
	{
		std::shared_ptr<Actor> actor=std::make_shared<Actor>();
		actor->actorName = mMapActor.ActorNameArray[index];
		actor->quat = mMapActor.ActorsQuatArray[index];
		actor->staticmeshName = mMapActor.MeshNameArray[index];
		actor->transform = mMapActor.ActorsTransformArray[index];
		actor->CBVoffset = index;
		mScene->actorLib.insert(std::pair<std::string, std::shared_ptr<Actor>>(actor->actorName, actor));
	}
}

void AssetManager::LoadAsset(
	Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice,
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList)
{
	std::string StaticMeshPath;
	std::set<std::string> StaticMeshs;//用于去重

	StaticMesh mesh;

	Geos.resize(mMapActor.Size());

	Vertex vertice;

	UINT vbByteSize;
	UINT ibByteSize;

	//循环加入MeshGeometry
	for (int i = 0; i < Geos.size(); i++)
	{
		std::vector<Vertex> vertices;
		std::vector<int32_t> indices;
		//相同的StaticMesh不用重复建立
		auto check = StaticMeshs.find(mMapActor.MeshNameArray[i]);
		if (check == StaticMeshs.end()) {
			StaticMeshs.insert(mMapActor.MeshNameArray[i]);
			MapofGeosMesh.insert(std::pair<int, std::string>(i, mMapActor.MeshNameArray[i]));
		}
		else { continue; }

		//读取mesh信息
		StaticMeshPath = "SplitMesh/" + mMapActor.MeshNameArray[i];
		//StaticMeshPath = "SplitMesh/SM_MatPreviewMesh_02";
		StaticMeshPath.erase(StaticMeshPath.length() - 1);
		StaticMeshPath += ".bat";
		mesh.LoadStaticMeshFromBat(StaticMeshPath);

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
		Geos[i]->Name = mesh.GetMeshName();

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