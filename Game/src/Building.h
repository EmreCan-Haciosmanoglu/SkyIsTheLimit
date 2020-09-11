#pragma once
#include "Can.h"
#include "Road.h"

namespace Can
{
	class Building
	{
	public:
		Building(Prefab* prefab, Road* connectedRoad, const glm::vec3& position, const glm::vec3& rotation);
		Building(Object* object, Road* connectedRoad, const glm::vec3& position, const glm::vec3& rotation);
		~Building();

	public:

		Road* connectedRoad;

		Object* object;
		glm::vec3 position;
	};
}