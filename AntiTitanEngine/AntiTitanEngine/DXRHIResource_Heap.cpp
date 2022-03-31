#include "stdafx.h"
#include "DXRHIResource_Heap.h"

DXRHIResource_Heap::~DXRHIResource_Heap()
{

}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DXRHIResource_Heap::GetDXHeap()
{
	return mDescriptorHeap;
}
