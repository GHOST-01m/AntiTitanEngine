#include "stdafx.h"
#include "DXRHIResource_RenderTarget.h"

DXRHIResource_RenderTarget::~DXRHIResource_RenderTarget()
{

}

void DXRHIResource_RenderTarget::SetCurrBackBufferIndex(int Index)
{
	mCurrBackBufferIndex = Index;
}

int DXRHIResource_RenderTarget::GetCurrBackBufferIndex()
{
	return mCurrBackBufferIndex;
}

int DXRHIResource_RenderTarget::GetSwapChainBufferCount()
{
	return SwapChainBufferCount;
}

//void DXRHIResource_RenderTarget::SetSwapChainBufferCount(int num)
//{
//	SwapChainBufferCount = num;
//}

ID3D12Resource* DXRHIResource_RenderTarget::CurrentBackBuffer() const
{
	return mSwapChainBuffer[mCurrBackBufferIndex].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE DXRHIResource_RenderTarget::CurrentBackBufferCpuHandle(std::shared_ptr<RHIResource_Heap>rtvHeap,int mRtvDescriptorSize) const
{

	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		std::dynamic_pointer_cast<DXRHIResource_Heap>(rtvHeap)->mDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		mCurrBackBufferIndex,
		mRtvDescriptorSize);
}

Microsoft::WRL::ComPtr<ID3D12Resource> DXRHIResource_RenderTarget::GetDepthStencilBuffer()
{
	return mDepthStencilBuffer;
}
