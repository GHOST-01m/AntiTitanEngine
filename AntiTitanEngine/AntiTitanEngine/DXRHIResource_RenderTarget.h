#pragma once
#include "RHIResource_RenderTarget.h"
class DXRHIResource_RenderTarget :
    public RHIResource_RenderTarget
{
public:
	~DXRHIResource_RenderTarget();

private:
	int mCurrBackBufferIndex = 0;//当前这一帧的交换链Index,用于索引当前mSwapChainBuffer数组取值
	static const int SwapChainBufferCount = 2;//交换链数量

public:
	void SetCurrBackBufferIndex(int Index);
	int  GetCurrBackBufferIndex();
	int  GetSwapChainBufferCount();
	Microsoft::WRL::ComPtr<ID3D12Resource> GetDepthStencilBuffer();

public:
	Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];
	Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;
};