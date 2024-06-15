#include "canpch.h"
#include "TreeManager.h"

#include "Types/RoadSegment.h"
#include "Types/Tree.h"
#include "Building.h"

#include "GameApp.h"
#include "Scenes/GameScene.h"
#include "RoadManager.h"
#include "BuildingManager.h"
#include "Helper.h"

#include <gl/GL.h>

#include "Can/Math.h"

namespace Can
{
	TreeManager::TreeManager(GameScene* scene)
		: m_Scene(scene)
	{
		m_Guideline = new Object(m_Scene->MainApplication->trees[m_Type]);
		m_Guideline->enabled = false;

		Ref<Texture2D> treeMap = m_Scene->MainApplication->treeMap;
		treeMap->Bind();
		GLubyte* pixels = new GLubyte[(u64)treeMap->GetWidth() * (u64)treeMap->GetHeight() * 4];
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

		u64 elmes_per_line = (u64)treeMap->GetWidth() * 4; // elements per line = 256 * "RGBA"

		u64 jump = 6;
		f32 halfOffset = jump / (TERRAIN_SCALE_DOWN * 2.0f);
		/*
		int r, g, b, a; // or GLubyte r, g, b, a;
		for (u64 y = jump / 2; y < treeMap->GeHeight(); y += jump)
		{
			for (u64 x = jump / 2; x < treeMap->GetWidth(); x += jump)
			{
				u64 row = y * elmes_per_line;
				u64 col = x * 4;

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
					if (colors.size() != 0)
					{
						int type = colors[Random::Integer((int)colors.size())];
						v3 offsetPos{ Random::f32(halfOffset), 0.0f, Random::f32(halfOffset) };
						v3 randomRot{ 0.0f, Random::f32(-glm::radians(90.0f),glm::radians(90.0f)), 0.0f };
						v3 randomScale{ Random::f32(-0.2f, 0.2f),  Random::f32(-0.2f, 0.2f),  Random::f32(-0.2f, 0.2f) };
						if (type == 0)
						{
							Object* tree = new Object(
								m_Scene->MainApplication->trees[0],
								offsetPos + v3{ (f32)x / TERRAIN_SCALE_DOWN, 0.0f, -((f32)y / TERRAIN_SCALE_DOWN) },
								randomRot + v3{ 0.0f, 0.0f, 0.0f },
								randomScale + v3{ 1.0f, 1.0f, 1.0f }
							);
							m_Trees.push_back(tree);
						}
						else if (type == 1)
						{
							Object* tree = new Object(
								m_Scene->MainApplication->trees[1],
								offsetPos + v3{ (f32)x / TERRAIN_SCALE_DOWN, 0.0f, -((f32)y / TERRAIN_SCALE_DOWN) },
								randomRot + v3{ 0.0f, 0.0f, 0.0f },
								randomScale + v3{ 1.0f, 1.0f, 1.0f }
							);
							m_Trees.push_back(tree);
						}
					}
				}
			}
		}
		*/std::cout << m_Trees.size() << " trees are generated!" << std::endl;
	}
	TreeManager::~TreeManager()
	{
	}

	void TreeManager::OnUpdate(v3& prevLocation, const v3& cameraPosition, const v3& cameraDirection)
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
	void TreeManager::OnUpdate_Adding(v3& prevLocation, const v3& cameraPosition, const v3& cameraDirection)
	{
		GameApp* app = m_Scene->MainApplication;
		b_AddingRestricted = false;
		m_Guideline->SetTransform(prevLocation);
		m_GuidelinePosition = prevLocation;

		bool collidedWithRoad = false;
		if ((m_Scene->m_RoadManager.restrictionFlags & (u8)RoadRestrictions::RESTRICT_COLLISIONS) && restrictions[0])
		{
			Prefab* prefab = m_Guideline->prefab;
			f32 tree_height = prefab->boundingBoxM.z - prefab->boundingBoxL.z;

			v3 A = v3{ prefab->boundingBoxL.x, prefab->boundingBoxL.y, prefab->boundingBoxL.z };
			v3 B = v3{ prefab->boundingBoxL.x, prefab->boundingBoxM.y, prefab->boundingBoxL.z };
			v3 C = v3{ prefab->boundingBoxM.x, prefab->boundingBoxL.y, prefab->boundingBoxL.z };
			v3 D = v3{ prefab->boundingBoxM.x, prefab->boundingBoxM.y, prefab->boundingBoxL.z };


			f32 rot = m_Guideline->rotation.z;
			A = glm::rotateZ(A, rot) + m_Guideline->position;
			B = glm::rotateZ(B, rot) + m_Guideline->position;
			C = glm::rotateZ(C, rot) + m_Guideline->position;
			D = glm::rotateZ(D, rot) + m_Guideline->position;

			std::vector<std::array<v3, 3>> tree_bounding_polygon = {
				std::array<v3, 3>{A, B, D},
				std::array<v3, 3>{A, C, D}
			};

			std::array<v3, 2> tree_bounding_box{
				v3{std::min({A.x, B.x, C.x, D.x}), std::min({A.y, B.y, C.y, D.y}), A.z},
				v3{std::max({A.x, B.x, C.x, D.x}), std::max({A.y, B.y, C.y, D.y}), A.z + tree_height}
			};

			auto& segments = m_Scene->m_RoadManager.road_segments;
			u64 capacity = segments.capacity;
			for(u64 i = 0; i<capacity;i++)
			{
				auto& value = segments.values[i];
				if (value.valid == false)
					continue;
				RoadSegment& rs = value.value;
				RoadType& type = app->road_types[rs.type];
				if (rs.elevation_type == -1)
					continue;

				f32 road_height = rs.elevation_type == -1 ? type.tunnel_height : type.road_height;
				std::array<v3, 2> road_bounding_box{
					rs.object->prefab->boundingBoxL + rs.CurvePoints[0],
					rs.object->prefab->boundingBoxM + rs.CurvePoints[0]
				};

				if (Math::check_bounding_box_bounding_box_collision(road_bounding_box, tree_bounding_box))
				{
					if (Math::check_bounding_polygon_bounding_polygon_collision_with_z(rs.bounding_polygon, road_height, tree_bounding_polygon, tree_height))
					{
						collidedWithRoad = true;
						break;
					}
				}
			}
		}

		bool collidedWithBuilding = false;
		if (m_Scene->m_BuildingManager.restrictions[0] && restrictions[0])
		{

			v2 treeL = (v2)m_Guideline->prefab->boundingBoxL;
			v2 treeM = (v2)m_Guideline->prefab->boundingBoxM;
			v2 treeP = (v2)m_Guideline->position;
			for (Building* building : m_Scene->m_BuildingManager.GetBuildings())
			{
				v2 buildingL = (v2)building->object->prefab->boundingBoxL;
				v2 buildingM = (v2)building->object->prefab->boundingBoxM;
				v2 buildingP = (v2)building->object->position;
				v2 mtv = Helper::check_rotated_rectangle_collision(
					treeL,
					treeM,
					0.0f,
					treeP,
					buildingL,
					buildingM,
					building->object->rotation.z,
					buildingP
				);
				if (glm::length(mtv) > 0.0f)
				{
					collidedWithBuilding = true;
					break;
				}
			}
		}

		b_AddingRestricted |= collidedWithRoad;
		b_AddingRestricted |= collidedWithBuilding;

		m_Guideline->tintColor = b_AddingRestricted ? v4{ 1.0f, 0.3f, 0.2f, 1.0f } : v4(1.0f);
	}
	void TreeManager::OnUpdate_Removing(v3& prevLocation, const v3& cameraPosition, const v3& cameraDirection)
	{
		m_SelectedTreeToRemove = m_Trees.end();

		for (auto it = m_Trees.cbegin(); it != m_Trees.cend(); ++it)
		{
			Object* tree = (*it)->object;
			tree->tintColor = v4(1.0f);

			if (Helper::CheckBoundingBoxHit(
				cameraPosition,
				cameraDirection,
				tree->prefab->boundingBoxL + tree->position,
				tree->prefab->boundingBoxM + tree->position
			))
			{
				m_SelectedTreeToRemove = it;
				tree->tintColor = v4{ 1.0f, 0.3f, 0.2f, 1.0f };
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
			v3 randomRot{ 0.0f, 0.0f, Random::Float(-glm::radians(90.0f),glm::radians(90.0f)) };
			v3 randomScale{ Random::Float(-0.2f, 0.2f),  Random::Float(-0.2f, 0.2f),  Random::Float(-0.2f, 0.2f) };
			Object* tree = new Object(
				m_Scene->MainApplication->trees[m_Type],
				m_GuidelinePosition,
				randomRot,
				randomScale + v3{ 1.0f, 1.0f, 1.0f }
			);
			m_Trees.push_back(new Tree{ m_Type, tree });
			ResetStates();
			m_Guideline->enabled = true;
		}
		return false;
	}
	bool TreeManager::OnMousePressed_Removing()
	{
		if (m_SelectedTreeToRemove != m_Trees.end())
		{
			Object* tree = (* m_SelectedTreeToRemove)->object;
			m_Trees.erase(m_SelectedTreeToRemove);

			delete tree;
		}
		return false;
	}

	void TreeManager::SetType(u64 type)
	{
		m_Type = type;
		delete m_Guideline;
		m_Guideline = new Object(m_Scene->MainApplication->trees[m_Type]);
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
		m_SelectedTreeToRemove = m_Trees.end();
		m_GuidelinePosition = v3(-1.0f);

		for (Tree* tree : m_Trees)
			tree->object->tintColor = v4(1.0f);

		m_Guideline->enabled = false;
		m_Guideline->tintColor = v4(1.0f);
	}
}