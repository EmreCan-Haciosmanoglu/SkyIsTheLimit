#pragma once
#include "Can.h"

namespace Can
{
	class Road;

	class Junction
	{
	public:
		Junction();
		~Junction();

	public:
		glm::vec3 position;
		Can::Object* object = nullptr;
		std::vector<Road*> connectedRoads;
	};
}