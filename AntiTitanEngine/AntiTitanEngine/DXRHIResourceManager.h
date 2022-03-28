#pragma once
#include "RHIResourceManager.h"
#include "DXRHIResource_VBIBBuffer.h"
#include "DXRHIDevice.h"
#include "DXRHIFactory.h"
#include "DXRHIResource_Shader.h"
#include "DXRHIResource_Texture.h"
#include "DXRHIResource_ShadowMap.h"
#include "DXRHIResource_Heap.h"

class DXRHIResourceManager :public RHIResourceManager
{
public:
	UINT mRtvDescriptorSize = 0;
	UINT mDsvDescriptorSize = 0;
	UINT mCbvSrvUavDescriptorSize = 0;
};

