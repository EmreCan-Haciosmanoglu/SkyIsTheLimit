#include "canpch.h"
#include "Road.h"

#include "Junction.h"
#include "End.h"

#include "Helper.h"

namespace Can
{
	Road::Road(const Ref<Prefab>& prefab, const glm::vec3& startPos, const glm::vec3& endPos)
		: startPosition(startPos)
		, endPosition(endPos)
		, direction(glm::normalize(endPos - startPos))
		, rotation({ 0.0f, glm::atan(direction.y / direction.x), glm::atan(direction.z / direction.x) + (direction.x <= 0 ? glm::radians(180.0f) : 0.0f) })
		, length(glm::length(endPos - startPos))
		, object(Helper::ConstructRoadObject(this, prefab, startPos, endPos))
	{
	}
	Road::Road(Object* object, const glm::vec3& startPos, const glm::vec3& endPos)
		: startPosition(startPos)
		, endPosition(endPos)
		, direction(glm::normalize(endPos - startPos))
		, length(glm::length(endPos - startPos))
		, object(object)
	{
	}
	Road::~Road()
	{
		delete object;
	}
}