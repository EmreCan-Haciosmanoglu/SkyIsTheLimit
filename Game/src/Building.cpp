#include "canpch.h"
#include "Building.h"

#include "Road.h"

namespace Can
{
	Building::Building(Prefab* prefab, Road* connectedRoad, const glm::vec3& position, const glm::vec3& rotation)
		: connectedRoad(connectedRoad)
		, object(new Object(prefab, prefab, position, glm::vec3{ 1.0f, 1.0f, 1.0f }, rotation))
		, position(position)
	{
	}
	Building::Building(Object* object, Road* connectedRoad, const glm::vec3& position, const glm::vec3& rotation)
		: connectedRoad(connectedRoad)
		, object(object)
		, position(position)
	{
	}
	Building::~Building()
	{
		delete object;
	}
}