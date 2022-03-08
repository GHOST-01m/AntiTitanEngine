#pragma once
#include "stdafx.h"

struct ObjectConstants
{
	XMFLOAT4X4 WorldViewProj = MathHelper::Identity4x4();
	glm::mat4 WorldViewProjMat4 = glm::identity<glm::mat4>();
};