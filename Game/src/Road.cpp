#include "canpch.h"
#include "Road.h"

#include "Junction.h"
#include "End.h"

#include "Helper.h"

namespace Can
{
    Road::Road(Prefab* prefab, const glm::vec3& startPos, const glm::vec3& endPos)
        : startPosition(startPos)
        , endPosition(endPos)
        , direction(glm::normalize(endPos - startPos))
        , length(glm::length(endPos - startPos))
        , object(Helper::ConstructRoadObject(prefab,startPos, endPos))
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