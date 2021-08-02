#include "canpch.h"
#include "TreeManager.h"

#include "Types/RoadSegment.h"
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

		int r, g, b, a; // or GLubyte r, g, b, a;
		u64 elmes_per_line = (u64)treeMap->GetWidth() * 4; // elements per line = 256 * "RGBA"

		u64 jump = 6;
		f32 halfOffset = jump / (TERRAIN_SCALE_DOWN * 2.0f);
		/*
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
		b_AddingRestricted = false;
		m_Guideline->SetTransform(prevLocation);
		m_GuidelinePosition = prevLocation;

		bool collidedWithRoad = false;
		if ((m_Scene->m_RoadManager.restrictionFlags & 0x4/*Change with #define*/) && restrictions[0])
		{
			v2 pos = (v2)m_Guideline->position;
			v2 A = (v2)m_Guideline->prefab->boundingBoxL;
			v2 D = (v2)m_Guideline->prefab->boundingBoxM;
			v2 B{ A.x, D.y }; // this is faster right???
			v2 C{ D.x, A.y }; // this is faster right???

			f32 rot = m_Guideline->rotation.y;
			A = Math::RotatePoint(A, rot) + pos;
			B = Math::RotatePoint(B, rot) + pos;
			C = Math::RotatePoint(C, rot) + pos;
			D = Math::RotatePoint(D, rot) + pos;

			std::array<std::array<v2, 3>, 2> polygonTree = {
				std::array<v2,3>{A, B, D},
				std::array<v2,3>{A, C, D}
			};

			for (RoadSegment& rs : m_Scene->m_RoadManager.m_Segments)
			{
				if (rs.elevation_type == -1)
					continue;
				f32 roadPrefabWidth = rs.type.road_width;
				const std::array<v3, 4>& cps = rs.GetCurvePoints();
				std::array<std::array<v2, 3>, 2> newRoadBoundingBox = Math::GetBoundingBoxOfBezierCurve(cps, roadPrefabWidth * 0.5f);

				if (Math::CheckPolygonCollision(newRoadBoundingBox, polygonTree))
				{
					std::array<std::array<v2, 3>, (10 - 1) * 2> newRoadBoundingPolygon = Math::GetBoundingPolygonOfBezierCurve<10, 10>(cps, roadPrefabWidth * 0.5f);
					if (Math::CheckPolygonCollision(newRoadBoundingPolygon, polygonTree))
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
				v2 mtv = Helper::CheckRotatedRectangleCollision(
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

		for (auto& it = m_Trees.begin(); it != m_Trees.end(); ++it)
		{
			Object* tree = *it;
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

		for (Object* tree : m_Trees)
			tree->tintColor = v4(1.0f);

		m_Guideline->enabled = false;
		m_Guideline->tintColor = v4(1.0f);
	}
}