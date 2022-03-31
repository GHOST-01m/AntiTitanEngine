#pragma once
#include "RHIResource_RenderTarget.h"
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
	Microsoft::WRL::ComPtr<ID3D12Resource> GetDepthStencilBuffer();

public:
	Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];
	Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;
};