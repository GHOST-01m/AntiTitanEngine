#pragma once
#include "RHIResource_Shader.h"
class DXRHIResource_Shader :
    public RHIResource_Shader
{
public:
	~DXRHIResource_Shader();
public:
	ComPtr<ID3DBlob> mvsByteCode = nullptr;
	ComPtr<ID3DBlob> mpsByteCode = nullptr;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

};

