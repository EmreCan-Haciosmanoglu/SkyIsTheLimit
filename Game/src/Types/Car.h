#pragma once
#include "Can.h"

namespace Can
{
	class RoadSegment;
	class Car
	{
	public:
		Car(
			Prefab* type,
			RoadSegment* roadSegment,
			size_t t_index,
			float speed,
			const glm::vec3& position,
			const glm::vec3& target,
			const glm::vec3& rotation
		);
		~Car();
		RoadSegment* roadSegment;
		size_t t_index = 0;
		float speed = 100.0f;

		Object* object;
		glm::vec3 position;
		glm::vec3 target;
		bool fromStart = false;
	};
}
