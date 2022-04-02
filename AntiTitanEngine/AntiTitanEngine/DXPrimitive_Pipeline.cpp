#include "stdafx.h"
#include "DXPrimitive_Pipeline.h"

DXPrimitive_Pipeline::~DXPrimitive_Pipeline()
{

}

ComPtr<ID3D12PipelineState> DXPrimitive_Pipeline::GetPipeline()
{
	return mPipeline;
}
