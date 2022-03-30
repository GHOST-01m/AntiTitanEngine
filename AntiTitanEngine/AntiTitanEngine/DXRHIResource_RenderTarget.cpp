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

Microsoft::WRL::ComPtr<ID3D12Resource> DXRHIResource_RenderTarget::GetDepthStencilBuffer()
{
	return mDepthStencilBuffer;
}
