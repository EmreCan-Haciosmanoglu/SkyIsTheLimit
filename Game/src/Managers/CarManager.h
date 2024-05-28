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
	};
	void remove_car(Car* c);
	void update_car(Car* const c, TimeStep ts);
}