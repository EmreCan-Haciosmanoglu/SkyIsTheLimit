#include "canpch.h"
#include "RoadManager.h"

#include "Types/RoadSegment.h"
#include "Types/RoadNode.h"
#include "Types/Person.h"
#include "Types/Tree.h"
#include "Building.h"

#include "GameApp.h"
#include "Scenes/GameScene.h"
#include "TreeManager.h"
#include "BuildingManager.h"
#include "Helper.h"

#include "Can/Math.h"

namespace Can
{
	RoadManager::RoadManager(GameScene* scene)
		: m_Scene(scene)
	{
		const RoadType& type = m_Scene->MainApplication->road_types[m_Type];

		m_GroundGuidelinesStart = new Object(type.asymmetric ? type.road_end_mirror : type.road_end);
		m_GroundGuidelinesStart->enabled = false;
		m_GroundGuidelinesEnd = new Object(type.road_end);
		m_GroundGuidelinesEnd->enabled = false;

		m_TunnelGuidelinesStart = new Object(type.asymmetric ? type.tunnel_end_mirror : type.tunnel_end);
		m_TunnelGuidelinesStart->enabled = false;
		m_TunnelGuidelinesEnd = new Object(type.tunnel_end);
		m_TunnelGuidelinesEnd->enabled = false;

		u64 roadTypeCount = m_Scene->MainApplication->road_types.size();
		for (u64 i = 0; i < roadTypeCount; i++)
		{
			const RoadType& t = m_Scene->MainApplication->road_types[i];

			m_GroundGuidelinesInUse.push_back(0);
			m_GroundGuidelines.push_back({});
			m_GroundGuidelines[i].push_back(new Object(t.road));
			m_GroundGuidelines[i][0]->enabled = false;

			m_TunnelGuidelinesInUse.push_back(0);
			m_TunnelGuidelines.push_back({});
			m_TunnelGuidelines[i].push_back(new Object(t.tunnel));
			m_TunnelGuidelines[i][0]->enabled = false;
		}
	}
	RoadManager::~RoadManager()
	{
		for (std::vector<Object*>& objs : m_GroundGuidelines)
		{
			u64 index = objs.size();
			while (index != 0)
			{
				index--;
				Object* obj = objs[index];
				objs.pop_back();
				delete obj;
			}
		}
		for (std::vector<Object*>& objs : m_TunnelGuidelines)
		{
			u64 index = objs.size();
			while (index != 0)
			{
				index--;
				Object* obj = objs[index];
				objs.pop_back();
				delete obj;
			}
		}
		delete m_GroundGuidelinesStart;
		delete m_GroundGuidelinesEnd;
		delete m_TunnelGuidelinesStart;
		delete m_TunnelGuidelinesEnd;
	}

	void RoadManager::OnUpdate(v3& prevLocation, f32 ts)
	{
		switch (m_ConstructionMode)
		{
		case RoadConstructionMode::None:
			break;
		case RoadConstructionMode::Straight:
			OnUpdate_Straight(prevLocation, ts);
			break;
		case RoadConstructionMode::QuadraticCurve:
			OnUpdate_QuadraticCurve(prevLocation, ts);
			break;
		case RoadConstructionMode::CubicCurve:
			OnUpdate_CubicCurve(prevLocation, ts);
			break;
		case  RoadConstructionMode::Change:
			OnUpdate_Change(prevLocation, ts);
			break;
		case  RoadConstructionMode::Destruct:
			OnUpdate_Destruction(prevLocation, ts);
			break;
		}
	}
	void RoadManager::OnUpdate_Straight(v3& prevLocation, f32 ts)
	{
		RoadType& type = m_Scene->MainApplication->road_types[m_Type];
		m_Scene->m_Terrain->enabled = (m_ConstructionPhase == 0 && m_Elevationtypes[0] >= 0) || (m_ConstructionPhase != 0 && m_Elevationtypes[3] >= 0);

		// HACK
		static f32 cooldown = 0.0f;
		if (cooldown <= 0.0f)
		{
			if (Input::IsKeyPressed(KeyCode::PageUp))
			{
				m_CurrentElevation += 0.1f;
				cooldown = 0.15f;
			}
			else if (Input::IsKeyPressed(KeyCode::PageDown))
			{
				m_CurrentElevation -= 0.1f;
				cooldown = 0.15f;
			}
			else if (Input::IsKeyPressed(KeyCode::Home))
			{
				m_CurrentElevation = 0.0f;
			}
		}
		else
		{
			cooldown -= ts;
		}

		s8 elevationValue = 0;
		if (m_CurrentElevation < -type.tunnel_height)
			elevationValue = -1;
		else if (m_CurrentElevation > type.bridge_height)
			elevationValue = 1;

		// Delete this when bridges are added
		m_CurrentElevation = std::min(0.0f, m_CurrentElevation);

		if (m_ConstructionPhase == 0)
		{
			m_Elevationtypes[0] = elevationValue;
			m_Elevationtypes[3] = elevationValue;
			m_Elevations[0] = m_CurrentElevation;
			m_Elevations[3] = m_CurrentElevation;

			SnapToGrid(prevLocation);
			SnapToRoad(prevLocation, true);

			if (!b_ConstructionStartSnapped)
				prevLocation.z += m_CurrentElevation;
			m_ConstructionPositions[0] = prevLocation;

			m_GroundGuidelinesStart->SetTransform(prevLocation + v3{ 0.0f, 0.0f, 0.15f }, v3{ 0.0f, 0.0f, glm::radians(180.0f) });
			m_GroundGuidelinesEnd->SetTransform(prevLocation + v3{ 0.0f, 0.0f, 0.15f }, v3(0.0f));
			m_TunnelGuidelinesStart->SetTransform(prevLocation + v3{ 0.0f, 0.0f, 0.15f }, v3{ 0.0f, 0.0f, glm::radians(180.0f) });
			m_TunnelGuidelinesEnd->SetTransform(prevLocation + v3{ 0.0f, 0.0f, 0.15f }, v3(0.0f));

			m_GroundGuidelinesStart->enabled = elevationValue == 0;
			m_GroundGuidelinesEnd->enabled = elevationValue == 0;

			m_TunnelGuidelinesStart->enabled = elevationValue == -1;
			m_TunnelGuidelinesEnd->enabled = elevationValue == -1;
		}
		else
		{
			m_Elevationtypes[3] = elevationValue;
			m_Elevations[3] = m_CurrentElevation;

			ResetGuideLines();

			b_ConstructionRestricted = false;
			SnapToGrid(prevLocation);
			SnapToRoad(prevLocation, false);

			if (!b_ConstructionEndSnapped)
				prevLocation.z += m_CurrentElevation;
			m_ConstructionPositions[3] = prevLocation;

			// TODO: After angle snapping 
			v2 dir = (v2)(prevLocation - m_ConstructionPositions[0]);
			bool angleIsRestricted = RestrictSmallAngles(dir, m_StartSnappedNode, m_StartSnappedSegment, m_StartSnappedT);
			angleIsRestricted |= RestrictSmallAngles(-dir, m_EndSnappedNode, m_EndSnappedSegment, m_EndSnappedT);

			v3 AB = m_ConstructionPositions[3] - m_ConstructionPositions[0];


			if (!b_ConstructionEndSnapped)
			{
				f32 length = glm::length(AB);
				if ((snapFlags & (u8)RoadSnapOptions::SNAP_TO_LENGTH) && (length > 0.5f))
				{
					length = length - std::fmod(length, type.road_length);
					AB = length * glm::normalize(AB);
					m_ConstructionPositions[3] = m_ConstructionPositions[0] + AB;
				}
				SnapToHeight({ 3 }, 0, AB);
				SnapToAngle(AB, m_StartSnappedNode, m_StartSnappedSegment, m_StartSnappedT);
				m_ConstructionPositions[3] = m_ConstructionPositions[0] + AB;
			}

			v3 A = m_ConstructionPositions[0];
			v3 D = m_ConstructionPositions[3];
			v3 AD = D - A;
			if (glm::length(AD) == 0.0f)
				AD.x = 0.001f;
			AD.z = 0.0f;
			AD = type.road_width * 0.5f * glm::normalize(AD);

			if (!b_ConstructionStartSnapped) A -= AD;
			if (!b_ConstructionStartSnapped) D += AD;

			AD = v3{ -AD.y , AD.x , 0.0f };

			v3 P1 = A + AD;
			v3 P2 = A - AD;
			v3 P3 = D + AD;
			v3 P4 = D - AD;

			std::vector<std::array<v3, 3>> new_road_bounding_polygon{
					std::array<v3,3>{ P1, P2, P3},
					std::array<v3,3>{ P2, P3, P4}
			};
			f32 guideline_height = elevationValue == -1 ? type.tunnel_height : type.road_height;
			std::array<v3, 2> new_road_bounding_box{ /* Find Better Way*/
				v3{std::min({P1.x, P2.x, P3.x, P4.x}), std::min({P1.y, P2.y, P3.y, P4.y}), std::min({A.z, D.z}) },
				v3{std::max({P1.x, P2.x, P3.x, P4.x}), std::max({P1.y, P2.y, P3.y, P4.y}), std::max({A.z, D.z}) + guideline_height}
			};
			bool lengthIsRestricted = (restrictionFlags & (u8)RoadRestrictions::RESTRICT_SHORT_LENGTH) && (glm::length(AB) < (2.0f * type.road_length));
			bool collisionIsRestricted = (restrictionFlags & (u8)RoadRestrictions::RESTRICT_COLLISIONS) ? check_road_road_collision(new_road_bounding_box, new_road_bounding_polygon) : false;

			if (m_Scene->m_BuildingManager.restrictions[0] && (restrictionFlags & (u8)RoadRestrictions::RESTRICT_COLLISIONS))
				highlight_road_building_collisions(new_road_bounding_box, new_road_bounding_polygon);
			if (m_Scene->m_TreeManager.restrictions[0] && (restrictionFlags & (u8)RoadRestrictions::RESTRICT_COLLISIONS))
				highlight_road_tree_collisions(new_road_bounding_box, new_road_bounding_polygon);

			b_ConstructionRestricted |= angleIsRestricted;
			b_ConstructionRestricted |= lengthIsRestricted;
			b_ConstructionRestricted |= collisionIsRestricted;

			DrawStraightGuidelines(m_ConstructionPositions[0], m_ConstructionPositions[3], m_Elevationtypes[0], m_Elevationtypes[3]);
		}
	}
	void RoadManager::OnUpdate_QuadraticCurve(v3& prevLocation, f32 ts)
	{
		RoadType& type = m_Scene->MainApplication->road_types[m_Type];
		m_Scene->m_Terrain->enabled =
			(m_ConstructionPhase == 0 && m_Elevationtypes[0] >= 0) ||
			(m_ConstructionPhase == 1 && m_Elevationtypes[1] >= 0) ||
			(m_ConstructionPhase == 2 && m_Elevationtypes[3] >= 0);
		// HACK
		static f32 cooldown = 0.0f;
		if (cooldown <= 0.0f)
		{
			if (Input::IsKeyPressed(KeyCode::PageUp))
			{
				m_CurrentElevation += 0.1f;
				cooldown = 0.15f;
			}
			else if (Input::IsKeyPressed(KeyCode::PageDown))
			{
				m_CurrentElevation -= 0.1f;
				cooldown = 0.15f;
			}
			else if (Input::IsKeyPressed(KeyCode::Home))
			{
				m_CurrentElevation = 0.0f;
			}
		}
		else
		{
			cooldown -= ts;
		}
		s8 elevationValue = 0;
		if (m_CurrentElevation < -type.tunnel_height)
			elevationValue = -1;
		else if (m_CurrentElevation > type.bridge_height)
			elevationValue = 1;

		// Delete this when bridges are added
		m_CurrentElevation = std::min(0.0f, m_CurrentElevation);

		ResetGuideLines();

		if (m_ConstructionPhase == 0)
		{
			m_Elevationtypes[0] = elevationValue;
			m_Elevationtypes[1] = elevationValue;
			m_Elevationtypes[2] = elevationValue;
			m_Elevationtypes[3] = elevationValue;
			m_Elevations[0] = m_CurrentElevation;
			m_Elevations[1] = m_CurrentElevation;
			m_Elevations[2] = m_CurrentElevation;
			m_Elevations[3] = m_CurrentElevation;

			SnapToGrid(prevLocation);
			SnapToRoad(prevLocation, true);

			if (!b_ConstructionStartSnapped)
				prevLocation.z += m_CurrentElevation;
			m_ConstructionPositions[0] = prevLocation;
			m_ConstructionPositions[1] = prevLocation;
			m_ConstructionPositions[2] = prevLocation;
			m_ConstructionPositions[3] = prevLocation;

			m_GroundGuidelinesStart->SetTransform(prevLocation + v3{ 0.0f, 0.0f, 0.15f }, v3{ 0.0f, 0.0f, glm::radians(180.0f) });
			m_GroundGuidelinesEnd->SetTransform(prevLocation + v3{ 0.0f, 0.0f, 0.15f }, v3(0.0f));
			m_TunnelGuidelinesStart->SetTransform(prevLocation + v3{ 0.0f, 0.0f, 0.15f }, v3{ 0.0f, 0.0f, glm::radians(180.0f) });
			m_TunnelGuidelinesEnd->SetTransform(prevLocation + v3{ 0.0f, 0.0f, 0.15f }, v3(0.0f));

			m_GroundGuidelinesStart->enabled = elevationValue == 0;
			m_GroundGuidelinesEnd->enabled = elevationValue == 0;

			m_TunnelGuidelinesStart->enabled = elevationValue == -1;
			m_TunnelGuidelinesEnd->enabled = elevationValue == -1;
		}
		else if (m_ConstructionPhase == 1)
		{
			m_Elevationtypes[2] = elevationValue;
			m_Elevationtypes[1] = elevationValue;
			m_Elevationtypes[3] = elevationValue;
			m_Elevations[1] = m_CurrentElevation;
			m_Elevations[2] = m_CurrentElevation;
			m_Elevations[3] = m_CurrentElevation;

			b_ConstructionRestricted = false;
			SnapToGrid(prevLocation);

			prevLocation.z += m_CurrentElevation;
			m_ConstructionPositions[1] = prevLocation;
			m_ConstructionPositions[2] = prevLocation;
			m_ConstructionPositions[3] = prevLocation;

			// TODO: After angle snapping
			v2 dir = (v2)(prevLocation - m_ConstructionPositions[0]);
			bool angleIsRestricted = RestrictSmallAngles(dir, m_StartSnappedNode, m_StartSnappedSegment, m_StartSnappedT);

			v3 AB = m_ConstructionPositions[3] - m_ConstructionPositions[0];

			SnapToHeight({ 1, 2, 3 }, 0, AB);
			SnapToAngle(AB, m_StartSnappedNode, m_StartSnappedSegment, m_StartSnappedT);
			m_ConstructionPositions[1] = m_ConstructionPositions[0] + AB;
			m_ConstructionPositions[2] = m_ConstructionPositions[0] + AB;
			m_ConstructionPositions[3] = m_ConstructionPositions[0] + AB;

			v3 A = m_ConstructionPositions[0];
			v3 D = m_ConstructionPositions[3];
			v3 AD = D - A;
			AD.z = 0.0f;
			AD = type.road_width * 0.5f * glm::normalize(AD);

			if (!b_ConstructionStartSnapped) A -= AD;
			D += AD;

			AD = v3{ -AD.y , AD.x, 0.0f };

			v3 P1 = A + AD;
			v3 P2 = A - AD;
			v3 P3 = D + AD;
			v3 P4 = D - AD;

			std::vector<std::array<v3, 3>> new_road_bounding_polygon = {
					std::array<v3,3>{ P1, P2, P3},
					std::array<v3,3>{ P2, P3, P4}
			};
			f32 guideline_height = elevationValue == -1 ? type.tunnel_height : type.road_height;
			std::array<v3, 2> new_road_bounding_box{ /* Find Better Way*/
				v3{std::min({P1.x, P2.x, P3.x, P4.x}), std::min({P1.y, P2.y, P3.y, P4.y}), std::min({A.z, D.z}) },
				v3{std::max({P1.x, P2.x, P3.x, P4.x}), std::max({P1.y, P2.y, P3.y, P4.y}), std::max({A.z, D.z}) + guideline_height}
			};

			bool collisionIsRestricted = (restrictionFlags & (u8)RoadRestrictions::RESTRICT_COLLISIONS) ? check_road_road_collision(new_road_bounding_box, new_road_bounding_polygon) : false;

			if (m_Scene->m_BuildingManager.restrictions[0] && (restrictionFlags & (u8)RoadRestrictions::RESTRICT_COLLISIONS))
				highlight_road_building_collisions(new_road_bounding_box, new_road_bounding_polygon);
			if (m_Scene->m_TreeManager.restrictions[0] && (restrictionFlags & (u8)RoadRestrictions::RESTRICT_COLLISIONS))
				highlight_road_tree_collisions(new_road_bounding_box, new_road_bounding_polygon);

			b_ConstructionRestricted |= angleIsRestricted;
			b_ConstructionRestricted |= collisionIsRestricted;
			DrawStraightGuidelines(m_ConstructionPositions[0], m_ConstructionPositions[3], m_Elevationtypes[0], m_Elevationtypes[3]);
		}
		else if (m_ConstructionPhase == 2)
		{
			m_Elevationtypes[3] = elevationValue;
			m_Elevations[3] = m_CurrentElevation;

			b_ConstructionRestricted = false;
			SnapToGrid(prevLocation);
			SnapToRoad(prevLocation, false);

			if (!b_ConstructionEndSnapped)
				prevLocation.z += m_CurrentElevation;
			m_ConstructionPositions[3] = prevLocation;

			if (!b_ConstructionEndSnapped)
				SnapToHeight({ 3 }, 0, v3(0.0f));
			// SnapToAngle For center angle
			/***Magic***/ {
				v2 Cd = (v2)m_ConstructionPositions[1];
				v2 A = (v2)m_ConstructionPositions[0];
				v2 B = (v2)m_ConstructionPositions[3];
				v2 ray = glm::normalize(Cd - A);
				v2 AB = B - A;
				f32 d = glm::dot(AB, AB) / (2.0f * glm::dot(AB, ray));
				v2 C = A + d * ray;
				if (d < 200.0f && d > 0.0f)
				{
					m_ConstructionPositions[2].x = C.x;
					m_ConstructionPositions[2].y = C.y;
				}
			}
			// TODO: After angle snapping
			v2 dir1 = (v2)(m_ConstructionPositions[1] - m_ConstructionPositions[0]);
			bool angleIsRestricted = RestrictSmallAngles(dir1, m_StartSnappedNode, m_StartSnappedSegment, m_StartSnappedT);
			v2 dir2 = (v2)(m_ConstructionPositions[2] - m_ConstructionPositions[3]);
			angleIsRestricted |= RestrictSmallAngles(dir2, m_EndSnappedNode, m_EndSnappedSegment, m_EndSnappedT);
			if (restrictionFlags & (u8)RoadRestrictions::RESTRICT_SMALL_ANGLES)
			{
				v3 dirToEnd = m_ConstructionPositions[2] - m_ConstructionPositions[3];
				v3 dirToStart = m_ConstructionPositions[2] - m_ConstructionPositions[0];

				f32 dotResult = glm::dot(dirToEnd, dirToStart);
				dotResult /= glm::length(dirToEnd) * glm::length(dirToStart);
				dotResult = std::max(-1.0f, std::min(1.0f, dotResult));
				f32 angle = glm::acos(dotResult);

				angleIsRestricted |= angle < glm::radians(20.0f);
			}


			std::array<v3, 4> cps = {
					m_ConstructionPositions[0],
						(m_ConstructionPositions[2] + m_ConstructionPositions[0]) * 0.5f,
						(m_ConstructionPositions[2] + m_ConstructionPositions[3]) * 0.5f,
						m_ConstructionPositions[3],
			};

			std::array<v3, 2>  new_road_bounding_box = Math::get_bounding_box_from_cubic_bezier_curve(cps, type.road_width * 0.5f, type.road_height);
			std::vector<std::array<v3, 3>> new_road_bounding_polygon = Math::get_bounding_polygon_from_bezier_curve(cps, type.road_width * 0.5f, type.road_length);

			bool lengthIsRestricted = (restrictionFlags & (u8)RoadRestrictions::RESTRICT_SHORT_LENGTH) && (glm::length(cps[0] - cps[3]) < (2.0f * type.road_length));
			bool collisionIsRestricted = (restrictionFlags & (u8)RoadRestrictions::RESTRICT_COLLISIONS) ? check_road_road_collision(new_road_bounding_box, new_road_bounding_polygon) : false;

			if (m_Scene->m_BuildingManager.restrictions[0] && (restrictionFlags & (u8)RoadRestrictions::RESTRICT_COLLISIONS))
				highlight_road_building_collisions(new_road_bounding_box, new_road_bounding_polygon);
			if (m_Scene->m_TreeManager.restrictions[0] && (restrictionFlags & (u8)RoadRestrictions::RESTRICT_COLLISIONS))
				highlight_road_tree_collisions(new_road_bounding_box, new_road_bounding_polygon);

			b_ConstructionRestricted |= angleIsRestricted;
			b_ConstructionRestricted |= lengthIsRestricted;
			b_ConstructionRestricted |= collisionIsRestricted;
			DrawCurvedGuidelines(cps);
		}
	}
	void RoadManager::OnUpdate_CubicCurve(v3& prevLocation, f32 ts)
	{
		RoadType& type = m_Scene->MainApplication->road_types[m_Type];
		m_Scene->m_Terrain->enabled = m_Elevationtypes[cubicCurveOrder[m_ConstructionPhase]] >= 0;

		// HACK
		static f32 cooldown = 0.0f;
		if (cooldown <= 0.0f)
		{
			if (Input::IsKeyPressed(KeyCode::PageUp))
			{
				m_CurrentElevation += 0.1f;
				cooldown = 0.15f;
			}
			else if (Input::IsKeyPressed(KeyCode::PageDown))
			{
				m_CurrentElevation -= 0.1f;
				cooldown = 0.15f;
			}
			else if (Input::IsKeyPressed(KeyCode::Home))
			{
				m_CurrentElevation = 0.0f;
			}
		}
		else
		{
			cooldown -= ts;
		}

		s8 elevationValue = 0;
		if (m_CurrentElevation < -type.tunnel_height)
			elevationValue = -1;
		else if (m_CurrentElevation > type.bridge_height)
			elevationValue = 1;

		// Delete this when bridges are added
		m_CurrentElevation = std::min(0.0f, m_CurrentElevation);

		ResetGuideLines();

		if (m_ConstructionPhase == 0)
		{
			m_Elevationtypes[0] = elevationValue;
			m_Elevationtypes[1] = elevationValue;
			m_Elevationtypes[2] = elevationValue;
			m_Elevationtypes[3] = elevationValue;
			m_Elevations[0] = m_CurrentElevation;
			m_Elevations[1] = m_CurrentElevation;
			m_Elevations[2] = m_CurrentElevation;
			m_Elevations[3] = m_CurrentElevation;

			SnapToGrid(prevLocation);
			SnapToRoad(prevLocation, true);

			if (!b_ConstructionStartSnapped)
				prevLocation.z += m_CurrentElevation;
			m_ConstructionPositions[0] = prevLocation;

			m_GroundGuidelinesStart->SetTransform(prevLocation + v3{ 0.0f, 0.0f, 0.15f }, v3{ 0.0f, 0.0f, glm::radians(180.0f) });
			m_GroundGuidelinesEnd->SetTransform(prevLocation + v3{ 0.0f, 0.0f, 0.15f }, v3(0.0f));
			m_TunnelGuidelinesStart->SetTransform(prevLocation + v3{ 0.0f, 0.0f, 0.15f }, v3{ 0.0f, 0.0f, glm::radians(180.0f) });
			m_TunnelGuidelinesEnd->SetTransform(prevLocation + v3{ 0.0f, 0.0f, 0.15f }, v3(0.0f));

			m_GroundGuidelinesStart->enabled = elevationValue == 0;
			m_GroundGuidelinesEnd->enabled = elevationValue == 0;

			m_TunnelGuidelinesStart->enabled = elevationValue == -1;
			m_TunnelGuidelinesEnd->enabled = elevationValue == -1;
		}
		else if (m_ConstructionPhase == 1)
		{
			m_Elevationtypes[cubicCurveOrder[1]] = elevationValue;
			m_Elevations[cubicCurveOrder[1]] = m_CurrentElevation;

			b_ConstructionRestricted = false;
			SnapToGrid(prevLocation);
			if (cubicCurveOrder[1] == 3)
				SnapToRoad(prevLocation, false);

			if (!(b_ConstructionEndSnapped && cubicCurveOrder[1] == 3))
				prevLocation.z += m_CurrentElevation;
			m_ConstructionPositions[cubicCurveOrder[1]] = prevLocation;

			// TODO: After angle snapping
			bool angleIsRestricted = false;
			if (cubicCurveOrder[1] == 1)
			{
				v2 dir = (v2)(m_ConstructionPositions[1] - m_ConstructionPositions[0]);
				angleIsRestricted = RestrictSmallAngles(dir, m_StartSnappedNode, m_StartSnappedSegment, m_StartSnappedT);
			}

			v3 AB = prevLocation - m_ConstructionPositions[0];

			if ((snapFlags & (u8)RoadSnapOptions::SNAP_TO_LENGTH) && (cubicCurveOrder[1] == 1) && (glm::length(AB) > 0.5f))
			{
				f32 length = glm::length(AB);
				length = length - std::fmod(length, type.road_length);
				AB = length * glm::normalize(AB);

				m_ConstructionPositions[1] = m_ConstructionPositions[0] + AB;
			}
			if (!b_ConstructionEndSnapped)
				SnapToHeight({ cubicCurveOrder[1] }, 0, AB);
			if (cubicCurveOrder[1] == 1)
			{
				SnapToAngle(AB, m_StartSnappedNode, m_StartSnappedSegment, m_StartSnappedT);
				m_ConstructionPositions[1] = m_ConstructionPositions[0] + AB;
			}

			v3 A = m_ConstructionPositions[0];
			v3 D = m_ConstructionPositions[3];
			v3 AD = D - A;
			AD.z = 0.0f;
			AD = type.road_width * 0.5f * glm::normalize(AD);

			if (!b_ConstructionStartSnapped) A -= AD;
			D += AD;

			AD = v3{ -AD.y , AD.x, 0.0f };

			v3 P1 = A + AD;
			v3 P2 = A - AD;
			v3 P3 = D + AD;
			v3 P4 = D - AD;

			std::vector<std::array<v3, 3>> new_road_bounding_polygon = {
					std::array<v3,3>{ P1, P2, P3},
					std::array<v3,3>{ P2, P3, P4}
			};
			f32 guideline_height = elevationValue == -1 ? type.tunnel_height : type.road_height;
			std::array<v3, 2> new_road_bounding_box{ /* Find Better Way*/
				v3{std::min({P1.x, P2.x, P3.x, P4.x}), std::min({P1.y, P2.y, P3.y, P4.y}), std::min({A.z, D.z}) },
				v3{std::max({P1.x, P2.x, P3.x, P4.x}), std::max({P1.y, P2.y, P3.y, P4.y}), std::max({A.z, D.z}) + guideline_height}
			};

			bool lengthIsRestricted = (restrictionFlags & (u8)RoadRestrictions::RESTRICT_SHORT_LENGTH) && glm::length(AB) < 2.0f * type.road_length;
			bool collisionIsRestricted = (restrictionFlags & (u8)RoadRestrictions::RESTRICT_COLLISIONS) ? check_road_road_collision(new_road_bounding_box, new_road_bounding_polygon) : false;

			if (m_Scene->m_BuildingManager.restrictions[0] && (restrictionFlags & (u8)RoadRestrictions::RESTRICT_COLLISIONS))
				highlight_road_building_collisions(new_road_bounding_box, new_road_bounding_polygon);
			if (m_Scene->m_TreeManager.restrictions[0] && (restrictionFlags & (u8)RoadRestrictions::RESTRICT_COLLISIONS))
				highlight_road_tree_collisions(new_road_bounding_box, new_road_bounding_polygon);

			b_ConstructionRestricted |= angleIsRestricted;
			b_ConstructionRestricted |= lengthIsRestricted;
			b_ConstructionRestricted |= collisionIsRestricted;
			DrawStraightGuidelines(m_ConstructionPositions[0], m_ConstructionPositions[cubicCurveOrder[1]], m_Elevationtypes[0], m_Elevationtypes[1]);

		}
		else if (m_ConstructionPhase == 2)
		{
			m_Elevationtypes[cubicCurveOrder[2]] = elevationValue;
			m_Elevations[cubicCurveOrder[2]] = m_CurrentElevation;

			b_ConstructionRestricted = false;
			SnapToGrid(prevLocation);
			if (cubicCurveOrder[2] == 3)
				SnapToRoad(prevLocation, false);

			if (!(b_ConstructionEndSnapped && cubicCurveOrder[2] == 3))
				prevLocation.z += m_CurrentElevation;
			m_ConstructionPositions[cubicCurveOrder[2]] = prevLocation;
			m_ConstructionPositions[cubicCurveOrder[3]] = prevLocation;

			// TODO: After angle snapping
			bool angleIsRestricted = false;
			if (cubicCurveOrder[3] == 1)
			{
				v2 dir = (v2)(m_ConstructionPositions[2] - m_ConstructionPositions[3]);
				bool angleIsRestricted = RestrictSmallAngles(dir, m_EndSnappedNode, m_EndSnappedSegment, m_EndSnappedT);
			}
			else
			{
				v2 dir = (v2)(m_ConstructionPositions[1] - m_ConstructionPositions[0]);
				bool angleIsRestricted = RestrictSmallAngles(dir, m_StartSnappedNode, m_StartSnappedSegment, m_StartSnappedT);
			}
			if (snapFlags & (u8)RoadSnapOptions::SNAP_TO_LENGTH)
			{
				v3 AB1 = m_ConstructionPositions[1] - m_ConstructionPositions[0];
				v3 AB2 = m_ConstructionPositions[2] - m_ConstructionPositions[3];

				f32 length1 = glm::length(AB1);
				f32 length2 = glm::length(AB2);

				if (cubicCurveOrder[2] == 1 && length1 > 0.1f)
				{
					length1 = length1 - std::fmod(length1, type.road_length);
					AB1 = length1 * glm::normalize(AB1);

					m_ConstructionPositions[1] = m_ConstructionPositions[0] + AB1;
				}
				else if (cubicCurveOrder[2] == 2 && length2 > 0.1f)
				{
					length2 = length2 - std::fmod(length2, type.road_length);
					AB2 = length2 * glm::normalize(AB2);

					m_ConstructionPositions[2] = m_ConstructionPositions[3] + AB2;
				}
			}
			if (!b_ConstructionEndSnapped)
				SnapToHeight({ cubicCurveOrder[2] }, 0, v3(0.0f));
			if (cubicCurveOrder[2] == 1)
			{
				v3 AB = m_ConstructionPositions[1] - m_ConstructionPositions[0];
				SnapToAngle(AB, m_StartSnappedNode, m_StartSnappedSegment, m_StartSnappedT);
				m_ConstructionPositions[1] = m_ConstructionPositions[0] + AB;
			}
			else if (cubicCurveOrder[3] == 1 && cubicCurveOrder[2] == 2)
			{
				v3 AB = m_ConstructionPositions[2] - m_ConstructionPositions[3];
				SnapToAngle(AB, m_EndSnappedNode, m_EndSnappedSegment, m_EndSnappedT);
				m_ConstructionPositions[2] = m_ConstructionPositions[3] + AB;
			}

			// needs some attention
			std::array<v3, 4> cps{
				m_ConstructionPositions[0],
				(m_ConstructionPositions[(cubicCurveOrder[3] == 1) ? 2 : 1] + m_ConstructionPositions[0]) * 0.5f,
				(m_ConstructionPositions[(cubicCurveOrder[3] == 1) ? 2 : 1] + m_ConstructionPositions[3]) * 0.5f,
				m_ConstructionPositions[3]
			};
			f32 guideline_width = (elevationValue == -1 ? type.tunnel_width : type.road_width) * 0.5f;
			f32 guideline_height = elevationValue == -1 ? type.tunnel_height : type.road_height;
			f32 guideline_length = elevationValue == -1 ? type.tunnel_length : type.road_length;
			std::array<v3, 2> new_road_bounding_box = Math::get_bounding_box_from_cubic_bezier_curve(cps, guideline_width, guideline_height);
			std::vector<std::array<v3, 3>> new_road_bounding_polygon = Math::get_bounding_polygon_from_bezier_curve(cps, guideline_width, guideline_length);

			bool lengthIsRestricted = (restrictionFlags & (u8)RoadRestrictions::RESTRICT_SHORT_LENGTH) &&
				(((cubicCurveOrder[3] == 1) && (glm::length(cps[2] - cps[3]) < 2.0f * type.road_length)) ||
					((cubicCurveOrder[3] != 1) && (glm::length(cps[1] - cps[0]) < 2.0f * type.road_length)));
			bool collisionIsRestricted = (restrictionFlags & (u8)RoadRestrictions::RESTRICT_COLLISIONS) ? check_road_road_collision(new_road_bounding_box, new_road_bounding_polygon) : false;

			if (m_Scene->m_BuildingManager.restrictions[0] && (restrictionFlags & (u8)RoadRestrictions::RESTRICT_COLLISIONS))
				highlight_road_building_collisions(new_road_bounding_box, new_road_bounding_polygon);
			if (m_Scene->m_TreeManager.restrictions[0] && (restrictionFlags & (u8)RoadRestrictions::RESTRICT_COLLISIONS))
				highlight_road_tree_collisions(new_road_bounding_box, new_road_bounding_polygon);

			b_ConstructionRestricted |= angleIsRestricted;
			b_ConstructionRestricted |= lengthIsRestricted;
			b_ConstructionRestricted |= collisionIsRestricted;

			DrawCurvedGuidelines(cps);
		}
		else if (m_ConstructionPhase == 3)
		{
			m_Elevationtypes[cubicCurveOrder[3]] = elevationValue;
			m_Elevations[cubicCurveOrder[3]] = m_CurrentElevation;

			b_ConstructionRestricted = false;
			SnapToGrid(prevLocation);
			if (cubicCurveOrder[3] == 3)
				SnapToRoad(prevLocation, false);

			if (!(b_ConstructionEndSnapped && cubicCurveOrder[3] == 3))
				prevLocation.z += m_CurrentElevation;
			m_ConstructionPositions[cubicCurveOrder[3]] = prevLocation;

			// TODO: After angle snapping
			v2 dir1 = (v2)(m_ConstructionPositions[1] - m_ConstructionPositions[0]);
			bool angleIsRestricted = RestrictSmallAngles(dir1, m_StartSnappedNode, m_StartSnappedSegment, m_StartSnappedT);
			v2 dir2 = (v2)(m_ConstructionPositions[2] - m_ConstructionPositions[3]);
			angleIsRestricted |= RestrictSmallAngles(dir2, m_EndSnappedNode, m_EndSnappedSegment, m_EndSnappedT);

			if (snapFlags & (u8)RoadSnapOptions::SNAP_TO_LENGTH)
			{
				v3 AB1 = m_ConstructionPositions[1] - m_ConstructionPositions[0];
				v3 AB2 = m_ConstructionPositions[2] - m_ConstructionPositions[3];

				f32 length1 = glm::length(AB1);
				f32 length2 = glm::length(AB2);

				if (cubicCurveOrder[3] == 1 && length1 > 0.1f)
				{
					length1 = length1 - std::fmod(length1, type.road_length);
					AB1 = length1 * glm::normalize(AB1);

					m_ConstructionPositions[1] = m_ConstructionPositions[0] + AB1;
				}
				else if (cubicCurveOrder[3] == 2 && length2 > 0.1f)
				{
					length2 = length2 - std::fmod(length2, type.road_length);
					AB2 = length2 * glm::normalize(AB2);

					m_ConstructionPositions[2] = m_ConstructionPositions[3] + AB2;
				}
			}
			if (!b_ConstructionEndSnapped)
				SnapToHeight({ cubicCurveOrder[3] }, 0, v3(0.0f));

			if (cubicCurveOrder[3] == 1)
			{
				v3 AB = m_ConstructionPositions[1] - m_ConstructionPositions[0];
				SnapToAngle(AB, m_StartSnappedNode, m_StartSnappedSegment, m_StartSnappedT);
				m_ConstructionPositions[1] = m_ConstructionPositions[0] + AB;
			}
			else if (cubicCurveOrder[3] == 2)
			{
				v3 AB = m_ConstructionPositions[2] - m_ConstructionPositions[3];
				SnapToAngle(AB, m_EndSnappedNode, m_EndSnappedSegment, m_EndSnappedT);
				m_ConstructionPositions[2] = m_ConstructionPositions[3] + AB;
			}

			f32 guideline_width = (elevationValue == -1 ? type.tunnel_width : type.road_width) * 0.5f;
			f32 guideline_height = elevationValue == -1 ? type.tunnel_height : type.road_height;
			f32 guideline_length = elevationValue == -1 ? type.tunnel_length : type.road_length;
			std::array<v3, 2> new_road_bounding_box = Math::get_bounding_box_from_cubic_bezier_curve(m_ConstructionPositions, guideline_width, guideline_height);
			std::vector<std::array<v3, 3>> new_road_bounding_polygon = Math::get_bounding_polygon_from_bezier_curve(m_ConstructionPositions, guideline_width, guideline_length);

			bool lengthIsRestricted =
				(restrictionFlags & (u8)RoadRestrictions::RESTRICT_SHORT_LENGTH) &&
				(glm::length(m_ConstructionPositions[0] - m_ConstructionPositions[1]) < 2.0f * type.road_length) &&
				(glm::length(m_ConstructionPositions[3] - m_ConstructionPositions[2]) < 2.0f * type.road_length);
			bool collisionIsRestricted = (restrictionFlags & (u8)RoadRestrictions::RESTRICT_COLLISIONS) ? check_road_road_collision(new_road_bounding_box, new_road_bounding_polygon) : false;

			if (m_Scene->m_BuildingManager.restrictions[0] && (restrictionFlags & (u8)RoadRestrictions::RESTRICT_COLLISIONS))
				highlight_road_building_collisions(new_road_bounding_box, new_road_bounding_polygon);
			if (m_Scene->m_TreeManager.restrictions[0] && (restrictionFlags & (u8)RoadRestrictions::RESTRICT_COLLISIONS))
				highlight_road_tree_collisions(new_road_bounding_box, new_road_bounding_polygon);


			b_ConstructionRestricted |= angleIsRestricted;
			b_ConstructionRestricted |= lengthIsRestricted;
			b_ConstructionRestricted |= collisionIsRestricted;

			DrawCurvedGuidelines(m_ConstructionPositions);
		}
	}
	void RoadManager::OnUpdate_Change(v3& prevLocation, f32 ts)
	{
		GameApp* app = m_Scene->MainApplication;
		if (selected_road_segment != -1)
			m_Segments[selected_road_segment].object->SetTransform(m_Segments[selected_road_segment].GetStartPosition());

		v2 prevLoc2D = (v2)prevLocation;
		Prefab* roadType = app->road_types[m_Type].road;
		u64 size = m_Segments.size();
		for (u64 rsIndex = 0; rsIndex < size; rsIndex++)
		{
			RoadSegment& rs = m_Segments[rsIndex];
			RoadType& type = app->road_types[rs.type];

			f32 rsl = type.road_length;
			f32 snapDist = type.road_width * 0.5f;
			const std::array<v3, 4>& cps = rs.GetCurvePoints();
			std::array<std::array<v2, 3>, 2> rsBoundingBox = Math::GetBoundingBoxOfBezierCurve(cps, snapDist);
			bool colidedWithBoundingBox = Math::CheckPolygonPointCollision(rsBoundingBox, prevLoc2D);
			if (colidedWithBoundingBox)
			{
				std::vector<f32> ts{ 0.0f };
				const std::vector<v3>& curve_samples = rs.curve_samples;
				u64 curve_samples_size = curve_samples.size();
				CAN_ASSERT(curve_samples_size > 1, "Samples size can't be smaller than 2");
				v3 point0 = curve_samples[0];
				for (u64 i = 1; i < curve_samples_size; i++)
				{
					v3 point1 = curve_samples[i];
					v3 dirToP1 = point1 - point0;
					dirToP1.z = 0.0f;
					dirToP1 = glm::normalize(dirToP1);

					v3 dirToPrev = prevLocation - point0;
					f32 l1 = glm::length(dirToPrev);

					f32 angle = glm::acos(glm::dot(dirToP1, dirToPrev) / l1);
					f32 dist = l1 * glm::sin(angle);

					if (dist < snapDist)
					{
						f32 c = l1 * glm::cos(angle);
						if (c >= -0.5f * rsl && c <= 1.5f * rsl)
						{
							selected_road_segment = rsIndex;
							rs.object->SetTransform(rs.GetStartPosition() + v3{ 0.0f, 0.0f, 0.1f });
							return;
						}
					}
					point0 = point1;
				}
			}
		}
		selected_road_segment = -1;
	}
	void RoadManager::OnUpdate_Destruction(v3& prevLocation, f32 ts)
	{
		SnapInformation snapInformation = CheckSnapping(prevLocation);
		m_DestructionSegment = snapInformation.segment;
		m_DestructionNode = snapInformation.node;

		for (RoadSegment& rs : m_Segments)
			rs.object->SetTransform(rs.GetStartPosition());

		for (RoadNode& node : m_Nodes)
			node.object->SetTransform(node.position);


		if (snapInformation.snapped)
		{
			if (m_DestructionNode != -1)
			{
				for (u64 rsIndex : m_Nodes[m_DestructionNode].roadSegments)
				{
					RoadSegment& rs = m_Segments[rsIndex];
					RoadNode& startNode = m_Nodes[rs.StartNode];
					RoadNode& endNode = m_Nodes[rs.EndNode];

					rs.object->SetTransform(rs.GetStartPosition() + v3{ 0.0f, 0.0f, 0.1f });

					if (rs.StartNode == m_DestructionNode)
					{
						if (endNode.roadSegments.size() == 1)
							endNode.object->SetTransform(endNode.position + v3{ 0.0f, 0.0f, 0.1f });
					}
					else
					{
						if (startNode.roadSegments.size() == 1)
							startNode.object->SetTransform(startNode.position + v3{ 0.0f, 0.0f, 0.1f });
					}
				}
			}
			else if (m_DestructionSegment != -1)
			{
				RoadSegment& rs = m_Segments[m_DestructionSegment];
				RoadNode& startNode = m_Nodes[rs.StartNode];
				RoadNode& endNode = m_Nodes[rs.EndNode];

				rs.object->SetTransform(rs.GetStartPosition() + v3{ 0.0f, 0.0f, 0.1f });

				if (endNode.roadSegments.size() == 1)
					endNode.object->SetTransform(endNode.position + v3{ 0.0f, 0.0f, 0.1f });
				if (startNode.roadSegments.size() == 1)
					startNode.object->SetTransform(startNode.position + v3{ 0.0f, 0.0f, 0.1f });
			}
		}
	}

	void RoadManager::DrawStraightGuidelines(const v3& pointA, const v3& pointB, s8 eA, s8 eB)
	{
		RoadType& type = m_Scene->MainApplication->road_types[m_Type];

		v3 AB = pointB - pointA;
		f32 entire_length = glm::length(AB);
		if (entire_length < 0.1f)
			return;

		f32 first_length = 0.0f;

		v3 direction = AB / entire_length;
		v2 dir = glm::normalize((v2)AB);
		f32 yaw = glm::acos(dir.x) * ((float)(dir.y > 0.0f) * 2.0f - 1.0f);
		v3 dirR = glm::rotateZ(AB, -yaw);
		dir = glm::normalize(v2{ dirR.x, dirR.z });
		f32 pitch = glm::acos(std::abs(dir.x)) * ((float)(dir.y < 0.0f) * 2.0f - 1.0f);

		int count = (int)(entire_length / type.road_length);
		u64 countT = 0;
		u64 countG = 0;
		f32 scale = (entire_length / type.road_length) / count;
		f32 scaleT = 1.0f;
		f32 scaleG = 1.0f;

		if (eA != eB)
		{
			v3 intersectionPoint = Helper::RayPlaneIntersection(
				pointA,
				direction,
				v3{ 0.0f, 0.0f, -type.tunnel_height },
				v3{ 0.0f, 0.0f, 1.0f }
			);
			first_length = glm::length(intersectionPoint - pointA);
			f32 ratio = first_length / entire_length;
			if (eA == -1)
			{
				countT = (int)((entire_length * ratio) / type.tunnel_length);
				countG = (int)((entire_length * (1.0f - ratio)) / type.road_length);

				scaleT = (entire_length * ratio) / (countT * type.tunnel_length);
				scaleG = (entire_length * (1.0f - ratio)) / (countG * type.road_length);
			}
			else
			{
				countG = (int)((entire_length * ratio) / type.road_length);
				countT = (int)((entire_length * (1.0f - ratio)) / type.tunnel_length);

				scaleG = (entire_length * ratio) / (countG * type.road_length);
				scaleT = (entire_length * (1.0f - ratio)) / (countT * type.tunnel_length);
			}
			count = countT + countG;
		}
		else if (eA == 0)
		{
			scaleG = scale;
			countG = count;
			first_length = entire_length;
		}
		else if (eA == -1)
		{
			scaleT = scale;
			countT = count;
		}

		bool lengthIsRestricted = (restrictionFlags & (u8)RoadRestrictions::RESTRICT_SHORT_LENGTH) && count < 1;


		m_GroundGuidelinesInUse[m_Type] += countG;
		m_TunnelGuidelinesInUse[m_Type] += countT;

		if (m_GroundGuidelinesInUse[m_Type] > m_GroundGuidelines[m_Type].size())
			for (u64 j = m_GroundGuidelines[m_Type].size(); j < m_GroundGuidelinesInUse[m_Type]; j++)
				m_GroundGuidelines[m_Type].push_back(new Object(m_Scene->MainApplication->road_types[m_Type].road));

		if (m_TunnelGuidelinesInUse[m_Type] > m_TunnelGuidelines[m_Type].size())
			for (u64 j = m_TunnelGuidelines[m_Type].size(); j < m_TunnelGuidelinesInUse[m_Type]; j++)
				m_TunnelGuidelines[m_Type].push_back(new Object(m_Scene->MainApplication->road_types[m_Type].tunnel));

		v3 pointStart = pointA;
		v3 pointEnd = pointB;
		if (eA == -1 && countG > 0)
		{
			direction *= -1;
			first_length = entire_length - first_length;
			pitch *= -1;
			yaw += glm::radians(180.0f);
			pointStart = pointB;
			pointEnd = pointA;
		}

		for (u64 j = 0; j < countG; j++)
		{
			Object* roadG = m_GroundGuidelines[m_Type][j];
			roadG->enabled = true;
			roadG->SetTransform(
				pointStart + (direction * (j * scaleG * type.road_length)) + v3{ 0.0f, 0.0f, 0.15f },
				v3{ 0.0f, pitch, yaw },
				v3{ 1.0f * scaleG, 1.0f, 1.0f }
			);
		}
		for (u64 j = 0; j < countT; j++)
		{
			Object* roadG = m_TunnelGuidelines[m_Type][j];
			roadG->enabled = true;
			roadG->SetTransform(
				pointStart + (direction * (first_length + j * scaleT * type.tunnel_length)) + v3{ 0.0f, 0.0f, 0.15f },
				v3{ 0.0f, pitch, yaw },
				v3{ 1.0f * scaleT, 1.0f, 1.0f }
			);
		}

		b_ConstructionRestricted |= lengthIsRestricted;

		m_GroundGuidelinesStart->enabled = !b_ConstructionStartSnapped && eA == 0;
		m_GroundGuidelinesEnd->enabled = !b_ConstructionEndSnapped && eB == 0;
		m_TunnelGuidelinesStart->enabled = !b_ConstructionStartSnapped && eA == -1;
		m_TunnelGuidelinesEnd->enabled = !b_ConstructionEndSnapped && eB == -1;

		m_GroundGuidelinesStart->SetTransform(pointStart + v3{ 0.0f, 0.0f, 0.15f }, { 0.0f, -pitch, yaw + glm::radians(180.0f) });
		m_GroundGuidelinesEnd->SetTransform(pointEnd + v3{ 0.0f, 0.0f, 0.15f }, { 0.0f, pitch, yaw });
		m_TunnelGuidelinesStart->SetTransform(pointStart + v3{ 0.0f, 0.0f, 0.15f }, { 0.0f, -pitch, yaw + glm::radians(180.0f) });
		m_TunnelGuidelinesEnd->SetTransform(pointEnd + v3{ 0.0f, 0.0f, 0.15f }, { 0.0f, pitch, yaw });

		v4 red = v4{ 1.0f, 0.3f, 0.2f, 1.0f };
		v4 white = v4(1.0f);
		m_GroundGuidelinesStart->tintColor = b_ConstructionRestricted ? red : white;
		m_GroundGuidelinesEnd->tintColor = b_ConstructionRestricted ? red : white;
		m_TunnelGuidelinesStart->tintColor = b_ConstructionRestricted ? red : white;
		m_TunnelGuidelinesEnd->tintColor = b_ConstructionRestricted ? red : white;

		for (std::vector<Object*>& os : m_GroundGuidelines)
			for (Object* rg : os)
				rg->tintColor = b_ConstructionRestricted ? v4{ 1.0f, 0.3f, 0.2f, 1.0f } : v4(1.0f);
		for (std::vector<Object*>& os : m_TunnelGuidelines)
			for (Object* rg : os)
				rg->tintColor = b_ConstructionRestricted ? v4{ 1.0f, 0.3f, 0.2f, 1.0f } : v4(1.0f);

	}
	void RoadManager::DrawCurvedGuidelines(const std::array<v3, 4>& curvePoints)
	{
		RoadType& type = m_Scene->MainApplication->road_types[m_Type];

		f32 l = glm::length(curvePoints[3] - curvePoints[0]);
		u64 count = 1;
		{
			while (l > type.road_length)
			{
				count *= 2;
				v3 p = Math::CubicCurve<f32>(curvePoints, 1.0f / count);
				l = glm::length(p - curvePoints[0]);
			}
			if (count > 1) count /= 2;
			while (l > type.road_length)
			{
				count++;
				v3 p = Math::CubicCurve<f32>(curvePoints, 1.0f / count);
				l = glm::length(p - curvePoints[0]);
			}
			if (count > 1) count--;
		}

		v3 AB1 = curvePoints[1] - curvePoints[0];
		v3 AB2 = curvePoints[2] - curvePoints[3];

		f32 rotationOffset1 = (f32)(AB1.x >= 0.0f) * glm::radians(180.0f);
		f32 rotationOffset2 = (f32)(AB2.x >= 0.0f) * glm::radians(180.0f);

		f32 rotationStart = glm::atan(-AB1.y / AB1.x) + rotationOffset1;
		f32 rotationEnd = glm::atan(-AB2.y / AB2.x) + rotationOffset2;

		m_GroundGuidelinesStart->enabled = !b_ConstructionStartSnapped && m_Elevationtypes[0] == 0;
		m_GroundGuidelinesEnd->enabled = !b_ConstructionEndSnapped && m_Elevationtypes[3] == 0;
		m_TunnelGuidelinesStart->enabled = !b_ConstructionStartSnapped && m_Elevationtypes[0] == -1;
		m_TunnelGuidelinesEnd->enabled = !b_ConstructionEndSnapped && m_Elevationtypes[3] == -1;

		m_GroundGuidelinesStart->SetTransform(curvePoints[0] + v3{ 0.0f, 0.0f, 0.15f }, v3{ 0.0f, 0.0f, -rotationStart });
		m_GroundGuidelinesEnd->SetTransform(curvePoints[3] + v3{ 0.0f, 0.0f, 0.15f }, v3{ 0.0f, 0.0f, -rotationEnd });
		m_TunnelGuidelinesStart->SetTransform(curvePoints[0] + v3{ 0.0f, 0.0f, 0.15f }, v3{ 0.0f, 0.0f, -rotationStart });
		m_TunnelGuidelinesEnd->SetTransform(curvePoints[3] + v3{ 0.0f, 0.0f, 0.15f }, v3{ 0.0f, 0.0f, -rotationEnd });

		m_GroundGuidelinesStart->tintColor = b_ConstructionRestricted ? v4{ 1.0f, 0.3f, 0.2f, 1.0f } : v4(1.0f);
		m_GroundGuidelinesEnd->tintColor = b_ConstructionRestricted ? v4{ 1.0f, 0.3f, 0.2f, 1.0f } : v4(1.0f);
		m_TunnelGuidelinesStart->tintColor = b_ConstructionRestricted ? v4{ 1.0f, 0.3f, 0.2f, 1.0f } : v4(1.0f);
		m_TunnelGuidelinesEnd->tintColor = b_ConstructionRestricted ? v4{ 1.0f, 0.3f, 0.2f, 1.0f } : v4(1.0f);

		for (u64& inUse : m_GroundGuidelinesInUse)
			inUse = 0;
		for (u64& inUse : m_TunnelGuidelinesInUse)
			inUse = 0;

		v3 p1 = curvePoints[0];
		u64 groundIndex = 0;
		u64 tunnelIndex = 0;
		for (u64 c = 0; c < count; c++)
		{
			v3 p2 = Math::CubicCurve<f32>(std::array<v3, 4>{curvePoints}, (c + 1.0f) / count);
			v3 vec1 = p2 - p1;
			f32 length = glm::length(vec1);
			v3 dir1 = vec1 / length;

			f32 scale = 1.0f; ;
			f32 rot1 = glm::acos(dir1.x) * ((f32)(dir1.y > 0.0f) * 2.0f - 1.0f);

			Object* guideline_object = nullptr;
			if (-p1.y > type.tunnel_height)
			{
				m_TunnelGuidelinesInUse[m_Type] ++;
				if (m_TunnelGuidelinesInUse[m_Type] > m_TunnelGuidelines[m_Type].size())
					m_TunnelGuidelines[m_Type].push_back(new Object(m_Scene->MainApplication->road_types[m_Type].tunnel));
				guideline_object = m_TunnelGuidelines[m_Type][tunnelIndex];
				scale = length / type.tunnel_length;
				tunnelIndex++;
			}
			else
			{
				m_GroundGuidelinesInUse[m_Type] ++;
				if (m_GroundGuidelinesInUse[m_Type] > m_GroundGuidelines[m_Type].size())
					m_GroundGuidelines[m_Type].push_back(new Object(m_Scene->MainApplication->road_types[m_Type].road));
				guideline_object = m_GroundGuidelines[m_Type][groundIndex];
				scale = length / type.road_length;
				groundIndex++;
			}

			guideline_object->enabled = true;
			guideline_object->SetTransform(
				p1 + v3{ 0.0f, 0.0f, 0.15f },
				v3{ 0.0f, 0.0f, rot1 },
				v3{ scale, 1.0f, 1.0f }
			);

			p1 = p2;
		}


		for (std::vector<Object*>& os : m_GroundGuidelines)
			for (Object* rg : os)
				rg->tintColor = b_ConstructionRestricted ? v4{ 1.0f, 0.3f, 0.2f, 1.0f } : v4(1.0f);
		for (std::vector<Object*>& os : m_TunnelGuidelines)
			for (Object* rg : os)
				rg->tintColor = b_ConstructionRestricted ? v4{ 1.0f, 0.3f, 0.2f, 1.0f } : v4(1.0f);
	}

	bool RoadManager::check_road_road_collision(const std::array<v3, 2>& bounding_box, const std::vector<std::array<v3, 3>>& bounding_polygon)
	{
		GameApp* app = m_Scene->MainApplication;
		RoadType& type = m_Scene->MainApplication->road_types[m_Type];
		f32 guideline_height = m_Elevationtypes[0] == -1 ? type.tunnel_height : type.road_height;

		u64 segmentCount = m_Segments.size();
		for (u64 rsIndex = 0; rsIndex < segmentCount; rsIndex++)
		{
			RoadSegment& rs = m_Segments[rsIndex];
			RoadType& type = app->road_types[rs.type];
			if (rsIndex == m_StartSnappedSegment)
				continue;
			if (rsIndex == m_EndSnappedSegment)
				continue;
			if (m_StartSnappedNode == rs.StartNode || m_StartSnappedNode == rs.EndNode)
				continue;
			if (m_EndSnappedNode == rs.StartNode || m_EndSnappedNode == rs.EndNode)
				continue;
			f32 rs_height = rs.elevation_type == -1 ? type.tunnel_height : type.road_height;

			std::array<v3, 2> old_road_bounding_box{
				rs.object->prefab->boundingBoxL + rs.CurvePoints[0],
				rs.object->prefab->boundingBoxM + rs.CurvePoints[0]
			};

			if (Math::check_bounding_box_bounding_box_collision(bounding_box, old_road_bounding_box))
				if (Math::check_bounding_polygon_bounding_polygon_collision_with_z(bounding_polygon, guideline_height, rs.bounding_polygon, rs_height))
					return true;
		}
		return false;
	}
	bool RoadManager::check_road_building_collision(Building* building, const std::array<v3, 2>& bounding_box, const std::vector<std::array<v3, 3>>& bounding_polygon)
	{
		RoadType& type = m_Scene->MainApplication->road_types[m_Type];
		f32 height = m_Elevationtypes[0] == -1 ? type.tunnel_height : type.road_height;

		Prefab* prefab = building->object->prefab;
		f32 rot = building->object->rotation.z;
		f32 building_height = prefab->boundingBoxM.z - prefab->boundingBoxL.z;

		v3 A = v3{ prefab->boundingBoxL.x, prefab->boundingBoxL.y, prefab->boundingBoxL.z };
		v3 B = v3{ prefab->boundingBoxL.x, prefab->boundingBoxM.y, prefab->boundingBoxL.z };
		v3 C = v3{ prefab->boundingBoxM.x, prefab->boundingBoxL.y, prefab->boundingBoxL.z };
		v3 D = v3{ prefab->boundingBoxM.x, prefab->boundingBoxM.y, prefab->boundingBoxL.z };

		A = glm::rotateZ(A, rot) + building->position;
		B = glm::rotateZ(B, rot) + building->position;
		C = glm::rotateZ(C, rot) + building->position;
		D = glm::rotateZ(D, rot) + building->position;

		std::vector<std::array<v3, 3>> building_bounding_polygon{
			std::array<v3,3>{A, B, D},
			std::array<v3,3>{A, C, D}
		};

		std::array<v3, 2> building_bounding_box{
			v3{std::min({A.x, B.x, C.x, D.x}), std::min({A.y, B.y, C.y, D.y}), std::min({A.z, B.z, C.z, D.z})},
			v3{std::max({A.x, B.x, C.x, D.x}), std::max({A.y, B.y, C.y, D.y}), std::max({A.z, B.z, C.z, D.z})}
		};

		building->object->tintColor = v4(1.0f);
		if (Math::check_bounding_box_bounding_box_collision(bounding_box, building_bounding_box))
			if (Math::check_bounding_polygon_bounding_polygon_collision_with_z(bounding_polygon, height, building_bounding_polygon, building_height))
				return true;
		return false;
	}
	bool RoadManager::check_road_tree_collision(Object* tree, const std::array<v3, 2>& bounding_box, const std::vector<std::array<v3, 3>>& bounding_polygon)
	{
		RoadType& type = m_Scene->MainApplication->road_types[m_Type];
		f32 height = m_Elevationtypes[0] == -1 ? type.tunnel_height : type.road_height;

		Prefab* prefab = tree->prefab;
		f32 rot = tree->rotation.z;
		f32 tree_height = prefab->boundingBoxM.z - prefab->boundingBoxL.z;

		v3 A = v3{ prefab->boundingBoxL.x, prefab->boundingBoxL.y, prefab->boundingBoxL.z };
		v3 B = v3{ prefab->boundingBoxL.x, prefab->boundingBoxM.y, prefab->boundingBoxL.z };
		v3 C = v3{ prefab->boundingBoxM.x, prefab->boundingBoxL.y, prefab->boundingBoxL.z };
		v3 D = v3{ prefab->boundingBoxM.x, prefab->boundingBoxM.y, prefab->boundingBoxL.z };

		A = glm::rotateZ(A * tree->scale, rot) + tree->position;
		B = glm::rotateZ(B * tree->scale, rot) + tree->position;
		C = glm::rotateZ(C * tree->scale, rot) + tree->position;
		D = glm::rotateZ(D * tree->scale, rot) + tree->position;

		std::vector<std::array<v3, 3>> tree_bounding_polygon{
			std::array<v3,3>{A, B, D},
			std::array<v3,3>{A, C, D}
		};

		std::array<v3, 2>tree_bounding_box{
			v3{std::min({A.x, B.x, C.x, D.x}), std::min({A.y, B.y, C.y, D.y}), std::min({A.z, B.z, C.z, D.z})},
			v3{std::max({A.x, B.x, C.x, D.x}), std::max({A.y, B.y, C.y, D.y}), std::max({A.z, B.z, C.z, D.z})}
		};

		tree->tintColor = v4(1.0f);
		if (Math::check_bounding_box_bounding_box_collision(bounding_box, tree_bounding_box))
			if (Math::check_bounding_polygon_bounding_polygon_collision_with_z(bounding_polygon, height, tree_bounding_polygon, tree_height))
				return true;
		return false;
	}

	void RoadManager::highlight_road_building_collisions(const std::array<v3, 2>& bounding_box, const std::vector<std::array<v3, 3>>& bounding_polygon)
	{
		for (Building* building : m_Scene->m_BuildingManager.GetBuildings())
			if (check_road_building_collision(building, bounding_box, bounding_polygon))
				building->object->tintColor = v4{ 1.0f, 0.3f, 0.2f, 1.0f };
	}
	void RoadManager::highlight_road_tree_collisions(const std::array<v3, 2>& bounding_box, const std::vector<std::array<v3, 3>>& bounding_polygon)
	{
		for (Tree* tree : m_Scene->m_TreeManager.GetTrees())
			if (check_road_tree_collision(tree->object, bounding_box, bounding_polygon))
				tree->object->tintColor = v4{ 1.0f, 0.3f, 0.2f, 1.0f };
	}

	bool RoadManager::OnMousePressed(MouseCode button)
	{
		switch (m_ConstructionMode)
		{
		case RoadConstructionMode::None:
			break;
		case RoadConstructionMode::Straight:
			if (button == MouseCode::Button1)
			{
				ResetStates();
				m_GroundGuidelinesStart->enabled = true;
				m_GroundGuidelinesEnd->enabled = true;
				return false;
			}
			else if (button != MouseCode::Button0)
				return false;
			OnMousePressed_Straight();
			break;
		case RoadConstructionMode::QuadraticCurve:
			if (button == MouseCode::Button1)
			{
				ResetStates();
				m_GroundGuidelinesStart->enabled = true;
				m_GroundGuidelinesEnd->enabled = true;
				return false;
			}
			if (button != MouseCode::Button0)
				return false;
			OnMousePressed_QuadraticCurve();
			break;
		case RoadConstructionMode::CubicCurve:
			if (button == MouseCode::Button1)
			{
				ResetStates();
				m_GroundGuidelinesStart->enabled = true;
				m_GroundGuidelinesEnd->enabled = true;
				return false;
			}
			if (button != MouseCode::Button0)
				return false;
			OnMousePressed_CubicCurve();
			break;
		case RoadConstructionMode::Change:
			if (button != MouseCode::Button0)
				return false;
			OnMousePressed_Change();
			break;
		case RoadConstructionMode::Destruct:
			if (button != MouseCode::Button0)
				return false;
			OnMousePressed_Destruction();
			break;
		}
		return false;
	}
	bool RoadManager::OnMousePressed_Straight()
	{
		if (b_ConstructionRestricted)
			return false;
		m_ConstructionPhase++;

		if (m_ConstructionPhase > 1)
		{
			if (m_Elevationtypes[0] == m_Elevationtypes[3])
			{
				AddRoadSegment({
						m_ConstructionPositions[0],
						(3.0f * m_ConstructionPositions[0] + m_ConstructionPositions[3]) * 0.25f,
						(3.0f * m_ConstructionPositions[3] + m_ConstructionPositions[0]) * 0.25f,
						m_ConstructionPositions[3],
					}, m_Elevationtypes[0]);
			}
			else
			{
				RoadType& type = m_Scene->MainApplication->road_types[m_Type];

				v3 intersectionPoint = Helper::RayPlaneIntersection(
					m_ConstructionPositions[0],
					glm::normalize(m_ConstructionPositions[3] - m_ConstructionPositions[0]),
					v3{ 0.0f, 0.0f, -type.tunnel_height },
					v3{ 0.0f, 0.0f, 1.0f }
				);

				///////////
				m_Nodes.push_back(RoadNode({}, intersectionPoint, -1));
				u64 nodeIndex = m_Nodes.size() - 1;
				m_Nodes[nodeIndex].index = nodeIndex;
				///////////

				s64 snapNode = m_EndSnappedNode;
				s64 snapSegment = m_EndSnappedSegment;
				bool snapped = b_ConstructionEndSnapped;
				m_EndSnappedNode = nodeIndex;
				m_EndSnappedSegment = -1;
				b_ConstructionEndSnapped = true;
				AddRoadSegment({
						m_ConstructionPositions[0],
						(3.0f * m_ConstructionPositions[0] + intersectionPoint) * 0.25f,
						(3.0f * intersectionPoint + m_ConstructionPositions[0]) * 0.25f,
						intersectionPoint,
					}, m_Elevationtypes[0]);
				m_EndSnappedNode = snapNode;
				m_EndSnappedSegment = snapSegment;
				b_ConstructionEndSnapped = snapped;

				snapNode = m_StartSnappedNode;
				snapSegment = m_StartSnappedSegment;
				snapped = b_ConstructionStartSnapped;
				m_StartSnappedNode = nodeIndex;
				m_StartSnappedSegment = -1;
				b_ConstructionStartSnapped = true;
				AddRoadSegment({
						intersectionPoint,
						(3.0f * intersectionPoint + m_ConstructionPositions[3]) * 0.25f,
						(3.0f * m_ConstructionPositions[3] + intersectionPoint) * 0.25f,
						m_ConstructionPositions[3],
					}, m_Elevationtypes[3]);
				m_StartSnappedNode = snapNode;
				m_StartSnappedSegment = snapSegment;
				b_ConstructionStartSnapped = snapped;
			}


			ResetStates();
			m_Scene->m_TreeManager.ResetStates();
			m_Scene->m_BuildingManager.ResetStates();
		}

		return false;
	}
	bool RoadManager::OnMousePressed_QuadraticCurve()
	{
		if (b_ConstructionRestricted)
			return false;
		m_ConstructionPhase++;

		if (m_ConstructionPhase > 2)
		{
			std::array<v3, 4> cps{
					m_ConstructionPositions[0],
					(m_ConstructionPositions[2] + m_ConstructionPositions[0]) * 0.5f,
					(m_ConstructionPositions[2] + m_ConstructionPositions[3]) * 0.5f,
					m_ConstructionPositions[3],
			};

			if (
				m_Elevationtypes[0] == m_Elevationtypes[1] &&
				m_Elevationtypes[0] == m_Elevationtypes[2] &&
				m_Elevationtypes[0] == m_Elevationtypes[3])
			{
				AddRoadSegment(cps, m_Elevationtypes[0]);
			}
			else
			{
				RoadType& type = m_Scene->MainApplication->road_types[m_Type];
				std::vector<f32> ts{ 0.0f };
				std::vector<v3> samples = Math::GetCubicCurveSamples(cps, type.road_length, ts);

				bool is_underground = m_Elevationtypes[0] == -1;
				u64 count = ts.size();

				s64 end_snap_node = m_EndSnappedNode;
				s64 end_snap_segment = m_EndSnappedSegment;
				bool end_snapped = b_ConstructionEndSnapped;

				for (u64 i = 0; i < count - 1; i++)
				{
					if ((is_underground && -samples[i].y < type.tunnel_height) || (!is_underground && -samples[i].y > type.tunnel_height))
					{
						m_Nodes.push_back(RoadNode({}, samples[i], -1));
						u64 nodeIndex = m_Nodes.size() - 1;
						m_Nodes[nodeIndex].index = nodeIndex;

						m_EndSnappedNode = nodeIndex;
						m_EndSnappedSegment = -1;
						b_ConstructionEndSnapped = true;

						AddRoadSegment({
							cps[0],
							cps[0] + (cps[1] - cps[0]) * ts[i],
							Math::QuadraticCurve(std::array<v3, 3>{ cps[0], cps[1], cps[2] }, ts[i]),
							samples[i]
							}, is_underground ? -1 : 0);

						cps = {
							samples[i],
							Math::QuadraticCurve(std::array<v3, 3>{ cps[1], cps[2], cps[3] }, ts[i]),
							cps[2] + (cps[3] - cps[2]) * ts[i],
							cps[3]
						};
						m_StartSnappedNode = m_EndSnappedNode;
						m_StartSnappedSegment = m_EndSnappedSegment;
						b_ConstructionStartSnapped = b_ConstructionEndSnapped;

						ts = { 0.0f };
						samples = Math::GetCubicCurveSamples(cps, type.road_length, ts);
						i = 0;
						count = ts.size();
						is_underground = !is_underground;
					}
				}

				m_EndSnappedNode = end_snap_node;
				m_EndSnappedSegment = end_snap_segment;
				b_ConstructionEndSnapped = end_snapped;

				AddRoadSegment(cps, is_underground ? -1 : 0);
			}

			ResetStates();
			m_Scene->m_TreeManager.ResetStates();
			m_Scene->m_BuildingManager.ResetStates();
		}
		return false;
	}
	bool RoadManager::OnMousePressed_CubicCurve()
	{
		if (b_ConstructionRestricted)
			return false;
		m_ConstructionPhase++;

		if (m_ConstructionPhase > 3)
		{
			std::array<v3, 4> cps = m_ConstructionPositions;

			if (
				m_Elevationtypes[0] == m_Elevationtypes[1] &&
				m_Elevationtypes[0] == m_Elevationtypes[2] &&
				m_Elevationtypes[0] == m_Elevationtypes[3])
			{
				AddRoadSegment(cps, m_Elevationtypes[0]);
			}
			else
			{
				RoadType& type = m_Scene->MainApplication->road_types[m_Type];
				std::vector<f32> ts{ 0.0f };
				std::vector<v3> samples = Math::GetCubicCurveSamples(cps, type.road_length, ts);

				bool is_underground = m_Elevationtypes[0] == -1;
				u64 count = ts.size();

				s64 end_snap_node = m_EndSnappedNode;
				s64 end_snap_segment = m_EndSnappedSegment;
				bool end_snapped = b_ConstructionEndSnapped;

				for (u64 i = 0; i < count - 1; i++)
				{
					if ((is_underground && -samples[i].y < type.tunnel_height) || (!is_underground && -samples[i].y > type.tunnel_height))
					{
						m_Nodes.push_back(RoadNode({}, samples[i], -1));
						u64 nodeIndex = m_Nodes.size() - 1;
						m_Nodes[nodeIndex].index = nodeIndex;

						m_EndSnappedNode = nodeIndex;
						m_EndSnappedSegment = -1;
						b_ConstructionEndSnapped = true;

						AddRoadSegment({
							cps[0],
							cps[0] + (cps[1] - cps[0]) * ts[i],
							Math::QuadraticCurve(std::array<v3, 3>{ cps[0], cps[1], cps[2] }, ts[i]),
							samples[i]
							}, is_underground ? -1 : 0);

						cps = {
							samples[i],
							Math::QuadraticCurve(std::array<v3, 3>{ cps[1], cps[2], cps[3] }, ts[i]),
							cps[2] + (cps[3] - cps[2]) * ts[i],
							cps[3]
						};
						m_StartSnappedNode = m_EndSnappedNode;
						m_StartSnappedSegment = m_EndSnappedSegment;
						b_ConstructionStartSnapped = b_ConstructionEndSnapped;

						ts = { 0.0f };
						samples = Math::GetCubicCurveSamples(cps, type.road_length, ts);
						i = 0;
						count = ts.size();
						is_underground = !is_underground;
					}
				}

				m_EndSnappedNode = end_snap_node;
				m_EndSnappedSegment = end_snap_segment;
				b_ConstructionEndSnapped = end_snapped;

				AddRoadSegment(cps, is_underground ? -1 : 0);
			}

			ResetStates();
			m_Scene->m_TreeManager.ResetStates();
			m_Scene->m_BuildingManager.ResetStates();
		}
		return false;
	}
	bool RoadManager::OnMousePressed_Change()
	{
		GameApp* app = m_Scene->MainApplication;
		if (selected_road_segment == -1)
			return false;
		RoadSegment& rs = m_Segments[selected_road_segment];
		if (rs.elevation_type == 0)
			Helper::UpdateTheTerrain(&rs, true);

		auto& currentType = app->road_types[rs.type];
		auto& newType = m_Scene->MainApplication->road_types[m_Type];
		if (newType.road == currentType.road)
		{
			if (!currentType.asymmetric)
				return false;

			auto& cps = rs.GetCurvePoints();
			u64 temp = rs.StartNode;
			rs.StartNode = rs.EndNode;
			rs.EndNode = temp;
			rs.SetCurvePoints({ cps[3], cps[2], cps[1], cps[0] });

			for (Building* building : rs.Buildings)
			{
				building->snapped_t = 1.0f - building->snapped_t;
				building->snapped_t_index = (rs.curve_samples.size() - 1) - building->snapped_t_index;
			}

			for (Car* car : rs.Cars)
			{
				car->fromStart = !car->fromStart;
				car->t_index = rs.curve_samples.size() - car->t_index;
				u64 nextIndex = car->t_index + (car->fromStart ? +1 : -1);
				nextIndex = std::max((const u64)0U, std::min(nextIndex, rs.curve_samples.size() - 1U));
				car->target = rs.curve_samples[car->t_index];
			}
		}
		else
		{
			rs.SetType(m_Type);
		}

		m_Nodes[rs.StartNode].Reconstruct();
		m_Nodes[rs.EndNode].Reconstruct();

		return false;
	}
	bool RoadManager::OnMousePressed_Destruction()
	{
		if (m_DestructionNode != -1)
		{
			RoadNode* node = &m_Nodes[m_DestructionNode];
			for (u64 i = node->roadSegments.size(); i > 0; i--) {
				u8 amount_update = RemoveRoadSegment(node->roadSegments[i - 1], m_DestructionNode);
				if (amount_update > 0) {
					m_DestructionNode -= amount_update;
					if (m_DestructionNode >= 0)
						node = &m_Nodes[m_DestructionNode];
				}
			}
		}
		else if (m_DestructionSegment != -1)
		{
			RemoveRoadSegment(m_DestructionSegment, 0);
		}
		return false;
	}

	void RoadManager::SetType(u64 type)
	{
		if (m_Type == type) return;
		m_Type = type;
		delete m_GroundGuidelinesEnd;
		delete m_GroundGuidelinesStart;
		delete m_TunnelGuidelinesEnd;
		delete m_TunnelGuidelinesStart;

		const RoadType& t = m_Scene->MainApplication->road_types[m_Type];
		m_GroundGuidelinesStart = new Object(t.asymmetric ? t.road_end_mirror : t.road_end);
		m_GroundGuidelinesEnd = new Object(t.road_end);
		m_TunnelGuidelinesStart = new Object(t.asymmetric ? t.tunnel_end_mirror : t.tunnel_end);
		m_TunnelGuidelinesEnd = new Object(t.tunnel_end);
	}
	void RoadManager::SetConstructionMode(RoadConstructionMode mode)
	{
		ResetStates();
		m_ConstructionMode = mode;

		switch (m_ConstructionMode)
		{
		case RoadConstructionMode::None:
			break;
		case RoadConstructionMode::Straight:
		case RoadConstructionMode::QuadraticCurve:
		case RoadConstructionMode::CubicCurve:
			m_GroundGuidelinesStart->enabled = true;
			m_GroundGuidelinesEnd->enabled = true;
			break;
		case RoadConstructionMode::Change:
			break;
		case RoadConstructionMode::Destruct:
			break;
		default:
			break;
		}
	}

	u64 RoadManager::AddRoadSegment(const std::array<v3, 4>& curvePoints, s8 elevation_type)
	{
		GameApp* app = m_Scene->MainApplication;
		RoadType& type = app->road_types[m_Type];
		f32 new_road_width = elevation_type == -1 ? type.tunnel_width : type.road_width;
		f32 new_road_height = elevation_type == -1 ? type.tunnel_height : type.road_height;
		f32 new_road_length = elevation_type == -1 ? type.tunnel_length : type.road_length;

		m_Segments.push_back(RoadSegment(m_Type, curvePoints, elevation_type));
		u64 rsIndex = m_Segments.size() - 1;

		std::array<v3, 2> new_road_bounding_box = Math::get_bounding_box_from_cubic_bezier_curve(curvePoints, new_road_width * 0.5f, new_road_height);
		std::vector<std::array<v3, 3>> new_road_bounding_polygon = Math::get_bounding_polygon_from_bezier_curve(curvePoints, new_road_width * 0.5f, new_road_length);

		auto& buildings = m_Scene->m_BuildingManager.GetBuildings();
		if (m_Scene->m_BuildingManager.restrictions[0] && (restrictionFlags & (u8)RoadRestrictions::RESTRICT_COLLISIONS))
		{
			for (u64 i = 0; i < buildings.size(); i++)
			{
				Building* building = buildings[i];
				if (check_road_building_collision(building, new_road_bounding_box, new_road_bounding_polygon))
				{
					if (building->connectedRoadSegment)
					{
						RoadSegment& rs = m_Segments[building->connectedRoadSegment];
						auto it = std::find(rs.Buildings.begin(), rs.Buildings.end(), building);
						rs.Buildings.erase(it);
					}
					buildings.erase(buildings.begin() + i);
					delete building;
					i--;
				}
			}
		}

		auto& trees = m_Scene->m_TreeManager.GetTrees();
		if (m_Scene->m_TreeManager.restrictions[0] && (restrictionFlags & (u8)RoadRestrictions::RESTRICT_COLLISIONS))
		{
			for (u64 i = 0; i < trees.size(); i++)
			{
				Object* tree = trees[i]->object;
				if (check_road_tree_collision(tree, new_road_bounding_box, new_road_bounding_polygon))
				{
					trees.erase(trees.begin() + i);
					delete tree;
					i--;
				}
			}
		}

		///////////////////
		if (m_StartSnappedNode != -1)
		{
			RoadNode& node = m_Nodes[m_StartSnappedNode];
			RoadSegment& rs = m_Segments[rsIndex];
			rs.StartNode = m_StartSnappedNode;
			node.AddRoadSegment({ rsIndex });
		}
		else if (m_StartSnappedSegment != -1)
		{
			m_Segments.push_back(RoadSegment(
				m_Type,
				curvePoints,
				m_Segments[m_StartSnappedSegment].elevation_type
			));
			u64 newRSIndex = m_Segments.size() - 1;
			RoadSegment& newRS = m_Segments[newRSIndex];

			RoadSegment& segment = m_Segments[m_StartSnappedSegment];
			std::array<v3, 4> curve1{
				segment.GetCurvePoint(0),
				segment.GetCurvePoint(1),
				v3(0.0f),
				curvePoints[0],
			};
			curve1[1] = curve1[0] + (curve1[1] - curve1[0]) * m_StartSnappedT;
			curve1[2] = Math::QuadraticCurve(std::array<v3, 3>{
				segment.GetCurvePoint(0),
					segment.GetCurvePoint(1),
					segment.GetCurvePoint(2)
			}, m_StartSnappedT);

			std::array<v3, 4> curve2 = {
				segment.GetCurvePoint(3),
				segment.GetCurvePoint(2),
				v3(0.0f),
				curvePoints[0],
			};
			curve2[1] = curve2[1] + (curve2[0] - curve2[1]) * m_StartSnappedT;
			curve2[2] = Math::QuadraticCurve(std::array<v3, 3>{
				segment.GetCurvePoint(1),
					segment.GetCurvePoint(2),
					segment.GetCurvePoint(3)
			}, m_StartSnappedT);
			newRS.SetType(segment.type);
			newRS.SetCurvePoints(curve2);

			segment.SetCurvePoints(curve1);

			m_Nodes.push_back(RoadNode({}, curvePoints[0], segment.elevation_type));
			RoadNode& endNode = m_Nodes[segment.EndNode];
			RoadNode& startNode = m_Nodes[segment.StartNode];
			endNode.RemoveRoadSegment(m_StartSnappedSegment);
			newRS.StartNode = segment.EndNode;
			endNode.AddRoadSegment({ newRSIndex });

			u64 nodeIndex = m_Nodes.size() - 1;
			RoadNode& node = m_Nodes[nodeIndex];
			node.index = nodeIndex;
			RoadSegment& rs = m_Segments[rsIndex];
			segment.EndNode = nodeIndex;
			segment.ReConstruct();
			newRS.EndNode = nodeIndex;
			newRS.ReConstruct();
			rs.StartNode = nodeIndex;
			rs.ReConstruct();

			node.AddRoadSegment({ (u64)m_StartSnappedSegment, newRSIndex, rsIndex });

			for (u64 bIndex = 0; bIndex < segment.Buildings.size(); bIndex++)
			{
				Building* building = segment.Buildings[bIndex];
				f32 t = building->snapped_t;
				u64 t_index = building->snapped_t_index;
				u64 count = segment.curve_samples.size() - 1;
				if (t < m_StartSnappedT)
				{
					building->snapped_t = t / m_StartSnappedT;
					building->snapped_t_index = t_index / m_StartSnappedT;
				}
				else
				{
					building->snapped_t = (1.0f - t) / (1.0f - m_StartSnappedT);
					building->snapped_t_index = (count - t_index) / (count - m_StartSnappedT);
					auto bIt = std::find(segment.Buildings.begin(), segment.Buildings.end(), building);
					segment.Buildings.erase(bIt);
					bIndex--;
					newRS.Buildings.push_back(building);
				}
			}
			for (u64 cIndex = 0; cIndex < segment.Cars.size(); cIndex++)
			{
				Car* car = segment.Cars[cIndex];
				u64 t_index = car->t_index;
				std::vector<f32> ts{ 0.0f };
				f32 lengthRoad = app->road_types[segment.type].road_length;
				std::vector<v3> samples = Math::GetCubicCurveSamples(segment.GetCurvePoints(), lengthRoad, ts);
				if (t_index >= ts.size())
				{
					t_index = ts.size() - 1;
				}
				f32 t = ts[t_index];
				if (t < m_StartSnappedT)
				{
					std::vector<f32> ts2{ 0.0f };
					std::vector<v3> samples2 = Math::GetCubicCurveSamples(segment.GetCurvePoints(), lengthRoad, ts2);
					f32 teanew = t / m_StartSnappedT;
					for (int i = 0; i < ts2.size(); i++)
					{
						if (ts2[i] < teanew)
							continue;
						if (car->fromStart)
						{
							car->t_index = i - 1;
							car->target = samples2[i];

						}
						else
						{
							car->t_index = i;
							car->target = samples2[i - 1];
						}
						break;
					}
				}
				else
				{

					std::vector<f32> ts2{ 0 };
					std::vector<v3> samples2 = Math::GetCubicCurveSamples(newRS.GetCurvePoints(), lengthRoad, ts2);
					f32 teanew = (1.0f - t) / (1.0f - m_StartSnappedT);
					for (int i = 0; i < ts2.size(); i++)
					{
						if (ts2[i] < teanew)
							continue;
						if (car->fromStart)
						{
							car->t_index = i;
							car->target = samples2[i - 1];
							car->fromStart = false;
						}
						else
						{
							car->t_index = i - 1;
							car->target = samples2[i];
							car->fromStart = true;
						}
						break;
					}
					newRS.Cars.push_back(car);
					car->roadSegment = newRSIndex;
					auto cIt = std::find(segment.Cars.begin(), segment.Cars.end(), car);
					segment.Cars.erase(cIt);
				}
			}
		}
		else
		{
			RoadSegment& rs = m_Segments[rsIndex];
			m_Nodes.push_back(RoadNode({}, curvePoints[0], rs.elevation_type));
			u64 nodeIndex = m_Nodes.size() - 1;
			RoadNode& node = m_Nodes[nodeIndex];
			rs.StartNode = nodeIndex;
			node.index = nodeIndex;
			node.AddRoadSegment({ rsIndex });
		}

		if (m_EndSnappedNode != -1)
		{
			RoadSegment& rs = m_Segments[rsIndex];
			RoadNode& node = m_Nodes[m_EndSnappedNode];
			rs.EndNode = m_EndSnappedNode;
			node.AddRoadSegment({ rsIndex });
		}
		else if (m_EndSnappedSegment != -1)
		{
			m_Segments.push_back(RoadSegment(
				m_Type,
				curvePoints,
				m_Segments[m_EndSnappedSegment].elevation_type
			));
			u64 newRSIndex = m_Segments.size() - 1;
			RoadSegment& newRS = m_Segments[newRSIndex];

			RoadSegment& segment = m_Segments[m_EndSnappedSegment];
			std::array<v3, 4> curve1{
				segment.GetCurvePoint(0),
				segment.GetCurvePoint(1),
				v3(0.0f),
				curvePoints[3],
			};
			curve1[1] = curve1[0] + (curve1[1] - curve1[0]) * m_EndSnappedT;
			curve1[2] = Math::QuadraticCurve(std::array<v3, 3>{
				segment.GetCurvePoint(0),
					segment.GetCurvePoint(1),
					segment.GetCurvePoint(2)
			}, m_EndSnappedT);

			std::array<v3, 4> curve2 = {
				segment.GetCurvePoint(3),
				segment.GetCurvePoint(2),
				v3(0.0f),
				curvePoints[3],
			};
			curve2[1] = curve2[1] + (curve2[0] - curve2[1]) * m_EndSnappedT;
			curve2[2] = Math::QuadraticCurve(std::array<v3, 3>{
				segment.GetCurvePoint(1),
					segment.GetCurvePoint(2),
					segment.GetCurvePoint(3)
			}, m_EndSnappedT);
			newRS.SetType(segment.type);
			newRS.SetCurvePoints(curve2);

			segment.SetCurvePoints(curve1);

			m_Nodes.push_back(RoadNode({ }, curvePoints[3], segment.elevation_type));
			RoadNode& endNode = m_Nodes[segment.EndNode];
			RoadNode& startNode = m_Nodes[segment.StartNode];
			endNode.RemoveRoadSegment(m_EndSnappedSegment);
			newRS.StartNode = segment.EndNode;
			endNode.AddRoadSegment({ newRSIndex });

			u64 nodeIndex = m_Nodes.size() - 1;
			RoadNode& node = m_Nodes[nodeIndex];
			node.index = nodeIndex;
			RoadSegment& rs = m_Segments[rsIndex];
			segment.EndNode = nodeIndex;
			segment.ReConstruct();
			newRS.EndNode = nodeIndex;
			newRS.ReConstruct();
			rs.EndNode = nodeIndex;
			rs.ReConstruct();

			node.AddRoadSegment({ (u64)m_EndSnappedSegment, newRSIndex, rsIndex });
			for (u64 bIndex = 0; bIndex < segment.Buildings.size(); bIndex++)
			{
				Building* building = segment.Buildings[bIndex];
				f32 t = building->snapped_t;
				u64 t_index = building->snapped_t_index;
				u64 count = segment.curve_samples.size() - 1;
				if (t < m_EndSnappedT)
				{
					building->snapped_t = t / m_StartSnappedT;
					building->snapped_t_index = t_index / m_StartSnappedT;
				}
				else
				{
					building->snapped_t = (1.0f - t) / (1.0f - m_StartSnappedT);
					building->snapped_t_index = (count - t_index) / (count - m_StartSnappedT);
					auto bIt = std::find(segment.Buildings.begin(), segment.Buildings.end(), building);
					segment.Buildings.erase(bIt);
					bIndex--;
					newRS.Buildings.push_back(building);
				}
			}

			for (u64 cIndex = 0; cIndex < segment.Cars.size(); cIndex++)
			{
				Car* car = segment.Cars[cIndex];
				u64 t_index = car->t_index;
				std::vector<f32> ts{ 0 };
				f32 lengthRoad = app->road_types[segment.type].road_length;
				std::vector<v3> samples = Math::GetCubicCurveSamples(segment.GetCurvePoints(), lengthRoad, ts);
				if (t_index >= ts.size())
				{
					t_index = ts.size() - 1;
				}
				f32 t = ts[t_index];
				if (t < m_EndSnappedT)
				{
					std::vector<f32> ts2{ 0 };
					std::vector<v3> samples2 = Math::GetCubicCurveSamples(segment.GetCurvePoints(), lengthRoad, ts2);
					f32 teanew = t / m_EndSnappedT;
					for (int i = 0; i < ts2.size(); i++)
					{
						if (ts2[i] < teanew)
							continue;
						if (car->fromStart)
						{
							car->t_index = i - 1;
							car->target = samples2[i];

						}
						else
						{
							car->t_index = i;
							car->target = samples2[i - 1];
						}
						break;
					}
				}
				else
				{

					std::vector<f32> ts2{ 0 };
					std::vector<v3> samples2 = Math::GetCubicCurveSamples(newRS.GetCurvePoints(), lengthRoad, ts2);
					f32 teanew = (1.0f - t) / (1.0f - m_EndSnappedT);
					for (int i = 0; i < ts2.size(); i++)
					{
						if (ts2[i] < teanew)
							continue;
						if (car->fromStart)
						{
							car->t_index = i;
							car->target = samples2[i - 1];
							car->fromStart = false;
						}
						else
						{
							car->t_index = i - 1;
							car->target = samples2[i];
							car->fromStart = true;
						}
						break;
					}
					newRS.Cars.push_back(car);
					car->roadSegment = newRSIndex;
					auto cIt = std::find(segment.Cars.begin(), segment.Cars.end(), car);
					segment.Cars.erase(cIt);
				}
			}
		}
		else
		{
			RoadSegment& rs = m_Segments[rsIndex];
			m_Nodes.push_back(RoadNode({}, curvePoints[3], rs.elevation_type));
			u64 nodeIndex = m_Nodes.size() - 1;
			RoadNode& node = m_Nodes[nodeIndex];
			rs.EndNode = nodeIndex;
			node.index = nodeIndex;
			node.AddRoadSegment({ rsIndex });
		}

		return rsIndex;
	}
	u8 RoadManager::RemoveRoadSegment(u64 roadSegment, u64 roadNode)
	{
		u8 amount_decrease = 0;
		RoadSegment& rs = m_Segments[roadSegment];

		auto& buildings = m_Scene->m_BuildingManager.m_Buildings;
		auto& cars = m_Scene->m_CarManager.m_Cars;
		auto& people = m_Scene->m_PersonManager.m_People;
		auto& walking_people = m_Scene->m_PersonManager.walking_people;

		if (rs.elevation_type == 0)
			Helper::UpdateTheTerrain(&m_Segments[roadSegment], true);

		while (rs.Buildings.size() > 0)
			remove_building(rs.Buildings[0]);

		while (rs.Cars.size() > 0)
			remove_car(rs.Cars[0]);

		while (rs.peoples.size() > 0)
			remove_person(rs.peoples[0]);

		for (u64 i = walking_people.size(); i > 0; i--)
		{
			auto& path = walking_people[i - 1]->path;
			auto it = std::find(path.begin(), path.end(), roadSegment);
			if (it != path.end())
			{
				Person* wp = walking_people[i - 1];
				wp->road_segment = -1;
				wp->path = {};
				wp->target_building = nullptr;
				wp->status = PersonStatus::AtHome;
				wp->time_left = Utility::Random::Float(1.0f, 5.0f);
				wp->object->enabled = false;
				wp->heading_to_a_building = false;
				walking_people.erase(std::next(walking_people.begin(), i - 1));
			}
		}

		RoadNode& startNode = m_Nodes[rs.StartNode];
		startNode.RemoveRoadSegment(roadSegment);
		if (startNode.roadSegments.size() == 0)
		{
			if (rs.StartNode < roadNode) amount_decrease++;
			for (Person* p : walking_people)
				for (u64 i = 1; i < p->path.size(); i += 2)
					if (p->path[i] > rs.StartNode)
						p->path[i]--;
			Helper::UpdateTheTerrain(startNode.bounding_polygon, true);
			u64 count = m_Segments.size();
			for (u64 rsIndex = 0; rsIndex < count; rsIndex++)
			{
				RoadSegment& segment = m_Segments[rsIndex];
				if (segment.StartNode > rs.StartNode)
					segment.StartNode--;
				if (segment.EndNode > rs.StartNode)
					segment.EndNode--;
			}
			for (u64 i = rs.StartNode; i < m_Nodes.size(); i++)
				m_Nodes[i].index--;
			m_Nodes.erase(std::find(m_Nodes.begin(), m_Nodes.end(), startNode));
		}
		RoadNode& endNode = m_Nodes[rs.EndNode];
		endNode.RemoveRoadSegment(roadSegment);

		if (endNode.roadSegments.size() == 0)
		{
			if (rs.EndNode < roadNode) amount_decrease++;
			Helper::UpdateTheTerrain(endNode.bounding_polygon, true);
			for (Person* p : walking_people)
				for (u64 i = 1; i < p->path.size(); i += 2)
					if (p->path[i] > rs.EndNode)
						p->path[i]--;
			u64 count = m_Segments.size();
			for (u64 rsIndex = 0; rsIndex < count; rsIndex++)
			{
				RoadSegment& segment = m_Segments[rsIndex];
				if (segment.StartNode > rs.EndNode)
					segment.StartNode--;
				if (segment.EndNode > rs.EndNode)
					segment.EndNode--;
			}
			for (u64 i = rs.EndNode; i < m_Nodes.size(); i++)
				m_Nodes[i].index--;
			m_Nodes.erase(std::find(m_Nodes.begin(), m_Nodes.end(), endNode));
		}

		u64 count = m_Nodes.size();
		for (u64 nIndex = 0; nIndex < count; nIndex++)
		{
			RoadNode& node = m_Nodes[nIndex];
			u64 c = node.roadSegments.size();
			for (u64 i = 0; i < c; i++)
				if (node.roadSegments[i] > roadSegment)
					node.roadSegments[i]--;
		}
		count = cars.size();
		for (u64 cIndex = 0; cIndex < count; cIndex++)
		{
			Car* car = cars[cIndex]; // well be changed to indices instead of pointers
			if ((u64)car->roadSegment > roadSegment)
				car->roadSegment--;
		}
		count = buildings.size();
		for (u64 bIndex = 0; bIndex < count; bIndex++)
		{
			Building* building = buildings[bIndex];
			if ((u64)building->connectedRoadSegment > roadSegment)
				building->connectedRoadSegment--;
		}
		for (Person* p : walking_people)
			for (u64 i = 0; i < p->path.size(); i += 2)
				if (p->path[i] > roadSegment)
					p->path[i]--;

		m_Segments.erase(std::find(m_Segments.begin(), m_Segments.end(), rs));
		return amount_decrease;
	}

	SnapInformation RoadManager::CheckSnapping(const v3& prevLocation)
	{
		GameApp* app = m_Scene->MainApplication;
		SnapInformation results;
		RoadType& type = app->road_types[m_Type];

		f32 min_distance_to_snap = -prevLocation.y > type.tunnel_height ? type.tunnel_width : type.road_width;

		results.location = prevLocation;
		for (u64 i = 0; i < m_Nodes.size(); i++)
		{
			RoadNode& node = m_Nodes[i];
			v2 distance_vector = (v2)(node.position - prevLocation);
			if (glm::length(distance_vector) < min_distance_to_snap)
			{
				results.location = node.position;
				results.snapped = true;
				results.node = i;
				results.elevation_type = node.elevation_type;
				return results;
			}
		}

		v2 point = (v2)prevLocation;
		for (u64 i = 0; i < m_Segments.size(); i++)
		{
			RoadSegment& segment = m_Segments[i];
			if (Math::CheckPolygonPointCollision(segment.bounding_rect, point))
			{
				f32 width = segment.elevation_type == -1 ? app->road_types[segment.type].tunnel_width : app->road_types[segment.type].road_width;
				f32 snapDist = (min_distance_to_snap + width) * 0.5f;
				std::vector<v3> samples = segment.curve_samples;
				u64 curve_samples_size = samples.size();
				v3 point0 = samples[0];
				for (u64 j = 1; j < curve_samples_size; j++)
				{
					v3 point1 = samples[j];
					v2 dirToP1 = (v2)(point1 - point0);
					f32 lenr = glm::length(dirToP1);

					v2 dirToPrev = (v2)(prevLocation - point0);
					f32 len = glm::length(dirToPrev);

					f32 angle = glm::acos(glm::dot(dirToP1, dirToPrev) / (lenr * len));
					f32 dist = len * glm::sin(angle);

					if (dist < snapDist)
					{
						f32 c = len * glm::cos(angle);
						if (c >= -0.5f * width && c <= lenr + 0.5f * width)
						{
							f32 t = std::max(0.0f, std::min(1.0f, glm::cos(angle)));
							results.location = point0 + v3(t * lenr * dirToP1, 0.0f);
							results.segment = i;
							results.snapped = true;
							results.T = Math::Lerp(segment.curve_t_samples[j - 1], segment.curve_t_samples[j], t);
							results.elevation_type = segment.elevation_type;
							return results;
						}
					}
					point0 = point1;
				}
			}
		}
		return results;
	}

	void RoadManager::ResetStates()
	{
		m_ConstructionPhase = 0;

		b_ConstructionStartSnapped = false;
		b_ConstructionEndSnapped = false;
		b_ConstructionRestricted = false;

		m_ConstructionPositions = {
			v3(0.0f),
			v3(0.0f),
			v3(0.0f)
		};

		m_StartSnappedNode = -1;
		m_StartSnappedSegment = -1;
		m_StartSnappedT = 0.0f;

		m_EndSnappedNode = -1;
		m_EndSnappedSegment = -1;
		m_EndSnappedT = 0.0f;

		m_DestructionNode = -1;
		m_DestructionSegment = -1;

		selected_road_segment = -1;
		selected_road_node = -1;

		for (RoadSegment& rs : m_Segments)
		{
			rs.object->enabled = true;	// Is this needed anymore??
			rs.object->SetTransform(rs.GetStartPosition());
		}

		for (RoadNode& node : m_Nodes)
		{
			node.object->enabled = true;	// Is this needed anymore??
			node.object->SetTransform(node.position);
		}

		ResetGuideLines();

		m_GroundGuidelinesStart->enabled = false;
		m_GroundGuidelinesEnd->enabled = false;
		m_TunnelGuidelinesStart->enabled = false;
		m_TunnelGuidelinesEnd->enabled = false;

		m_GroundGuidelinesStart->tintColor = v4(1.0f);
		m_GroundGuidelinesEnd->tintColor = v4(1.0f);
		m_TunnelGuidelinesStart->tintColor = v4(1.0f);
		m_TunnelGuidelinesEnd->tintColor = v4(1.0f);
	}

	void RoadManager::SnapToGrid(v3& prevLocation)
	{
		if (snapFlags & (u8)RoadSnapOptions::SNAP_TO_GRID)
		{
			prevLocation.x = prevLocation.x - std::fmod(prevLocation.x, 0.5f) + 0.25f;
			prevLocation.y = prevLocation.y - std::fmod(prevLocation.y, 0.5f) + 0.25f;
		}
	}
	void RoadManager::SnapToRoad(v3& prevLocation, bool isStart)
	{
		if (snapFlags & (u8)RoadSnapOptions::SNAP_TO_ROAD)
		{
			SnapInformation snapInformation = CheckSnapping(prevLocation);
			prevLocation = snapInformation.location;
			if (isStart)
			{
				b_ConstructionStartSnapped = snapInformation.snapped;
				m_StartSnappedSegment = snapInformation.segment;
				m_StartSnappedNode = snapInformation.node;
				m_StartSnappedT = snapInformation.T;
				if (b_ConstructionStartSnapped)
					m_Elevationtypes[0] = snapInformation.elevation_type;
			}
			else
			{
				b_ConstructionEndSnapped = snapInformation.snapped;
				m_EndSnappedSegment = snapInformation.segment;
				m_EndSnappedNode = snapInformation.node;
				m_EndSnappedT = snapInformation.T;
				if (b_ConstructionEndSnapped)
					m_Elevationtypes[3] = snapInformation.elevation_type;
			}
		}
	}
	void RoadManager::SnapToHeight(const std::vector<u8>& indices, u8 index, v3& AB)
	{
		if (snapFlags & (u8)RoadSnapOptions::SNAP_TO_HEIGHT)
		{
			for (u8 i = 0; i < indices.size(); i++)
				m_ConstructionPositions[indices[i]].z = m_ConstructionPositions[index].z;
			AB.z = 0.0f;
		}
	}
	void RoadManager::SnapToAngle(v3& AB, s64 snappedNode, s64 snappedRoadSegment, f32 snappedT)
	{
		if (snapFlags & (u8)RoadSnapOptions::SNAP_TO_ANGLE)
		{
			f32 rotation1 = glm::atan(-AB.y / AB.x) + (f32)(AB.x < 0.0f) * glm::radians(180.0f);
			f32 length1 = glm::length(AB);
			if (length1 > 0.1f)
			{
				f32 newAngle = 0.0f;
				f32 endAngle = glm::degrees(rotation1);
				f32 angle = 0.0f;
				v3 crossProduct = v3(0.0f, 1.0f, 0.0f);
				if (snappedNode != -1)
				{
					RoadNode& node = m_Nodes[snappedNode];
					f32 minAngle = 180.0f;
					for (u64 rsIndex : node.roadSegments)
					{
						RoadSegment& rs = m_Segments[rsIndex];
						v3 dir = rs.StartNode == snappedNode ? rs.GetStartDirection() : rs.GetEndDirection();

						f32 dotResult = glm::dot(dir, AB);
						dotResult = std::max(-1.0f, std::min(1.0f, (dotResult / (glm::length(dir) * glm::length(AB)))));
						newAngle = glm::degrees(glm::acos(dotResult));

						if (newAngle < minAngle)
						{
							minAngle = newAngle;
							crossProduct = glm::cross(dir, AB);
						}
					}
					angle = minAngle;
					if (angle < 30.0f)
						newAngle = 30.0f;
					else if (angle > 80.0f && angle < 100.0f)
						newAngle = 90.0f;
					else if (angle > 170.0f)
						newAngle = 180.0f;
					else if (Input::IsKeyPressed(KeyCode::LeftControl))
						newAngle = angle + 2.5f - std::fmod(angle + 2.5f, 5.0f);
					else
						newAngle = angle;
				}
				else if (snappedRoadSegment != -1)
				{
					RoadSegment& segment = m_Segments[snappedRoadSegment];
					v3 tangent = Math::CubicCurveTangent(segment.GetCurvePoints(), snappedT);
					crossProduct = glm::cross(tangent, AB);

					f32 dotResult = glm::dot(tangent, AB);
					dotResult = std::max(-1.0f, std::min(1.0f, (dotResult / (glm::length(tangent) * glm::length(AB)))));
					angle = glm::degrees(glm::acos(dotResult));

					if (angle < 30.0f)
						newAngle = 30.0f;
					else if (angle > 80.0f && angle < 100.0f)
						newAngle = 90.0f;
					else if (angle > 150.0f)
						newAngle = 150.0f;
					else if (Input::IsKeyPressed(KeyCode::LeftControl))
						newAngle = angle + 2.5f - std::fmod(angle + 2.5f, 5.0f);
					else
						newAngle = angle;
				}
				else if (Input::IsKeyPressed(KeyCode::LeftControl))
				{
					f32 angle = std::fmod(endAngle + 720.0f, 360.0f);
					newAngle = angle + 2.5f - std::fmod(angle + 2.5f, 5.0f);
				}
				f32 angleDiff = crossProduct.z > 0.0 ? glm::radians(newAngle - angle) : glm::radians(angle - newAngle);
				AB = glm::rotateZ(AB, angleDiff);
			}
		}
	}
	void RoadManager::ResetGuideLines()
	{
		for (std::vector<Object*>& os : m_GroundGuidelines)
			for (Object* rsg : os)
				rsg->enabled = false;
		for (u64& inUse : m_GroundGuidelinesInUse)
			inUse = 0;
		for (std::vector<Object*>& os : m_TunnelGuidelines)
			for (Object* rsg : os)
				rsg->enabled = false;
		for (u64& inUse : m_TunnelGuidelinesInUse)
			inUse = 0;
	}
	bool RoadManager::RestrictSmallAngles(v2 direction, s64 snappedNode, s64 snappedRoadSegment, f32 snappedT)
	{
		if (restrictionFlags & (u8)RoadRestrictions::RESTRICT_SMALL_ANGLES)
		{
			direction = glm::normalize(direction);

			if (snappedNode != -1)
			{
				RoadNode& node = m_Nodes[snappedNode];
				for (u64 rsIndex : node.roadSegments)
				{
					RoadSegment& rs = m_Segments[rsIndex];

					v2 dirToOldRS = (v2)(rs.StartNode == snappedNode ? rs.GetStartDirection() : rs.GetEndDirection());
					dirToOldRS = glm::normalize(dirToOldRS);

					f32 dotResult = glm::dot(direction, dirToOldRS);
					dotResult /= glm::length(direction) * glm::length(dirToOldRS);
					dotResult = std::max(-1.0f, std::min(1.0f, dotResult));
					f32 angle = glm::acos(dotResult);
					if (angle < glm::radians(30.0f))
					{
						return true;
					}
				}
			}
			else if (snappedRoadSegment != -1)
			{
				RoadSegment& rs = m_Segments[snappedRoadSegment];
				v2 tangent = (v2)(v3)(Math::CubicCurveTangent(rs.GetCurvePoints(), snappedT));
				tangent = glm::normalize(tangent);

				f32 dotResult = glm::dot(direction, tangent);
				dotResult /= glm::length(direction) * glm::length(tangent);
				dotResult = std::max(-1.0f, std::min(1.0f, dotResult));
				f32 angle = glm::acos(dotResult);

				return angle < glm::radians(30.0f) || angle > glm::radians(150.0f);
			}
		}
		return false;
	}
}