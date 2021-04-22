#pragma once

#include "Can/Renderer/Object.h"

namespace Can
{
	class RoadSegment;
	class RoadNode
	{
	public:
		RoadNode(const std::vector<RoadSegment*>& roadSegments, const glm::vec3& position);
		~RoadNode() { delete object; }
	
		void Reconstruct();

		void AddRoadSegment(RoadSegment* roadSegment);
		void RemoveRoadSegment(RoadSegment* roadSegment);

		Object* object = nullptr;
		std::vector<RoadSegment*> roadSegments;
	
		glm::vec3 position;
	};
}