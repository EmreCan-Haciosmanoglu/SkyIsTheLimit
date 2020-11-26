#pragma once

#include "Can/Renderer/Object.h"

namespace Can
{
	class Building;

	enum class BuildingConstructionMode
	{
		None,
		Construct,
		Upgrade,
		Destruct
	};

	class BuildingManager
	{
	public:
		BuildingManager();
		~BuildingManager();

		void OnUpdate_Construction(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);
		void OnUpdate_Destruction(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);

		bool OnMousePressed_Construction();
		bool OnMousePressed_Destruction();

		void SetSelectedConstructionBuilding(size_t index);

		void SetConstructionMode(BuildingConstructionMode mode);
		
		inline const BuildingConstructionMode GetConstructionMode() const { return m_ConstructionMode; }
		inline BuildingConstructionMode GetConstructionMode() { return m_ConstructionMode; }

		void ResetStates();
	public:

		static std::array<bool, 2> snapOptions;
		// 0 : Roads
		// 1 : Buildings

		static std::array<bool, 2> restriction;
		// 0 : Collision
		// 1 : Snapping to a road

		BuildingConstructionMode m_ConstructionMode = BuildingConstructionMode::None;

		size_t m_BuildingType = 0;

		static std::vector<Building*> m_Buildings;

	private:

		// Building Construction Transforms
		glm::vec3 m_BuildingConstructionCoordinate = glm::vec3(-1.0f);
		glm::vec3 m_BuildingConstructionRotation = glm::vec3(0.0f);

		// Building Construction Snap
		Road* m_BuildingConstructionSnappedRoad = nullptr;

		// Building Destruction Snap
		//Building* m_BuildingDestructionSnappedBuilding = nullptr;
		std::vector<Building*>::iterator& m_BuildingDestructionSnappedBuilding = m_Buildings.end();

		Object* m_BuildingGuideline = nullptr;

		bool b_ConstructionRestricted = false;
	};
}