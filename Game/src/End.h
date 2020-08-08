#pragma once
#include "Can.h"

namespace Can
{
	class Road;

	class End
	{
	public:
		End(Road* connectedRoad, const Ref<Prefab>& prefab);
		End(Road* connectedRoad, const Ref<Prefab>& prefab, const glm::vec3& position);
		End(Road* connectedRoad, const Ref<Prefab>& prefab, const glm::vec3& position, const glm::vec3& scale);
		End(Road* connectedRoad, const Ref<Prefab>& prefab, const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation);
		End(Road* connectedRoad, Object* object);
		~End();

	public:
		Can::Object* object;
		Road* connectedRoad;
	};
}