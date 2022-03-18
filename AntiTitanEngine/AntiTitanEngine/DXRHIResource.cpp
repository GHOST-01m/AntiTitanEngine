#include "stdafx.h"
#include "DXRHIResource.h"

std::shared_ptr<Device> DXRHIResource::GetDevice()
{
	return std::make_shared<DXDevice>();
}
