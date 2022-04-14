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


//ID3D12Resource* DXPrimitive_RenderTarget::CurrentBackBuffer() const
//{
//	return mSwapChainBuffer[mCurrBackBufferIndex].Get();
//}

std::shared_ptr<Primitive_GPUResource> DXPrimitive_RenderTarget::GetCurrentSwapChainBuffer() const
{
	return mSwapChainResource[mCurrBackBufferIndex];
}

D3D12_CPU_DESCRIPTOR_HANDLE DXPrimitive_RenderTarget::GetCurrentBackBufferCpuHandle() const
{
	//auto rtvHeap = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager()->GetHeapByName(rtvHeapName);

	auto resource = std::dynamic_pointer_cast<DXPrimitive_GPUResource>(GetCurrentSwapChainBuffer());
	auto rtvSize = resource->rtvSize;
	auto dxResource = std::dynamic_pointer_cast<DXPrimitive_GPUResource>(resource);
	auto rtvHeap = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager()->GetHeapByName(rtvHeapName);
	auto mRtvHeap = std::dynamic_pointer_cast<DXPrimitive_Heap>(rtvHeap);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvHeap->mDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	rtvHeapHandle.Offset(dxResource->rtvHeapOffsetLocation, rtvSize);
	
	return rtvHeapHandle;
	//return CD3DX12_CPU_DESCRIPTOR_HANDLE(
	//	std::dynamic_pointer_cast<DXPrimitive_Heap>(rtvHeap)->mDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
	//	mCurrBackBufferIndex,
	//	mRtvDescriptorSize);
}

std::shared_ptr<Primitive_GPUResource> DXPrimitive_RenderTarget::GetDSVResource()
{
	return mDSVResource;
}
//
//std::shared_ptr<Primitive_GPUResource> DXPrimitive_RenderTarget::GetSRVResource()
//{
//	return mSRVResource;
//}
