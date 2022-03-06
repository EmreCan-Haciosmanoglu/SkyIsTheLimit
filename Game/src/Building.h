#pragma once
#include "Can.h"

namespace Can
{
	class RoadSegment;
	class Building
	{
	public:
		Building(Prefab* prefab, s64 connectedRoadSegment, float snappedT, const glm::vec3& position, const glm::vec3& rotation);
		Building(Object* object, s64 connectedRoadSegment, float snappedT, const glm::vec3& position, const glm::vec3& rotation);
		~Building();

	public:

		u64 type = 0;
		s64 connectedRoadSegment;
		float snappedT = 0.0f;

		Object* object;
		glm::vec3 position;
	};
}