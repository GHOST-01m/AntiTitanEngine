#pragma once
#include "RHIResource_Heap.h"
class DXRHIResource_Heap :
    public RHIResource_Heap
{
public:
    ~DXRHIResource_Heap();
public:
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDescriptorHeap;
};