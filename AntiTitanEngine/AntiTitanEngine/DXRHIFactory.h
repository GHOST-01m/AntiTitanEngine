#pragma once
#include "MyStruct.h"

class DXRHIFactory :public RHIFactory {
public:
	Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;
};