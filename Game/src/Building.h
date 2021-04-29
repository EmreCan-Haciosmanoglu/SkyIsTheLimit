#pragma once
#include "Can.h"

namespace Can
{
	class RoadSegment;
	class Person;

	class Building
	{
	public:
		Building(Prefab* prefab, RoadSegment* connectedRoadSegment, float snappedT, const glm::vec3& position, const glm::vec3& rotation);
		Building(Object* object, RoadSegment* connectedRoadSegment, float snappedT, const glm::vec3& position, const glm::vec3& rotation);
		~Building();

	public:

		RoadSegment* connectedRoadSegment;
		std::vector<Person*> residents;
		float snappedT = 0.0f;
		u8 capacity = 0;

		Object* object;
		glm::vec3 position;
	};
}