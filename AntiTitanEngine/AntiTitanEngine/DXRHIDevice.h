#pragma once
#include "MyStruct.h"

class DXRHIDevice :public RHIDevice {
public:
	~DXRHIDevice();

public:
	Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;
};