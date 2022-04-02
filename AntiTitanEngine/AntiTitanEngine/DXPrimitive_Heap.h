#pragma once
#include "Primitive_Heap.h"
class DXPrimitive_Heap :
    public Primitive_Heap
{
public:
    ~DXPrimitive_Heap();
    std::string name;
public:
    int GetCurrentHeapSize()override;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetDXHeap();
public:
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDescriptorHeap;
};