#pragma once

#include "Can/Renderer/Object.h"

namespace Can
{
	class RoadNode
	{
	public:
		RoadNode(const std::vector<u64>& roadSegments, const v3& position);
		RoadNode(RoadNode&& other);
		~RoadNode();
	
		void Reconstruct();

		void AddRoadSegment(u64 roadSegment);
		void RemoveRoadSegment(u64 roadSegment);

		Object* object = nullptr;
		std::vector<u64> roadSegments;
	
		v3 position = v3(0.0f);
	};
}