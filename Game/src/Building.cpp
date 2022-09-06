#include "canpch.h"
#include "Building.h"
#include "Types/Person.h"

#include "Types/RoadSegment.h"

namespace Can
{
	Building::Building(
		Prefab* prefab,
		s64 connectedRoadSegment,
		u64 snapped_t_index,
		f32 snapped_t,
		const glm::vec3& position,
		const glm::vec3& rotation
	)	: connectedRoadSegment(connectedRoadSegment)
		, snapped_t_index(snapped_t_index)
		, snapped_t(snapped_t)
		, object(new Object(prefab, position, rotation))
		, position(position)
	{

	}
}