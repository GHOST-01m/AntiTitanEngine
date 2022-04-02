#pragma once
#include "Primitive_RenderTarget.h"
#include "DXPrimitive_GPUResource.h"

class DXPrimitive_RenderTarget :
    public Primitive_RenderTarget
{
public:
	~DXPrimitive_RenderTarget();

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

public:
	std::shared_ptr<Primitive_GPUResource> GetGpuResource() override;

public:
	Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];
	std::shared_ptr<Primitive_GPUResource> mGPUResource;

public:
	CD3DX12_CPU_DESCRIPTOR_HANDLE mhCpuSrvHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mhGpuSrvHandle;
	CD3DX12_CPU_DESCRIPTOR_HANDLE mhCpuDsvHandle;
};