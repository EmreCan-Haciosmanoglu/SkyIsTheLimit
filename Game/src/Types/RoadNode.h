#pragma once

#include "Can/Renderer/Object.h"

namespace Can
{
	class RoadSegment;
	struct sort_by_angle
	{
		inline bool operator() (const RoadSegment* roadSegment1, const RoadSegment* roadSegment2)
		{
			float roadSegmentR1 = 0.001f;
			float roadSegmentR2 = 0.002f;
			if (
				roadSegment1->EndNode != nullptr &&
				roadSegment2->EndNode != nullptr &&
				roadSegment1->EndNode == roadSegment2->EndNode
				)
			{
				roadSegmentR1 = roadSegment1->GetEndRotation().y;
				roadSegmentR2 = roadSegment2->GetEndRotation().y;
			}
			else if (
				roadSegment1->EndNode != nullptr &&
				roadSegment2->StartNode != nullptr &&
				roadSegment1->EndNode == roadSegment2->StartNode
				)
			{
				roadSegmentR1 = roadSegment1->GetEndRotation().y;
				roadSegmentR2 = roadSegment2->GetStartRotation().y;
			}
			else if (
				roadSegment1->StartNode != nullptr &&
				roadSegment2->EndNode != nullptr &&
				roadSegment1->StartNode == roadSegment2->EndNode
				)
			{
				roadSegmentR1 = roadSegment1->GetStartRotation().y;
				roadSegmentR2 = roadSegment2->GetEndRotation().y;
			}
			else if (
				roadSegment1->StartNode != nullptr &&
				roadSegment2->StartNode != nullptr &&
				roadSegment1->StartNode == roadSegment2->StartNode
				)
			{
				roadSegmentR1 = roadSegment1->GetStartRotation().y;
				roadSegmentR2 = roadSegment2->GetStartRotation().y;
			}
			roadSegmentR1 = std::fmod(roadSegmentR1 + glm::radians(360.0f), glm::radians(360.0f));
			roadSegmentR2 = std::fmod(roadSegmentR2 + glm::radians(360.0f), glm::radians(360.0f));
			return (roadSegmentR1 < roadSegmentR2);
		}
	};
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