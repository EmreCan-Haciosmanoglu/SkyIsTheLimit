#include "canpch.h"
#include "End.h"

#include "Types/RoadSegment.h"

namespace Can
{
	End::End(RoadSegment* connectedRoadSegment, Prefab* prefab, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale)
		: position(position)
		, connectedRoadSegment(connectedRoadSegment)
		, object(new Object(prefab, position, rotation, scale))
	{
	}
	End::End(RoadSegment* connectedRoad, Object* object)
		: position(object->position)
		, connectedRoadSegment(connectedRoadSegment)
		, object(object)
	{
	}
	End::~End()
	{
		delete object;
	}
}