#pragma once
#include "Actor.h"
class Scene
{
public:
	std::map<std::string, std::shared_ptr<Actor>> actorLib;
};

