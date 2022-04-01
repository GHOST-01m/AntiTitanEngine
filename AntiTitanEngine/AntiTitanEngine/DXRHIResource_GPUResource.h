#pragma once
#include "RHIResource_GPUResource.h"
class DXRHIResource_GPUResource :public RHIResource_GPUResource
{
public:
	~DXRHIResource_GPUResource();

public:
	std::string mResourceName;

public:
	D3D12_RESOURCE_STATES currentType;
	Microsoft::WRL::ComPtr<ID3D12Resource> mResource = nullptr;
};