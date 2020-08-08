#include "canpch.h"
#include "End.h"

#include "Road.h"

namespace Can
{
	End::End(Road* connectedRoad, const Ref<Prefab>& prefab)
		: connectedRoad(connectedRoad)
		, object(new Object(prefab, prefab))
	{
	}
	End::End(Road* connectedRoad, const Ref<Prefab>& prefab, const glm::vec3& position)
		: connectedRoad(connectedRoad)
		, object(new Object(prefab, prefab, position))
	{
	}
	End::End(Road* connectedRoad, const Ref<Prefab>& prefab, const glm::vec3& position, const glm::vec3& scale)
		: connectedRoad(connectedRoad)
		, object(new Object(prefab, prefab, position, scale))
	{
	}
	End::End(Road* connectedRoad, const Ref<Prefab>& prefab, const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation)
		: connectedRoad(connectedRoad)
		, object(new Object(prefab, prefab, position, scale, rotation))
	{
	}
	End::End(Road* connectedRoad, Object* object)
		: connectedRoad(connectedRoad)
		, object(object)
	{
	}
	End::~End()
	{
		delete object;
	}
}