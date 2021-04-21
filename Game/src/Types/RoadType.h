#pragma once

#include "Can/Renderer/Prefab.h"

namespace Can
{
	struct RoadType
	{
		Prefab* road;
		Prefab* junction;
		Prefab* end;

		float width;
		float length;
		float junction_length;
	};
}