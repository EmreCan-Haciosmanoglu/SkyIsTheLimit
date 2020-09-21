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
		Junction(Object* object, const glm::vec3& position = { 0.0f, 0.0f, 0.0f });
		~Junction();

	public:
		void ConstructObject();
		void ReconstructObject();


	public:
		glm::vec3 position;

		Object* object = nullptr;
		std::vector<Road*> connectedRoads;

	};
}