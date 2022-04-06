#include "stdafx.h"
#include "DXPrimitive_Heap.h"

DXPrimitive_Heap::~DXPrimitive_Heap()
{

}

int DXPrimitive_Heap::GetCurrentHeapSize()
{
	return currentHeapSize;
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DXPrimitive_Heap::GetDXHeap()
{
	return mDescriptorHeap;
}
