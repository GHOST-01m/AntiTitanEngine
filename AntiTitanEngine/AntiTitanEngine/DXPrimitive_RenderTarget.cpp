#include "stdafx.h"
#include "DXPrimitive_RenderTarget.h"

DXPrimitive_RenderTarget::~DXPrimitive_RenderTarget()
{
}

void DXPrimitive_RenderTarget::SetCurrBackBufferIndex(int Index)
{
	mCurrBackBufferIndex = Index;
}

int DXPrimitive_RenderTarget::GetCurrBackBufferIndex()
{
	return mCurrBackBufferIndex;
}

int DXPrimitive_RenderTarget::GetSwapChainBufferCount()
{
	return SwapChainBufferCount;
}


ID3D12Resource* DXPrimitive_RenderTarget::CurrentBackBuffer() const
{
	return mSwapChainBuffer[mCurrBackBufferIndex].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE DXPrimitive_RenderTarget::CurrentBackBufferCpuHandle() const
{
		auto rtvHeap = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager()->GetHeapByName(rtvHeapName);

		return CD3DX12_CPU_DESCRIPTOR_HANDLE(
			std::dynamic_pointer_cast<DXPrimitive_Heap>(rtvHeap)->mDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			mCurrBackBufferIndex,
			mRtvDescriptorSize);
}

std::shared_ptr<Primitive_GPUResource> DXPrimitive_RenderTarget::GetGpuResource()
{
	return mGPUResource;
}
