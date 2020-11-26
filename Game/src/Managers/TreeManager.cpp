#include "canpch.h"
#include "TreeManager.h"

#include "Road.h"
#include "Junction.h"
#include "Building.h"
#include "End.h"

#include "GameApp.h"
#include "Scenes/GameScene.h"
#include "RoadManager.h"
#include "BuildingManager.h"
#include "Helper.h"

namespace Can
{
	TreeManager::TreeManager(GameScene* scene)
		: m_Scene(scene)
	{
	}
	TreeManager::~TreeManager()
	{
	}

	void TreeManager::OnUpdate_Adding(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
	{
		b_AddingRestricted = false;
		m_Guideline->SetTransform(prevLocation);
		m_GuidelinePosition = prevLocation;
		m_Guideline->tintColor = glm::vec4(1.0f);

		bool collidedWithRoad = false;
		if (m_Scene->m_RoadManager->restrictions[2] && restrictions[0])
		{
			glm::vec2 treeL = { m_Guideline->prefab->boundingBoxL.x, m_Guideline->prefab->boundingBoxL.z };
			glm::vec2 treeM = { m_Guideline->prefab->boundingBoxM.x, m_Guideline->prefab->boundingBoxM.z };
			for (Road* road : m_Scene->m_RoadManager->GetRoads())
			{
				float roadHalfWidth = (road->object->prefab->boundingBoxM.z - road->object->prefab->boundingBoxL.z) / 2.0f;
				glm::vec2 roadL = { 0.0f, -roadHalfWidth };
				glm::vec2 roadM = { road->length, roadHalfWidth };
				glm::vec2 mtv = Helper::CheckRotatedRectangleCollision(
					roadL,
					roadM,
					road->rotation.y,
					glm::vec2{ road->GetStartPosition().x, road->GetStartPosition().z },
					treeL,
					treeM,
					0.0f,
					glm::vec2{ m_GuidelinePosition.x, m_GuidelinePosition.z }
				);
				if (mtv.x != 0.0f || mtv.y != 0.0f)
				{
					collidedWithRoad = true;
					m_Guideline->tintColor = glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f };
					break;
				}
			}
		}

		bool collidedWithBuilding = false;
		if (m_Scene->m_BuildingManager->restrictions[0] && restrictions[0])
		{

			glm::vec2 treeL = { m_Guideline->prefab->boundingBoxL.x, m_Guideline->prefab->boundingBoxL.z };
			glm::vec2 treeM = { m_Guideline->prefab->boundingBoxM.x, m_Guideline->prefab->boundingBoxM.z };
			glm::vec2 treeP = { m_Guideline->position.x, m_Guideline->position.z };
			for (Building* building : m_Scene->m_BuildingManager->GetBuildings())
			{
				glm::vec2 buildingL = { building->object->prefab->boundingBoxL.x, building->object->prefab->boundingBoxL.z };
				glm::vec2 buildingM = { building->object->prefab->boundingBoxM.x, building->object->prefab->boundingBoxM.z };
				glm::vec2 buildingP = { building->object->position.x, building->object->position.z };
				glm::vec2 mtv = Helper::CheckRotatedRectangleCollision(
					treeL,
					treeM,
					0.0f,
					treeP,
					buildingL,
					buildingM,
					building->object->rotation.y,
					buildingP
				);
				if (mtv.x != 0.0f || mtv.y != 0.0f)
				{
					collidedWithBuilding = true;
					m_Guideline->tintColor = glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f };
					break;
				}
			}
		}

		b_AddingRestricted = collidedWithRoad | collidedWithBuilding;
		m_Guideline->tintColor = b_AddingRestricted ? glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f } : glm::vec4(1.0f);
	}
	void TreeManager::OnUpdate_Removing(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
	{
		m_SelectedTreeToRemove = m_Trees.end();

		for (auto& it = m_Trees.begin(); it != m_Trees.end(); ++it)
		{
			Object* tree = *it;
			tree->tintColor = glm::vec4(1.0f);

			if (Helper::CheckBoundingBoxHit(
				cameraPosition,
				cameraDirection,
				tree->prefab->boundingBoxL + tree->position,
				tree->prefab->boundingBoxM + tree->position
			))
			{
				m_SelectedTreeToRemove = it;
				tree->tintColor = glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f };
				break;
			}
		}
	}

	bool TreeManager::OnMousePressed_Adding()
	{
		if (!b_AddingRestricted)
		{
			using namespace Utility;
			glm::vec3 randomRot{ 0.0f, Random::Float(-glm::radians(90.0f),glm::radians(90.0f)), 0.0f };
			glm::vec3 randomScale{ Random::Float(-0.2, 0.2),  Random::Float(-0.2, 0.2),  Random::Float(-0.2, 0.2) };
			Object* tree = new Object(
				m_Scene->MainApplication->trees[m_Type],
				m_Scene->MainApplication->trees[m_Type],
				m_GuidelinePosition,
				randomScale + glm::vec3{ 1.0f, 1.0f, 1.0f },
				randomRot + glm::vec3{ 0.0f, 0.0f, 0.0f }
			);
			m_Trees.push_back(tree);
			ResetStates();
			m_Guideline->enabled = true;
		}
		return false;
	}
	bool TreeManager::OnMousePressed_Removing()
	{
		if (m_SelectedTreeToRemove != m_Trees.end())
		{
			Object* tree = *m_SelectedTreeToRemove;
			m_Trees.erase(m_SelectedTreeToRemove);

			delete tree;
		}
		return false;
	}

	void TreeManager::SetType(size_t type)
	{
		m_Type = type;
		delete m_Guideline;
		m_Guideline = new Object(m_Scene->MainApplication->trees[m_Type], m_Scene->MainApplication->trees[m_Type], glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f));
	}
	void TreeManager::SetConstructionMode(TreeConstructionMode mode)
	{
		ResetStates();
		m_ConstructionMode = mode;

		switch (m_ConstructionMode)
		{
		case Can::TreeConstructionMode::None:
			break;
		case Can::TreeConstructionMode::Adding:
			m_Guideline->enabled = true;
			break;
		case Can::TreeConstructionMode::Removing:
			break;
		default:
			break;
		}
	}

	void TreeManager::ResetStates()
	{
		m_GuidelinePosition = glm::vec3(-1.0f);

		for (Object* tree : m_Trees)
			tree->tintColor = glm::vec4(1.0f);

		m_Guideline->enabled = false;
		m_Guideline->tintColor = glm::vec4(1.0f);
	}
}