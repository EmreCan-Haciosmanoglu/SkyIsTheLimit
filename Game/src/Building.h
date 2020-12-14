#pragma once
#include "Can.h"

namespace Can
{
	class RoadSegment;
	class Building
	{
	public:
		Building(Prefab* prefab, RoadSegment* connectedRoadSegment, const glm::vec3& position, const glm::vec3& rotation);
		Building(Object* object, RoadSegment* connectedRoadSegment, const glm::vec3& position, const glm::vec3& rotation);
		~Building();

	public:

		RoadSegment* connectedRoadSegment;

		Object* object;
		glm::vec3 position;
	};
}