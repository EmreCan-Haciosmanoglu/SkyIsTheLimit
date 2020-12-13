#pragma once
#include "Can.h"

namespace Can
{
	class RoadSegment;

	class End
	{
	public:
		End(RoadSegment* connectedRoadSegment, Prefab* prefab, const glm::vec3& position = glm::vec3(0.0f), const glm::vec3& scale = glm::vec3(1.0f), const glm::vec3& rotation = glm::vec3(0.0f));
		End(RoadSegment* connectedRoadSegment, Object* object);
		~End();

	public:

		glm::vec3 position;

		Can::Object* object;
		RoadSegment* connectedRoadSegment;
	};
}