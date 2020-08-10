#pragma once
#include "Can.h"

namespace Can
{
	class Road;

	class Junction
	{
	public:
		Junction(const glm::vec3& position = { 0.0f, 0.0f, 0.0f });
		Junction(const std::vector<Road*>& connectedRoads, const glm::vec3& position = { 0.0f, 0.0f, 0.0f });
		Junction(const std::vector<Object*>& object, const glm::vec3& position = { 0.0f, 0.0f, 0.0f });
		~Junction() = default;

	public:
		void ConstructObject();
		void ReconstructObject();


	public:
		glm::vec3 position;

		std::vector<Object*> junctionPieces;
		std::vector<Road*> connectedRoads;

	};
}