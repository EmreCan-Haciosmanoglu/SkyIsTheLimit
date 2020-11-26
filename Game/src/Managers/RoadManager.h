#pragma once

#include "Can/Renderer/Object.h"

namespace Can
{
	class Road;
	class Junction;
	class End;

	enum class RoadConstructionMode
	{
		None,
		Construct,
		Upgrade,
		Destruct
	};

	struct RoadSnapInformation
	{
		bool snapped;
		glm::vec3 snapLocation;
		Junction* snappedJunction = nullptr;
		End* snappedEnd = nullptr;
		Road* snappedRoad = nullptr;
	};

	class RoadManager
	{
	public:
		RoadManager();
		~RoadManager();

		void OnUpdate_Construction(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);
		void OnUpdate_Destruction(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);

		bool OnMousePressed_Construction(const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);
		bool OnMousePressed_Destruction();

		void SetSelectedConstructionRoad(size_t index);

		void SetConstructionMode(RoadConstructionMode mode);

		inline const RoadConstructionMode GetConstructionMode() const { return m_ConstructionMode; }
		inline RoadConstructionMode GetConstructionMode() { return m_ConstructionMode; }

		void DeleteSelectedRoad(Road* road);

		RoadSnapInformation DidRoadSnapped(const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);

		void ResetStates();
	public:

		static std::array<bool, 5> snapOptions;
		// 0 : Roads
		// 1 : Length
		// 2 : Height
		// 3 : Angle
		// 4 : Grid

		static std::array<bool, 3> restrictions;
		// 0 : Small Angle
		// 1 : Short Length
		// 2 : Collision

	private:
		RoadConstructionMode m_ConstructionMode = RoadConstructionMode::Construct;

		std::vector<Road*> m_Roads;
		std::vector<Junction*> m_Junctions;
		std::vector<End*> m_Ends;

		bool b_RoadConstructionStarted = false;
		bool b_RoadConstructionEnded = false;
		bool b_RoadConstructionStartSnapped = false;
		bool b_RoadConstructionEndSnapped = false;

		// Road Construction Transforms
		glm::vec3 m_RoadConstructionStartCoordinate = glm::vec3(-1.0f);
		glm::vec3 m_RoadConstructionEndCoordinate = glm::vec3(-1.0f);

		// Road Construction Start Snap
		Junction* m_RoadConstructionStartSnappedJunction = nullptr;
		End* m_RoadConstructionStartSnappedEnd = nullptr;
		Road* m_RoadConstructionStartSnappedRoad = nullptr;

		// Road Construction End Snap
		Junction* m_RoadConstructionEndSnappedJunction = nullptr;
		End* m_RoadConstructionEndSnappedEnd = nullptr;
		Road* m_RoadConstructionEndSnappedRoad = nullptr;

		// Road Destruction Snap
		Junction* m_RoadDestructionSnappedJunction = nullptr;
		End* m_RoadDestructionSnappedEnd = nullptr;
		Road* m_RoadDestructionSnappedRoad = nullptr;

		std::vector<std::vector<Object*>> m_RoadGuidelines;
		std::vector<size_t> m_RoadGuidelinesInUse;
		Object* m_RoadGuidelinesStart = nullptr; // End /? Object
		Object* m_RoadGuidelinesEnd = nullptr;

		bool b_ConstructionRestricted = false;
	};
}