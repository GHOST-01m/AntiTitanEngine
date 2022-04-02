#pragma once
#include "Primitive_GPUResource.h"
class DXPrimitive_GPUResource :public Primitive_GPUResource
{
public:
	~DXPrimitive_GPUResource();

public:
	std::string mResourceName;

public:
	D3D12_RESOURCE_STATES currentType;
	Microsoft::WRL::ComPtr<ID3D12Resource> mResource = nullptr;
};