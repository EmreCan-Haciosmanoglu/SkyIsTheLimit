#pragma once
#include "Can.h"


namespace Can
{
	class RoadSegment;
	class Building;
	class Car;
	class Person
	{
	public:
		Person(
			Prefab* type,
			RoadSegment* roadSegment,
			size_t t_index,
			float speed,
			const glm::vec3& position,
			const glm::vec3& target,
			const glm::vec3& rotation
		);
		~Person() { delete object; }

		// Simulation
		RoadSegment* roadSegment;
		size_t t_index = 0;
		float speed = 10.0f;
		float t = 0;
		std::array<glm::vec3, 3> driftpoints;
		glm::vec3 position;
		glm::vec3 target;
		bool fromStart = false;
		bool inJunction = false;
		// 
		Object* object;
		std::string firstName ="Adam";
		std::string midName ="Madam";
		std::string surName ="Tadaam";
		Building* home = nullptr;
		Building* work = nullptr;
		Car* iCar = nullptr;

	};
}