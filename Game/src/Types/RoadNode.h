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
		RoadNode& operator=(RoadNode&& other);
		friend bool operator==(const RoadNode& left, const RoadNode& right) { return &left == &right; }
	
		void Reconstruct();

		void AddRoadSegment(std::vector<u64> arr);
		void RemoveRoadSegment(u64 roadSegment);

		Object* object = nullptr;
		std::vector<u64> roadSegments;
	
		v3 position = v3(0.0f);
		u64 index = (u64)(-1);
	};
}