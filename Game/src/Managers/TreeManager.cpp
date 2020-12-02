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

#include <gl/GL.h>

namespace Can
{
	TreeManager::TreeManager(GameScene* scene)
		: m_Scene(scene)
	{
		m_Guideline = new Object(m_Scene->MainApplication->trees[m_Type], m_Scene->MainApplication->trees[m_Type], glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), false);


		Ref<Texture2D> treeMap = m_Scene->MainApplication->treeMap;
		treeMap->Bind();
		GLubyte* pixels = new GLubyte[(size_t)treeMap->GetWidth() * (size_t)treeMap->GeHeight() * 4];
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

		int r, g, b, a; // or GLubyte r, g, b, a;
		size_t elmes_per_line = (size_t)treeMap->GetWidth() * 4; // elements per line = 256 * "RGBA"

		size_t jump = 6;
		float halfOffset = jump / (TERRAIN_SCALE_DOWN * 2.0f);

		for (size_t y = jump / 2; y < treeMap->GeHeight(); y += jump)
		{
			for (size_t x = jump / 2; x < treeMap->GetWidth(); x += jump)
			{
				size_t row = y * elmes_per_line;
				size_t col = x * 4;

				r = (int)pixels[row + col];
				g = (int)pixels[row + col + 1U];
				b = (int)pixels[row + col + 2U];
				a = (int)pixels[row + col + 3U];

				int n = Utility::Random::Integer(256);
				if (n < a)
				{
					using namespace Can::Utility;
					std::vector<int> colors = {};
					if (r > 128)
						colors.push_back(0);
					if (g > 128)
						colors.push_back(1);
					if (b > 128)
						colors.push_back(2);

					int type = colors[Random::Integer((int)colors.size())];
					glm::vec3 offsetPos{ Random::Float(halfOffset), 0.0f, Random::Float(halfOffset) };
					glm::vec3 randomRot{ 0.0f, Random::Float(-glm::radians(90.0f),glm::radians(90.0f)), 0.0f };
					glm::vec3 randomScale{ Random::Float(-0.2, 0.2),  Random::Float(-0.2, 0.2),  Random::Float(-0.2, 0.2) };
					if (type == 0)
					{
						Object* tree = new Object(
							m_Scene->MainApplication->trees[0],
							m_Scene->MainApplication->trees[0],
							offsetPos + glm::vec3{ (float)x / TERRAIN_SCALE_DOWN, 0.0f, -((float)y / TERRAIN_SCALE_DOWN) },
							randomScale + glm::vec3{ 1.0f, 1.0f, 1.0f },
							randomRot + glm::vec3{ 0.0f, 0.0f, 0.0f }
						);
						m_Trees.push_back(tree);
					}
					else if (type == 1)
					{
						Object* tree = new Object(
							m_Scene->MainApplication->trees[1],
							m_Scene->MainApplication->trees[1],
							offsetPos + glm::vec3{ (float)x / TERRAIN_SCALE_DOWN, 0.0f, -((float)y / TERRAIN_SCALE_DOWN) },
							randomScale + glm::vec3{ 1.0f, 1.0f, 1.0f },
							randomRot + glm::vec3{ 0.0f, 0.0f, 0.0f }
						);
						m_Trees.push_back(tree);
					}
				}
			}
		}
		std::cout << m_Trees.size() << std::endl;
	}
	TreeManager::~TreeManager()
	{
	}

	void TreeManager::OnUpdate(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
	{
		switch (m_ConstructionMode)
		{
		case TreeConstructionMode::None:
			break;
		case TreeConstructionMode::Adding:
			OnUpdate_Adding(prevLocation, cameraPosition, cameraDirection);
			break;
		case TreeConstructionMode::Removing:
			OnUpdate_Removing(prevLocation, cameraPosition, cameraDirection);
			break;
		default:
			break;
		}
	}
	void TreeManager::OnUpdate_Adding(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
	{
		b_AddingRestricted = false;
		m_Guideline->SetTransform(prevLocation);
		m_GuidelinePosition = prevLocation;
		m_Guideline->tintColor = glm::vec4(1.0f);

		bool collidedWithRoad = false;
		if (m_Scene->m_RoadManager.restrictions[2] && restrictions[0])
		{
			glm::vec2 treeL = { m_Guideline->prefab->boundingBoxL.x, m_Guideline->prefab->boundingBoxL.z };
			glm::vec2 treeM = { m_Guideline->prefab->boundingBoxM.x, m_Guideline->prefab->boundingBoxM.z };
			for (Road* road : m_Scene->m_RoadManager.GetRoads())
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
		if (m_Scene->m_BuildingManager.restrictions[0] && restrictions[0])
		{

			glm::vec2 treeL = { m_Guideline->prefab->boundingBoxL.x, m_Guideline->prefab->boundingBoxL.z };
			glm::vec2 treeM = { m_Guideline->prefab->boundingBoxM.x, m_Guideline->prefab->boundingBoxM.z };
			glm::vec2 treeP = { m_Guideline->position.x, m_Guideline->position.z };
			for (Building* building : m_Scene->m_BuildingManager.GetBuildings())
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

	bool TreeManager::OnMousePressed(MouseCode button)
	{
		switch (m_ConstructionMode)
		{
		case TreeConstructionMode::None:
			break;
		case TreeConstructionMode::Adding:
			OnMousePressed_Adding();
			break;
		case TreeConstructionMode::Removing:
			OnMousePressed_Removing();
			break;
		default:
			break;
		}
		return false;
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