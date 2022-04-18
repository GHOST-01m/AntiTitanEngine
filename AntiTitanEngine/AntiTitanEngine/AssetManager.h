#pragma once
#include "MapActorsInfo.h"
#include "map"
#include "vector"
#include "FLight.h"
#include "Scene.h"
class AssetManager
{
public:
	std::map<std::string,std::shared_ptr<StaticMesh>> StaticMeshLib;
	std::shared_ptr<StaticMesh>   GetStaticMeshByName(std::string name);
	void InsertStaticMeshToLib(std::string name, std::shared_ptr<StaticMesh> staticMesh);
	std::shared_ptr<Scene> GetScene();

public:
	std::vector<std::unique_ptr<MeshGeometry>>* GetGeometryLibrary();
	MapActorsInfo* GetMapActorInfo();
	std::map<int, std::string>* GetMapofGeosMesh();
	int GetGeoKeyByName(std::string name);

	std::vector<std::unique_ptr<MeshGeometry>> Geos;

public:
	MapActorsInfo mMapActor;
	std::shared_ptr<Scene> mScene;
	std::shared_ptr<FLight> mLight;
	std::shared_ptr<FLight> GetLight();
	std::map<int, std::string> MapofGeosMesh;//ͨ��Mesh�����ҵ�Geos�������Ӧ��MeshGeometry

public:
	void LoadExternalMapActor(std::string Path);

	void LoadAsset(//�ѷ���,Ӧ����Renderer���LoadAsset(),���Ǿɰ��WindowsApp�ﻹ���ã���ɾ��
		Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice,
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList);

	void AddAsset();
	void RemoveAsset();
};