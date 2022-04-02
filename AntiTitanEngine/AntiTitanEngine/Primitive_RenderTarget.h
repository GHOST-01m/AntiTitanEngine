#pragma once
#include "Primitive_GPUResource.h"
class Primitive_RenderTarget
{
public:
	virtual ~Primitive_RenderTarget();

	virtual std::shared_ptr<Primitive_GPUResource> GetGpuResource()=0;
public:
	std::string name;
	float width;
	float height;
};

