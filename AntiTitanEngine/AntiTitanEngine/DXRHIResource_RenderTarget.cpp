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


ID3D12Resource* DXRHIResource_RenderTarget::CurrentBackBuffer() const
{
	return mSwapChainBuffer[mCurrBackBufferIndex].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE DXRHIResource_RenderTarget::CurrentBackBufferCpuHandle() const
{
		auto rtvHeap = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager()->GetHeapByName(rtvHeapName);

		return CD3DX12_CPU_DESCRIPTOR_HANDLE(
			std::dynamic_pointer_cast<DXRHIResource_Heap>(rtvHeap)->mDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			mCurrBackBufferIndex,
			mRtvDescriptorSize);
}

std::shared_ptr<RHIResource_GPUResource> DXRHIResource_RenderTarget::GetGpuResource()
{
	return mGPUResource;
}
