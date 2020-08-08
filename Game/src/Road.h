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
		Road(const Ref<Prefab>& prefab, const glm::vec3& startPos, const glm::vec3& endPos, const std::array<Ref<Prefab>, 3> type);
		Road(Object* object, const glm::vec3& startPos, const glm::vec3& endPos, const std::array<Ref<Prefab>, 3> type);
		~Road();

	public:
		void ConstructObject(const Ref<Prefab>& prefab);
		void ReconstructObject(const Ref<Prefab>& prefab);

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

		Object* object;

		Junction* startJunction = nullptr;
		Junction* endJunction = nullptr;
		End* startEnd = nullptr;
		End* endEnd = nullptr;

		std::array<Ref<Prefab>, 3> type;
		// 0 => Road
		// 1 => Junction
		// 2 => End
	};
}