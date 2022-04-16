#include "stdafx.h"
#include "Primitive_RenderTarget.h"

Primitive_RenderTarget::~Primitive_RenderTarget()
{

}

std::shared_ptr<Primitive_GPUResource> Primitive_RenderTarget::GetCurrentSwapChainBuffer() const
{
	return nullptr;
}
