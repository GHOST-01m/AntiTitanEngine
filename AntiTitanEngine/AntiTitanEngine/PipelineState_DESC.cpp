#include "stdafx.h"
#include "PipelineState_DESC.h"

void PipelineState_DESC::SetDefauleRasterizerDESC()
{
	rasterizeDesc.FillMode = FILL_MODE_SOLID;
	rasterizeDesc.CullMode = CULL_MODE_BACK;
	rasterizeDesc.FrontCounterClockwise = false;
	rasterizeDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	rasterizeDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	rasterizeDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	rasterizeDesc.DepthClipEnable = true;
	rasterizeDesc.MultisampleEnable = false;
	rasterizeDesc.AntialiasedLineEnable = false;
	rasterizeDesc.ForcedSampleCount = 0;
	rasterizeDesc.ConservativeRasterizationMode = 0;

}
