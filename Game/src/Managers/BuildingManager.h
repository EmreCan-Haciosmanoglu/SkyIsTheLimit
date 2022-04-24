#pragma once

#include "Can/Renderer/Object.h"

namespace Can
{
	class GameScene;
	class Building;
	class RoadSegment;

	enum class BuildingConstructionMode
	{
		Construct,
		Upgrade,
		Destruct,
		None
	};

	class BuildingManager
	{
	public:
		BuildingManager(GameScene* scene);
		~BuildingManager();

		void OnUpdate(v3& prevLocation, const v3& cameraPosition, const v3& cameraDirection);
		void OnUpdate_Construction(v3& prevLocation, const v3& cameraPosition, const v3& cameraDirection);
		void OnUpdate_Destruction(v3& prevLocation, const v3& cameraPosition, const v3& cameraDirection);

		bool OnMousePressed(MouseCode button);
		bool OnMousePressed_Construction();
		bool OnMousePressed_Destruction();

		void SetType(size_t type);
		inline size_t GetType() { return m_Type; }

		void SetConstructionMode(BuildingConstructionMode mode);
		
		inline const BuildingConstructionMode GetConstructionMode() const { return m_ConstructionMode; }
		inline BuildingConstructionMode GetConstructionMode() { return m_ConstructionMode; }

		inline const std::vector<Building*>& GetBuildings() const { return m_Buildings; }
		inline std::vector<Building*>& GetBuildings() { return m_Buildings; }

		void ResetStates();
		Building* getAvailableWorkBuilding();
	public:

		std::array<bool, 2> snapOptions { true, true };
		// 0 : Roads
		// 1 : Buildings

		std::array<bool, 2> restrictions { true, true };
		// 0 : Collision
		// 1 : Snapping to a road

		GameScene* m_Scene = nullptr;

		BuildingConstructionMode m_ConstructionMode = BuildingConstructionMode::None;

		size_t m_Type = 0;

		std::vector<Building*> m_Buildings;
		std::vector<Building*> m_HomeBuildings;
		std::vector<Building*> m_WorkBuildings;

		v3 m_GuidelinePosition = v3(0.0f);
		v3 m_GuidelineRotation = v3(0.0f);

		u64 m_SnappedRoadSegment = (u64)-1;
		u64 snapped_t_index = 0;
		f32 snapped_t = -1.0f;
		bool snapped_from_right = false;

		std::vector<Building*>::iterator& m_SelectedBuildingToDestruct = m_Buildings.end();

		Object* m_Guideline = nullptr;

		bool b_ConstructionRestricted = false;
	};

	void remove_building(Building* b);
}