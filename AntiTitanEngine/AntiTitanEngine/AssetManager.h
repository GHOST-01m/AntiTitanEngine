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
	std::map<int, std::string> MapofGeosMesh;//通过Mesh名字找到Geos数组里对应的MeshGeometry

public:
	void LoadExternalMapActor(std::string Path);

	void LoadAsset(//已废弃,应该用Renderer里的LoadAsset(),但是旧版的WindowsApp里还在用，不删了
		Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice,
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList);

	void AddAsset();
	void RemoveAsset();
};