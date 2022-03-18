#pragma once
#include "RHIResource.h"
class DXRHIResource :public RHIResource
{
public:
    std::shared_ptr<Device> GetDevice()override;

public:
    Microsoft::WRL::ComPtr<ID3D12Device> mDXDevice;
};



class DXDevice : public Device
{
public:
	Microsoft::WRL::ComPtr<ID3D12Device> mDXDevice;
};