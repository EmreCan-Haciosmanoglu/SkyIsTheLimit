#include "canpch.h"
#include "End.h"

#include "Road.h"

namespace Can
{
	End::End(Road* connectedRoad, Prefab* prefab)
		: position({ 0.0f, 0.0f, 0.0f })
		, connectedRoad(connectedRoad)
		, object(new Object(prefab, prefab))
	{
	}
	End::End(Road* connectedRoad, Prefab* prefab, const glm::vec3& position)
		: position(position)
		, connectedRoad(connectedRoad)
		, object(new Object(prefab, prefab, position))
	{
	}
	End::End(Road* connectedRoad, Prefab* prefab, const glm::vec3& position, const glm::vec3& scale)
		: position(position)
		, connectedRoad(connectedRoad)
		, object(new Object(prefab, prefab, position, scale))
	{
	}
	End::End(Road* connectedRoad, Prefab* prefab, const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation)
		: position(position)
		, connectedRoad(connectedRoad)
		, object(new Object(prefab, prefab, position, scale, rotation))
	{
	}
	End::End(Road* connectedRoad, Object* object)
		: position(object->position)
		, connectedRoad(connectedRoad)
		, object(object)
	{
	}
	End::~End()
	{
		delete object;
	}
}