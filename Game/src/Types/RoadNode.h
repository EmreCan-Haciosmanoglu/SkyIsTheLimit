#pragma once

#include "Can/Renderer/Object.h"

namespace Can
{
	class Person;
	class RoadNode
	{
	public:
		RoadNode() {}
		RoadNode(const std::vector<u64>& roadSegments, const v3& position, u8 elevation_type);
		RoadNode(RoadNode&& other);
		~RoadNode();
		RoadNode& operator=(RoadNode&& other);
		friend bool operator==(const RoadNode& left, const RoadNode& right) { return &left == &right; }

		void Reconstruct();

		void AddRoadSegment(std::vector<u64> arr);
		void RemoveRoadSegment(u64 roadSegment);

		Object* object = nullptr;
		std::vector<u64> roadSegments;
		std::vector<Person*> people{};

		v3 position = v3(0.0f);
		u64 index = (u64)(-1);

		// better name
		s8 elevation_type = 0;
		// -1 tunnel
		// +0 ground
		// +1 bridge

		std::vector<std::array<v3, 3>> bounding_polygon{};
	};
}