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
	std::map<int, std::string> MapofGeosMesh;//ͨ��Mesh�����ҵ�Geos�������Ӧ��MeshGeometry

public:
	void LoadExternalMapActor(std::string Path);

	void LoadAsset(//�ѷ���,Ӧ����Renderer���LoadAsset(),���Ǿɰ��WindowsApp�ﻹ���ã���ɾ��
		Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice,
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList);

	void AddAsset();
	void RemoveAsset();
};