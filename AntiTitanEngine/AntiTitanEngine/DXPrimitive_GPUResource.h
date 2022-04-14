#pragma once
#include "Primitive_GPUResource.h"
class DXPrimitive_GPUResource :public Primitive_GPUResource
{
public:
	~DXPrimitive_GPUResource();

public:
	std::string mResourceName;

public:
	int srvHeapOffsetLocation = -1;
	int rtvHeapOffsetLocation = -1;
	int dsvHeapOffsetLocation = -1;

	int srvSize = -1;
	int rtvSize = -1;
	int dsvSize = -1;
public:
	D3D12_RESOURCE_STATES currentType;
	Microsoft::WRL::ComPtr<ID3D12Resource> mResource = nullptr;
};