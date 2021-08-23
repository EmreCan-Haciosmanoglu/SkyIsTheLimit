#include "canpch.h"
#include "Building.h"

#include "Types/RoadSegment.h"

namespace Can
{
	Building::Building(Prefab* prefab, s64 connectedRoadSegment, float snappedT, const glm::vec3& position, const glm::vec3& rotation)
		: connectedRoadSegment(connectedRoadSegment)
		, snappedT(snappedT)
		, object(new Object(prefab, position, rotation))
		, position(position)
	{
	}
	Building::Building(Object* object, s64 connectedRoadSegment, float snappedT, const glm::vec3& position, const glm::vec3& rotation)
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