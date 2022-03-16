#pragma once
#include "ActorsInfo.h"
#include "map"
#include "vector"

class AssetManager
{
public:
	std::vector<std::unique_ptr<MeshGeometry>>* GetGeometryLibrary();
	ActorsInfo* GetMapActorInfo();
	std::map<int, std::string>* GetMapofGeosMesh();
	int GetGeoKeyByName(std::string name);

	std::vector<std::unique_ptr<MeshGeometry>> Geos;

public:
	ActorsInfo mMapActor;
	std::map<int, std::string> MapofGeosMesh;//通过Mesh名字找到Geos数组里对应的MeshGeometry

public:
	void LoadExternalMapActor(std::string Path);

	void LoadAsset(//已废弃,应该用Renderer里的LoadAsset(),但是旧版的WindowsApp里还在用，不删了
		Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice,
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList);

	void AddAsset();
	void RemoveAsset();
};