#pragma once
#include "Can.h"

namespace Can
{
	class Junction;
	class End;

	class Road
	{
	public:
		Road() = default;
		Road(const Ref<Prefab>& prefab, const glm::vec3& startPos, const glm::vec3& endPos);
		Road(Object* object, const glm::vec3& startPos, const glm::vec3& endPos);
		~Road();

	public:
		glm::vec3 startPosition;
		glm::vec3 endPosition;

		glm::vec3 direction;
		glm::vec3 rotation;
		float length;

		Can::Object* object;

		Junction* startJunction;
		Junction* endJunction;
		End* startEnd;
		End* endEnd;
	};
}