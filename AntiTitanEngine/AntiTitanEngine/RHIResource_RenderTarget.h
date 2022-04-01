#pragma once
#include "RHIResource_GPUResource.h"
class RHIResource_RenderTarget
{
public:
	virtual ~RHIResource_RenderTarget();

	virtual std::shared_ptr<RHIResource_GPUResource> GetGpuResource()=0;
public:
	std::string name;
};

