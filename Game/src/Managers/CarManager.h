#pragma once

#include "Can/Renderer/Object.h"
#include "Types/Car.h"

namespace Can
{
	class GameScene;
	class RoadSegment;

	enum class CarConstructionMode
	{
		Adding,
		Removing,
		None
	};

	class CarManager
	{

	public:
		CarManager(GameScene* scene);
		~CarManager();

		void OnUpdate(v3& prevLocation, const v3& cameraPosition, const v3& cameraDirection);
		void OnUpdate_Adding(v3& prevLocation, const v3& cameraPosition, const v3& cameraDirection);
		void OnUpdate_Removing(v3& prevLocation, const v3& cameraPosition, const v3& cameraDirection);

		bool OnMousePressed(MouseCode button);
		bool OnMousePressed_Adding();
		bool OnMousePressed_Removing();

		void SetType(u64 type);
		inline u64 GetType() { return m_Type; }

		void SetConstructionMode(CarConstructionMode mode);

		inline const CarConstructionMode GetConstructionMode() const { return m_ConstructionMode; }
		inline CarConstructionMode GetConstructionMode() { return m_ConstructionMode; }

		inline const std::vector<Car*>& GetCars() const { return m_Cars; }
		inline std::vector<Car*>& GetCars() { return m_Cars; }

		void ResetStates();


		GameScene* m_Scene = nullptr;

		CarConstructionMode m_ConstructionMode = CarConstructionMode::None;

		u64 m_Type = 0;

		std::vector<Car*> m_Cars = {};

		s64 m_SnappedRoadSegment = -1;
		

		std::vector<Car*>::iterator& m_SelectedCarToRemove = m_Cars.end();

		Object* m_Guideline = nullptr;
	};
	void remove_car(Car* c);
}