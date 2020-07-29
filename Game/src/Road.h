#pragma once
#include "Can.h"

namespace Can
{
	class Junction;
	class End;

	class Road
	{
	public:
		Road();
		~Road();

	public:
		glm::vec3 startPos;
		glm::vec3 endPos;
		Can::Object* object;
		Junction* startJunction;
		Junction* endJunction;
		End* startEnd;
		End* endEnd;
	};
}