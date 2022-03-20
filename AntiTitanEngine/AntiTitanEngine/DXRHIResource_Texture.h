#pragma once
#include "RHIResource_Texture.h"
class DXRHIResource_Texture :
    public RHIResource_Texture
{
public:
	~DXRHIResource_Texture();

public:
	// Unique material name for lookup.
	std::string Name;
	std::wstring Filename;

	Microsoft::WRL::ComPtr<ID3D12Resource> Resource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap = nullptr;

};

