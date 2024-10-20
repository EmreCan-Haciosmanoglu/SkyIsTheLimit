#pragma once

#include "Can/Renderer/Object.h"
#include "Types/Car.h"

namespace Can
{
	class GameScene;
	class RoadSegment;

	class CarManager
	{

	public:
		CarManager(GameScene* scene);
		~CarManager() = default;

		GameScene* m_Scene = nullptr;
		std::vector<Car*> m_Cars = {};
		std::vector<Car*> get_cars_on_the_road();
	};
	void remove_car(Car* c);
	void update_cars(TimeStep ts);
	void set_car_target_and_direction(Car* car, const v3& target);
}