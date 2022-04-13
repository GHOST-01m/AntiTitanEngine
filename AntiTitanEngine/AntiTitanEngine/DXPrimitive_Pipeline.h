#pragma once
#include "Primitive_Pipeline.h"
class DXPrimitive_Pipeline :
    public Primitive_Pipeline
{
public:
    ~DXPrimitive_Pipeline();

public:
    ComPtr<ID3D12PipelineState> GetDXPipeline();
    ComPtr<ID3D12PipelineState> mPipeline;
};