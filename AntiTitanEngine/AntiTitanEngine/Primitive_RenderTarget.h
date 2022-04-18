#pragma once
#include "Primitive_GPUResource.h"
class Primitive_RenderTarget
{
public:
	virtual ~Primitive_RenderTarget();

	virtual std::shared_ptr<Primitive_GPUResource> GetDSVResource() = 0;


public:
	virtual std::shared_ptr<Primitive_GPUResource> GetCurrentSwapChainBuffer() const;

	std::vector<std::shared_ptr<Primitive_GPUResource>> mSwapChainResource;
	std::shared_ptr<Primitive_GPUResource> mDSVResource;

public:
	std::string name;
	std::string defaultPipeline;

	float width;
	float height;
};