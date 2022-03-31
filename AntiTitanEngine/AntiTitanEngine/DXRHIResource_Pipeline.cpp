#include "stdafx.h"
#include "DXRHIResource_Pipeline.h"

DXRHIResource_Pipeline::~DXRHIResource_Pipeline()
{

}

ComPtr<ID3D12PipelineState> DXRHIResource_Pipeline::GetPipeline()
{
	return mPipeline;
}
