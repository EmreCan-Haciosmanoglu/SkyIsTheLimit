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
		Road(Road* road, const glm::vec3& startPos, const glm::vec3& endPos);
		Road(Prefab* prefab, const glm::vec3& startPos, const glm::vec3& endPos, const std::array<Prefab*, 3>& type, size_t typeIndex);
		Road(Object* object, const glm::vec3& startPos, const glm::vec3& endPos, const std::array<Prefab*, 3>& type, size_t typeIndex);
		~Road();

	public:
		void ConstructObject(Prefab* prefab);
		void ReconstructObject(Prefab* prefab);

		inline const glm::vec3& GetStartPosition() const { return startPosition; }
		inline const glm::vec3& GetEndPosition() const { return endPosition; }

		void SetStartPosition(const glm::vec3& pos);
		void SetEndPosition(const glm::vec3& pos);

	private:
		glm::vec3 startPosition;
		glm::vec3 endPosition;

	public:

		glm::vec3 direction;
		glm::vec3 rotation; // Maybe duplicated
		float length;

		Object* object = nullptr;

		Junction* startJunction = nullptr;
		Junction* endJunction = nullptr;
		End* startEnd = nullptr;
		End* endEnd = nullptr;

		size_t typeIndex;
		std::array<Prefab*, 3> type = { nullptr, nullptr, nullptr };
	};
}