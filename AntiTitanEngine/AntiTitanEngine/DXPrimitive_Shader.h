#pragma once
#include "Primitive_Shader.h"
class DXPrimitive_Shader :
    public Primitive_Shader
{
public:
	~DXPrimitive_Shader();
public:
	ComPtr<ID3DBlob> mvsByteCode = nullptr;
	ComPtr<ID3DBlob> mpsByteCode = nullptr;

	ComPtr<ID3DBlob> mvsShadowMapCode = nullptr;
	ComPtr<ID3DBlob> mpsShadowMapCode = nullptr;
	ComPtr<ID3DBlob> mpsShadowMapCode_AlphaTested = nullptr;


	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

};

