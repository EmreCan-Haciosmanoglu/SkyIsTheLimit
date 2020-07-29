#pragma once
#include "Can.h"

namespace Can
{
	class Road;

	class End
	{
	public:
		End();
		~End();

	public:
		glm::vec3 position;
		glm::vec2 rotation;
		Can::Object* object;
		Road* connectedRoad;
	};
}