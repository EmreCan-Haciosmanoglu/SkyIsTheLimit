#pragma once
#include "Can.h"

namespace Can
{
	class Road;

	class End
	{
	public:
		End(Road* connectedRoad, Prefab* prefab);
		End(Road* connectedRoad, Prefab* prefab, const glm::vec3& position);
		End(Road* connectedRoad, Prefab* prefab, const glm::vec3& position, const glm::vec3& scale);
		End(Road* connectedRoad, Prefab* prefab, const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation);
		End(Road* connectedRoad, Object* object);
		~End();

	public:

		glm::vec3 position;

		Can::Object* object;
		Road* connectedRoad;
	};
}