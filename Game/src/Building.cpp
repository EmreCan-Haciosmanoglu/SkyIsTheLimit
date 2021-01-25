#include "canpch.h"
#include "Building.h"

#include "Types/RoadSegment.h"

namespace Can
{
	Building::Building(Prefab* prefab, RoadSegment* connectedRoadSegment, float snappedT, const glm::vec3& position, const glm::vec3& rotation)
		: connectedRoadSegment(connectedRoadSegment)
		, snappedT(snappedT)
		, object(new Object(prefab, prefab, position, glm::vec3{ 1.0f, 1.0f, 1.0f }, rotation))
		, position(position)
	{
	}
	Building::Building(Object* object, RoadSegment* connectedRoadSegment, float snappedT, const glm::vec3& position, const glm::vec3& rotation)
		: connectedRoadSegment(connectedRoadSegment)
		, snappedT(snappedT)
		, object(object)
		, position(position)
	{
	}
	Building::~Building()
	{
		delete object;
	}
}