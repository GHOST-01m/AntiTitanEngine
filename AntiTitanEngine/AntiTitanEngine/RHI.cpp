#include "stdafx.h"
#include "RHI.h"

std::shared_ptr<RHIResourceManager> RHI::GetRHIResourceManager()
{
	return mRHIResourceManager;
}
