#pragma once
#include "RHIResource_Pipeline.h"
class DXRHIResource_Pipeline :
    public RHIResource_Pipeline
{
public:
    ~DXRHIResource_Pipeline();



public:
    ComPtr<ID3D12PipelineState> GetPipeline();
    ComPtr<ID3D12PipelineState> mPipeline;
};

