#pragma once
#include "Primitive_RenderTarget.h"
#include "DXPrimitive_GPUResource.h"

class DXPrimitive_RenderTarget :
    public Primitive_RenderTarget
{
public:
	~DXPrimitive_RenderTarget();

private:
	int mCurrBackBufferIndex = 0;//当前这一帧的交换链Index,用于索引当前mSwapChainBuffer数组取值
	static const int SwapChainBufferCount = 2;//交换链数量

public:
	void SetCurrBackBufferIndex(int Index);
	int  GetCurrBackBufferIndex();
	int  GetSwapChainBufferCount();
	std::string rtvHeapName="";
	int mRtvDescriptorSize;
	//ID3D12Resource* CurrentBackBuffer()const;
	std::shared_ptr<Primitive_GPUResource> CurrentSwapChainBuffer()const;
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferCpuHandle()const;//	mRtvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

public:
	std::shared_ptr<Primitive_GPUResource> GetDSVResource() override;

public:
	//Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];
	std::vector<std::shared_ptr<Primitive_GPUResource>> mSwapChainResource;
	std::shared_ptr<Primitive_GPUResource> mDSVResource; 

public:
	CD3DX12_CPU_DESCRIPTOR_HANDLE mhCpuSrvHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mhGpuSrvHandle;
	CD3DX12_CPU_DESCRIPTOR_HANDLE mhCpuDsvHandle;
};