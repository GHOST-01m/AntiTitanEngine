#pragma once
#include "RHIResource_RenderTarget.h"
#include "DXRHIResource_GPUResource.h"

class DXRHIResource_RenderTarget :
    public RHIResource_RenderTarget
{
public:
	~DXRHIResource_RenderTarget();

private:
	int mCurrBackBufferIndex = 0;//��ǰ��һ֡�Ľ�����Index,����������ǰmSwapChainBuffer����ȡֵ
	static const int SwapChainBufferCount = 2;//����������

public:
	void SetCurrBackBufferIndex(int Index);
	int  GetCurrBackBufferIndex();
	int  GetSwapChainBufferCount();
	std::string rtvHeapName="";//���������Լ����RenderTarget���ĸ�rtvHeap��,��ʵ��Ӧ����һ��ƫ�Ƽ�¼
	int mRtvDescriptorSize;
	ID3D12Resource* CurrentBackBuffer()const;
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferCpuHandle()const;//	mRtvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//Microsoft::WRL::ComPtr<ID3D12Resource> GetDepthStencilBuffer();
public:
	std::shared_ptr<RHIResource_GPUResource> GetGpuResource() override;

public:
	Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];
	//Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;
	std::shared_ptr<RHIResource_GPUResource> mDSResource;

public:
	CD3DX12_CPU_DESCRIPTOR_HANDLE mhCpuSrvHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mhGpuSrvHandle;
	CD3DX12_CPU_DESCRIPTOR_HANDLE mhCpuDsvHandle;
};