#pragma once
#include "RHIResource_ShadowMap.h"
class DXRHIResource_ShadowMap :
    public RHIResource_ShadowMap
{
public:
    ~DXRHIResource_ShadowMap();

public:
	UINT mWidth = 4096;//SRVµÄÏñËØ??
	UINT mHeight = 4096;

	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;

public:
	CD3DX12_CPU_DESCRIPTOR_HANDLE mhCpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mhGpuSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE mhCpuDsv;

public:
    Microsoft::WRL::ComPtr<ID3D12Resource> mShadowResource = nullptr;
};