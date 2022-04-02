#pragma once
#include "Primitive_Texture.h"
class DXPrimitive_Texture :
    public Primitive_Texture
{
public:
	~DXPrimitive_Texture();

public:
	// Unique material name for lookup.
	std::string Name;
	std::wstring Filename;

	Microsoft::WRL::ComPtr<ID3D12Resource> Resource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap = nullptr;
};

