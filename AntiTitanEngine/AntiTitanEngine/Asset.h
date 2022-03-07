#pragma once
class Asset
{
public:
	ActorsInfo MapActor;
	std::vector<std::unique_ptr<MeshGeometry>> Geos;
	std::map<int, std::string> MapofGeosMesh;//ͨ��Mesh�����ҵ�Geos�������Ӧ��MeshGeometry

public:
	void LoadExternalMapActor(std::string Path);

	void LoadAsset(
		Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice,
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList);

	void AddAsset();
	void RemoveAsset();
};

