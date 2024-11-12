#include "canpch.h"
#include "RoadManager.h"

#include "Building.h"

#include "GameApp.h"
#include "Scenes/GameScene.h"
#include "TreeManager.h"
#include "BuildingManager.h"
#include "Helper.h"

#include "Can/Math.h"

#include "Types/Building_Type.h"
#include "Types/Vehicle_Type.h"
#include "Types/RoadSegment.h"
#include "Types/Transition.h"
#include "Types/Road_Type.h"
#include "Types/RoadNode.h"
#include "Types/Person.h"
#include "Types/Tree.h"

namespace Can
{
	void resnapp_buildings(u64 old_rs_index, u64 new_rs_index, u64 snapped_index);
	void reassign_people_on_the_road(u64 old_rs_index, u64 new_rs_index, u64 new_rn_index, u64 snapped_index);
	void update_path_of_walking_people_if_a_node_is_changed(u64 road_node_index, u64 modified_index, bool isAdded);

	RoadManager::RoadManager(GameScene* scene)
		: m_Scene(scene)
	{
		const Road_Type& type = m_Scene->MainApplication->road_types[m_Type];

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
			const Road_Type& t = m_Scene->MainApplication->road_types[i];

			m_GroundGuidelinesInUse.push_back(0);
			m_GroundGuidelines.push_back({});
			m_GroundGuidelines[i].push_back(new Object(t.road));
			m_GroundGuidelines[i][0]->enabled = false;

			m_TunnelGuidelinesInUse.push_back(0);
			m_TunnelGuidelines.push_back({});
			m_TunnelGuidelines[i].push_back(new Object(t.tunnel));
			m_TunnelGuidelines[i][0]->enabled = false;
		}

		new_unordered_array<RoadSegment>(&road_segments, (u64)10);
		new_unordered_array<RoadNode>(&road_nodes, (u64)10);
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
		Road_Type& type = m_Scene->MainApplication->road_types[m_Type];
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
			bool angleIsRestricted = RestrictSmallAngles(dir, m_StartSnappedNode, m_StartSnappedSegment, start_snapped_index);
			angleIsRestricted |= RestrictSmallAngles(-dir, m_EndSnappedNode, m_EndSnappedSegment, end_snapped_index);

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
				SnapToAngle(AB, m_StartSnappedNode, m_StartSnappedSegment, start_snapped_index);
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
		Road_Type& type = m_Scene->MainApplication->road_types[m_Type];
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
			bool angleIsRestricted = RestrictSmallAngles(dir, m_StartSnappedNode, m_StartSnappedSegment, start_snapped_index);

			v3 AB = m_ConstructionPositions[3] - m_ConstructionPositions[0];

			SnapToHeight({ 1, 2, 3 }, 0, AB);
			SnapToAngle(AB, m_StartSnappedNode, m_StartSnappedSegment, start_snapped_index);
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
			bool angleIsRestricted = RestrictSmallAngles(dir1, m_StartSnappedNode, m_StartSnappedSegment, start_snapped_index);
			v2 dir2 = (v2)(m_ConstructionPositions[2] - m_ConstructionPositions[3]);
			angleIsRestricted |= RestrictSmallAngles(dir2, m_EndSnappedNode, m_EndSnappedSegment, end_snapped_index);
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
		Road_Type& type = m_Scene->MainApplication->road_types[m_Type];
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
				angleIsRestricted = RestrictSmallAngles(dir, m_StartSnappedNode, m_StartSnappedSegment, start_snapped_index);
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
				SnapToAngle(AB, m_StartSnappedNode, m_StartSnappedSegment, start_snapped_index);
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
				bool angleIsRestricted = RestrictSmallAngles(dir, m_EndSnappedNode, m_EndSnappedSegment, end_snapped_index);
			}
			else
			{
				v2 dir = (v2)(m_ConstructionPositions[1] - m_ConstructionPositions[0]);
				bool angleIsRestricted = RestrictSmallAngles(dir, m_StartSnappedNode, m_StartSnappedSegment, start_snapped_index);
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
				SnapToAngle(AB, m_StartSnappedNode, m_StartSnappedSegment, start_snapped_index);
				m_ConstructionPositions[1] = m_ConstructionPositions[0] + AB;
			}
			else if (cubicCurveOrder[3] == 1 && cubicCurveOrder[2] == 2)
			{
				v3 AB = m_ConstructionPositions[2] - m_ConstructionPositions[3];
				SnapToAngle(AB, m_EndSnappedNode, m_EndSnappedSegment, end_snapped_index);
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
			bool angleIsRestricted = RestrictSmallAngles(dir1, m_StartSnappedNode, m_StartSnappedSegment, start_snapped_index);
			v2 dir2 = (v2)(m_ConstructionPositions[2] - m_ConstructionPositions[3]);
			angleIsRestricted |= RestrictSmallAngles(dir2, m_EndSnappedNode, m_EndSnappedSegment, end_snapped_index);

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
				SnapToAngle(AB, m_StartSnappedNode, m_StartSnappedSegment, start_snapped_index);
				m_ConstructionPositions[1] = m_ConstructionPositions[0] + AB;
			}
			else if (cubicCurveOrder[3] == 2)
			{
				v3 AB = m_ConstructionPositions[2] - m_ConstructionPositions[3];
				SnapToAngle(AB, m_EndSnappedNode, m_EndSnappedSegment, end_snapped_index);
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
			road_segments[selected_road_segment].object->SetTransform(road_segments[selected_road_segment].GetStartPosition());

		v2 prevLoc2D = (v2)prevLocation;
		Prefab* roadType = app->road_types[m_Type].road;
		u64 capacity = road_segments.capacity;
		for (u64 rsIndex = 0; rsIndex < capacity; rsIndex++)
		{
			auto values = road_segments.values;
			if (values[rsIndex].valid == false)
				continue;
			RoadSegment& rs = values[rsIndex].value;
			Road_Type& type = app->road_types[rs.type];

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
				assert(curve_samples_size > 1 && "Samples size can't be smaller than 2");
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

		for (u64 i = 0; i < road_segments.capacity; i++)
		{
			auto& value = road_segments.values[i];
			if (value.valid)
				value.value.object->SetTransform(value.value.GetStartPosition());
		}

		for (u64 i = 0; i < road_nodes.capacity; i++)
		{
			auto& value = road_nodes.values[i];
			if (value.valid)
				value.value.object->SetTransform(value.value.position);
		}


		if (snapInformation.snapped)
		{
			if (m_DestructionNode != -1)
			{
				RoadNode& road_node = road_nodes[m_DestructionNode];
				for (u64 rsIndex : road_node.roadSegments)
				{
					RoadSegment& rs = road_segments[rsIndex];
					RoadNode& startNode = road_nodes[rs.StartNode];
					RoadNode& endNode = road_nodes[rs.EndNode];

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
				RoadSegment& rs = road_segments[m_DestructionSegment];
				RoadNode& startNode = road_nodes[rs.StartNode];
				RoadNode& endNode = road_nodes[rs.EndNode];

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
		Road_Type& type = m_Scene->MainApplication->road_types[m_Type];

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
			v3 intersectionPoint = Math::ray_plane_intersection(
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
			count = (int)countT + (int)countG;
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
		Road_Type& type = m_Scene->MainApplication->road_types[m_Type];

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
				m_TunnelGuidelinesInUse[m_Type]++;
				if (m_TunnelGuidelinesInUse[m_Type] > m_TunnelGuidelines[m_Type].size())
					m_TunnelGuidelines[m_Type].push_back(new Object(m_Scene->MainApplication->road_types[m_Type].tunnel));
				guideline_object = m_TunnelGuidelines[m_Type][tunnelIndex];
				scale = length / type.tunnel_length;
				tunnelIndex++;
			}
			else
			{
				m_GroundGuidelinesInUse[m_Type]++;
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
		Road_Type& t = m_Scene->MainApplication->road_types[m_Type];
		f32 guideline_height = m_Elevationtypes[0] == -1 ? t.tunnel_height : t.road_height;

		u64 capacity = road_segments.capacity;
		for (u64 rsIndex = 0; rsIndex < capacity; rsIndex++)
		{
			auto values = road_segments.values;
			if (values[rsIndex].valid == false)
				continue;
			RoadSegment& rs = values[rsIndex].value;
			Road_Type& type = app->road_types[rs.type];
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
		Road_Type& type = m_Scene->MainApplication->road_types[m_Type];
		f32 height = m_Elevationtypes[0] == -1 ? type.tunnel_height : type.road_height;

		Prefab* prefab = building->object->prefab;
		f32 rot = building->object->rotation.z;
		f32 building_height = prefab->boundingBoxM.z - prefab->boundingBoxL.z;

		v3 A = v3{ prefab->boundingBoxL.x, prefab->boundingBoxL.y, prefab->boundingBoxL.z };
		v3 B = v3{ prefab->boundingBoxL.x, prefab->boundingBoxM.y, prefab->boundingBoxL.z };
		v3 C = v3{ prefab->boundingBoxM.x, prefab->boundingBoxL.y, prefab->boundingBoxL.z };
		v3 D = v3{ prefab->boundingBoxM.x, prefab->boundingBoxM.y, prefab->boundingBoxL.z };

		A = glm::rotateZ(A, rot) + building->object->position;
		B = glm::rotateZ(B, rot) + building->object->position;
		C = glm::rotateZ(C, rot) + building->object->position;
		D = glm::rotateZ(D, rot) + building->object->position;

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
		Road_Type& type = m_Scene->MainApplication->road_types[m_Type];
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
				Road_Type& type = m_Scene->MainApplication->road_types[m_Type];

				v3 intersectionPoint = Math::ray_plane_intersection(
					m_ConstructionPositions[0],
					glm::normalize(m_ConstructionPositions[3] - m_ConstructionPositions[0]),
					v3{ 0.0f, 0.0f, -type.tunnel_height },
					v3{ 0.0f, 0.0f, 1.0f }
				);

				auto [road_node, road_node_index] = array_add_empty(&road_nodes);
				RoadNode::construct(road_node, {}, intersectionPoint, -1, road_node_index);

				s64 snapNode = m_EndSnappedNode;
				s64 snapSegment = m_EndSnappedSegment;
				bool snapped = b_ConstructionEndSnapped;
				m_EndSnappedNode = road_node_index;
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
				m_StartSnappedNode = road_node_index;
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
				Road_Type& type = m_Scene->MainApplication->road_types[m_Type];
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
						auto [road_node, road_node_index] = array_add_empty(&road_nodes);
						RoadNode::construct(road_node, {}, samples[i], -1, road_node_index);

						m_EndSnappedNode = road_node_index;
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
				Road_Type& type = m_Scene->MainApplication->road_types[m_Type];
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
						auto [road_node, road_node_index] = array_add_empty(&road_nodes);
						RoadNode::construct(road_node, {}, samples[i], -1, road_node_index);

						m_EndSnappedNode = road_node_index;
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
		auto& road_types = m_Scene->MainApplication->road_types;
		const auto& people_on_the_road = m_Scene->m_PersonManager.get_people_on_the_road();
		const auto& cars_on_the_road = m_Scene->m_CarManager.get_cars_on_the_road();

		if (selected_road_segment == -1)
			return false;
		RoadSegment& rs = road_segments[selected_road_segment];
		if (rs.elevation_type == 0)
			Helper::UpdateTheTerrain(&rs, true);

		auto& currentType = road_types[rs.type];
		auto& new_type = road_types[m_Type];

		// if not zoneable 
		if (new_type.zoneable == false)
		{
			//	*delete houses
			while (rs.buildings.size() > 0)
				remove_building(rs.buildings[0]);

			//	*reset people walking sidewalks on this road
			while (rs.people.size())
			{
				u64 size = rs.people.size();
				Person* p = rs.people[size - 1];
				reset_person_back_to_building_from(p);
			}

			//	*reset cars driving on this road
			while (rs.vehicles.size())
			{
				u64 size = rs.vehicles.size();
				Car* c = rs.vehicles[size - 1];
				reset_car_back_to_building_from(c);
			}
		}

		// TODO: if oneway?

		// if asymetric -> turn sides
		if (new_type.road == currentType.road)
		{
			if (!currentType.asymmetric)
				return false;
			//	*flip path's next_road_node 
			//		end->start
			//		start->end
			for (Person* p : people_on_the_road)
			{
				switch (p->status)
				{
				case PersonStatus::Driving:
				case PersonStatus::DrivingForWork:
					for (RS_Transition_For_Vehicle* t : p->car_driving->path)
						if (t->road_segment_index == selected_road_segment)
							t->next_road_node_index = t->road_segment_index == rs.EndNode ? rs.StartNode : rs.EndNode;
					break;
				case PersonStatus::Walking:
				case PersonStatus::WalkingDead:
					// TODO: every other is RS_Transition_For_Walking
					// so not foreach
					for (Transition* t : p->path)
					{
						auto tw = (RS_Transition_For_Walking*)t;
						if (tw->road_segment_index == selected_road_segment)
						{
							tw->from_right = !tw->from_right;
							tw->from_start = !tw->from_start;
						}
					}
					break;
				case PersonStatus::AtHome:
				case PersonStatus::AtWork:
					break;
				default:
					assert(false && "UnImplemented PersonStatus case!!!");
					break;
				}
			}

			auto& cps = rs.GetCurvePoints();
			u64 temp = rs.StartNode;
			rs.StartNode = rs.EndNode;
			rs.EndNode = temp;
			rs.SetCurvePoints({ cps[3], cps[2], cps[1], cps[0] });

			for (Building* building : rs.buildings)
			{
				building->snapped_t = 1.0f - building->snapped_t;
				building->snapped_t_index = (rs.curve_samples.size() - 1) - building->snapped_t_index;
			}
		}
		else
		{
			rs.SetType(m_Type);
		}

		road_nodes[rs.StartNode].Reconstruct();
		road_nodes[rs.EndNode].Reconstruct();

		// *update paths that passes this road
		for (Person* p : people_on_the_road)
		{
			switch (p->status)
			{
			case PersonStatus::Driving:
			case PersonStatus::DrivingForWork:
			{
				u64 count = p->car_driving->path.size();
				for (u64 i = 0; i < count - 1; i++)
				{
					auto td = p->car_driving->path[i];
					if (td->road_segment_index == selected_road_segment)
					{
						auto td_next = p->car_driving->path[i + 1];

						RoadSegment& current_road_segment = road_segments[td->road_segment_index];
						Road_Type& current_road_type = road_types[current_road_segment.type];
						auto& next_road_node_connected_road_segments = road_nodes[td->next_road_node_index].roadSegments;

						auto curr_it = std::find(
							next_road_node_connected_road_segments.begin(),
							next_road_node_connected_road_segments.end(),
							td->road_segment_index
						);
						assert(curr_it != next_road_node_connected_road_segments.end());
						s64 start_index = std::distance(
							next_road_node_connected_road_segments.begin(),
							curr_it
						);

						auto next_it = std::find(
							next_road_node_connected_road_segments.begin(),
							next_road_node_connected_road_segments.end(),
							td_next->road_segment_index
						);
						assert(next_it != next_road_node_connected_road_segments.end());
						s64 end_index = std::distance(
							next_road_node_connected_road_segments.begin(),
							next_it
						);
						if (start_index == end_index) {
							if (td_next->next_road_node_index == current_road_segment.EndNode)
							{
								td->lane_index = 0;
								td->lane_index += current_road_type.lanes_backward.size();
							}
							else
							{
								td->lane_index += current_road_type.lanes_backward.size() - 1;
							}
						}
						else
						{
							s64 road_end_counts = next_road_node_connected_road_segments.size();
							end_index = (end_index + road_end_counts - start_index) % road_end_counts;
							road_end_counts -= 2;
							start_index = 0;
							end_index--;
							if (end_index < road_end_counts * 0.3f)
							{
								if (td->next_road_node_index == current_road_segment.EndNode)
								{
									td->lane_index = current_road_type.lanes_forward.size() - 1;
									if (current_road_type.zoneable) td->lane_index -= 1;
									td->lane_index += current_road_type.lanes_backward.size();
								}
								else
								{
									td->lane_index = 0;
									if (current_road_type.zoneable) td->lane_index += 1;
								}
							}
							else if (end_index > road_end_counts * 0.7f)
							{
								if (td->next_road_node_index == current_road_segment.EndNode)
								{
									td->lane_index = 0;
									td->lane_index += current_road_type.lanes_backward.size();
								}
								else
								{
									td->lane_index = current_road_type.lanes_backward.size() - 1;
								}
							}
							else
							{
								if (td->next_road_node_index == current_road_segment.EndNode)
								{
									s64 lane_count = current_road_type.lanes_forward.size();
									if (current_road_type.zoneable) lane_count -= 1;
									td->lane_index = lane_count * 0.5f;
									td->lane_index += current_road_type.lanes_backward.size();
								}
								else
								{
									s64 lane_count = current_road_type.lanes_backward.size();
									if (current_road_type.zoneable)
									{
										lane_count -= 1;
										td->lane_index = 1;
									}
									td->lane_index += lane_count * 0.5f;
								}
							}
						}

						u64 prev_points_count = td->points_stack.size();
						td->points_stack.clear();
						auto& current_road_segment_curve_samples = current_road_segment.curve_samples;
						u64 curve_sample_count = current_road_segment_curve_samples.size();
						f32 dist_from_center = 0.0f;
						if (td->lane_index < current_road_type.lanes_backward.size())
							dist_from_center = current_road_type.lanes_backward[td->lane_index].distance_from_center;
						else
							dist_from_center = current_road_type.lanes_forward[td->lane_index - current_road_type.lanes_backward.size()].distance_from_center;
						v3 p0 = current_road_segment_curve_samples[0];
						for (u64 curve_sample_index = 1; curve_sample_index < curve_sample_count; curve_sample_index++)
						{
							v3 p1 = current_road_segment_curve_samples[curve_sample_index];
							v3 dir_to_p1 = p1 - p0;
							v3 cw_rotated_dir = glm::normalize(v3{ dir_to_p1.y, -dir_to_p1.x, 0.0f });
							v3 path_point = p0 + cw_rotated_dir * dist_from_center;
							td->points_stack.push_back(path_point);
							p0 = p1;
						}
						v3 dir_to_p1 = current_road_segment.GetEndDirection() * -1.0f;
						v3 cw_rotated_dir = glm::normalize(v3{ dir_to_p1.y, -dir_to_p1.x, 0.0f });
						v3 path_point = p0 + cw_rotated_dir * dist_from_center;
						td->points_stack.push_back(path_point);
						if (td->next_road_node_index == current_road_segment.EndNode)
							std::reverse(td->points_stack.begin(), td->points_stack.end());
						u64 curr_points_count = td->points_stack.size();
						for (u64 k = 0; curr_points_count - prev_points_count; k++)
							td->points_stack.pop_back();
					}
				}

				auto td_prev = p->car_driving->path[count - 2];
				auto td = p->car_driving->path[count - 1];
				if (td->road_segment_index == selected_road_segment)
				{
					RoadSegment& current_road_segment = road_segments[td->road_segment_index];
					Road_Type& current_road_type = road_types[current_road_segment.type];
					if (p->path_end_building->snapped_to_right)
					{
						if (td->lane_index < current_road_type.lanes_backward.size())
						{
							td->lane_index = current_road_type.lanes_forward.size() - 2;
							td->lane_index += current_road_type.lanes_backward.size();
						}
						else
						{
							td->lane_index = current_road_type.lanes_backward.size() - 1;
						}
					}
					else
					{
						if (td->lane_index < current_road_type.lanes_backward.size())
						{
							td->lane_index = 0;
							td->lane_index += current_road_type.lanes_backward.size();
						}
						else
						{
							td->lane_index = 1;
						}
					}


					u64 prev_points_count = td->points_stack.size();
					td->points_stack.clear();
					auto& current_road_segment_curve_samples = current_road_segment.curve_samples;
					u64 curve_sample_count = current_road_segment_curve_samples.size();
					f32 dist_from_center = 0.0f;
					if (td->lane_index < current_road_type.lanes_backward.size())
						dist_from_center = current_road_type.lanes_backward[td->lane_index].distance_from_center;
					else
						dist_from_center = current_road_type.lanes_forward[td->lane_index - current_road_type.lanes_backward.size()].distance_from_center;
					v3 p0 = current_road_segment_curve_samples[0];
					for (u64 curve_sample_index = 1; curve_sample_index < curve_sample_count; curve_sample_index++)
					{
						v3 p1 = current_road_segment_curve_samples[curve_sample_index];
						v3 dir_to_p1 = p1 - p0;
						v3 cw_rotated_dir = glm::normalize(v3{ dir_to_p1.y, -dir_to_p1.x, 0.0f });
						v3 path_point = p0 + cw_rotated_dir * dist_from_center;
						td->points_stack.push_back(path_point);
						p0 = p1;
					}
					v3 dir_to_p1 = current_road_segment.GetEndDirection() * -1.0f;
					v3 cw_rotated_dir = glm::normalize(v3{ dir_to_p1.y, -dir_to_p1.x, 0.0f });
					v3 path_point = p0 + cw_rotated_dir * dist_from_center;
					td->points_stack.push_back(path_point);
					if (td->next_road_node_index == current_road_segment.EndNode)
						std::reverse(td->points_stack.begin(), td->points_stack.end());
					u64 curr_points_count = td->points_stack.size();
					for (u64 k = 0; curr_points_count - prev_points_count; k++)
						td->points_stack.pop_back();
				}
				break;
			}
			case PersonStatus::AtHome:
			case PersonStatus::AtWork:
			case PersonStatus::Walking:
			case PersonStatus::WalkingDead:
				// Do nothing
				break;
			default:
				assert(false);
			}
		}

		// *resnap houses
		resnapp_buildings(selected_road_segment, selected_road_segment, rs.curve_samples.size());
		return false;
	}
	bool RoadManager::OnMousePressed_Destruction()
	{
		if (m_DestructionNode != -1)
		{
			RoadNode& road_node = road_nodes[m_DestructionNode];
			for (u64 road_segment_index : road_node.roadSegments)
				RemoveRoadSegment(road_segment_index);

			// Because we're deleting all connected road_segments 
			//		eventually road_node will be left alone and
			//		deleted by the last road_segment
		}
		else if (m_DestructionSegment != -1)
		{
			RemoveRoadSegment(m_DestructionSegment);
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

		const Road_Type& t = m_Scene->MainApplication->road_types[m_Type];
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
		Road_Type& type = app->road_types[m_Type];
		f32 new_road_width = elevation_type == -1 ? type.tunnel_width : type.road_width;
		f32 new_road_height = elevation_type == -1 ? type.tunnel_height : type.road_height;
		f32 new_road_length = elevation_type == -1 ? type.tunnel_length : type.road_length;

		auto [road_segment, road_segment_index] = array_add_empty(&road_segments);
		RoadSegment::construct(road_segment, m_Type, curvePoints, elevation_type);

		std::array<v3, 2> new_road_bounding_box = Math::get_bounding_box_from_cubic_bezier_curve(curvePoints, new_road_width * 0.5f, new_road_height);
		std::vector<std::array<v3, 3>> new_road_bounding_polygon = Math::get_bounding_polygon_from_bezier_curve(curvePoints, new_road_width * 0.5f, new_road_length);

		auto& buildings = m_Scene->m_BuildingManager.GetBuildings();
		if (m_Scene->m_BuildingManager.restrictions[0] && (restrictionFlags & (u8)RoadRestrictions::RESTRICT_COLLISIONS))
		{
			for (u64 building_index = 0; building_index < buildings.size(); building_index++)
			{
				Building* building = buildings[building_index];
				if (check_road_building_collision(building, new_road_bounding_box, new_road_bounding_polygon))
				{
					remove_building(building);
					building_index--;
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
			RoadNode& road_node = road_nodes[m_StartSnappedNode];
			RoadSegment& road_segment = road_segments[road_segment_index];
			road_segment.StartNode = m_StartSnappedNode;
			road_node.AddRoadSegment({ road_segment_index });

			u64 new_index = std::distance(
				road_node.roadSegments.begin(),
				std::find(
					road_node.roadSegments.begin(),
					road_node.roadSegments.end(),
					road_segment_index
				));
			update_path_of_walking_people_if_a_node_is_changed(m_StartSnappedNode, new_index, true);
		}
		else if (m_StartSnappedSegment != -1)
		{
			auto [new_road_segment, new_road_segment_index] = array_add_empty(&road_segments);
			auto [new_road_node, new_road_node_index] = array_add_empty(&road_nodes);
			RoadSegment& start_snapped_road_segment = road_segments[m_StartSnappedSegment];
			RoadSegment::construct(
				new_road_segment,
				m_Type,
				curvePoints,
				start_snapped_road_segment.elevation_type
			);

			std::array<v3, 4> curve1{
				start_snapped_road_segment.GetCurvePoint(0),
				start_snapped_road_segment.GetCurvePoint(1),
				v3(0.0f),
				curvePoints[0],
			};

			f32 start_snapped_t = start_snapped_road_segment.curve_t_samples[start_snapped_index];
			curve1[1] = curve1[0] + (curve1[1] - curve1[0]) * start_snapped_t;
			curve1[2] = Math::QuadraticCurve(std::array<v3, 3>{
				start_snapped_road_segment.GetCurvePoint(0),
					start_snapped_road_segment.GetCurvePoint(1),
					start_snapped_road_segment.GetCurvePoint(2)
			}, start_snapped_t);

			std::array<v3, 4> curve2 = {
				start_snapped_road_segment.GetCurvePoint(3),
				start_snapped_road_segment.GetCurvePoint(2),
				v3(0.0f),
				curvePoints[0],
			};
			curve2[1] = curve2[1] + (curve2[0] - curve2[1]) * start_snapped_t;
			curve2[2] = Math::QuadraticCurve(std::array<v3, 3>{
				start_snapped_road_segment.GetCurvePoint(1),
					start_snapped_road_segment.GetCurvePoint(2),
					start_snapped_road_segment.GetCurvePoint(3)
			}, start_snapped_t);
			new_road_segment->SetType(start_snapped_road_segment.type);
			new_road_segment->SetCurvePoints(curve2);

			start_snapped_road_segment.SetCurvePoints(curve1);

			RoadNode& endNode = road_nodes[start_snapped_road_segment.EndNode];
			RoadNode& startNode = road_nodes[start_snapped_road_segment.StartNode];

			new_road_segment->EndNode = new_road_node_index;
			new_road_segment->StartNode = start_snapped_road_segment.EndNode;

			endNode.RemoveRoadSegment(m_StartSnappedSegment);
			start_snapped_road_segment.EndNode = new_road_node_index;
			endNode.AddRoadSegment({ new_road_segment_index });

			road_segment->StartNode = new_road_node_index;
			RoadNode::construct(
				new_road_node,
				{ (u64)m_StartSnappedSegment, new_road_segment_index, road_segment_index },
				curvePoints[0],
				start_snapped_road_segment.elevation_type,
				new_road_node_index
			);

			resnapp_buildings(m_StartSnappedSegment, new_road_segment_index, start_snapped_index);
			reassign_people_on_the_road(m_StartSnappedSegment, new_road_segment_index, new_road_node_index, start_snapped_index);
		}
		else
		{
			auto [road_node, road_node_index] = array_add_empty(&road_nodes);
			RoadNode::construct(road_node, {}, curvePoints[0], road_segment->elevation_type, road_node_index);
			road_segment->StartNode = road_node_index;
			road_node->AddRoadSegment({ road_segment_index });
		}

		if (m_EndSnappedNode != -1)
		{
			RoadSegment& road_segment = road_segments[road_segment_index];
			RoadNode& road_node = road_nodes[m_EndSnappedNode];
			road_segment.EndNode = m_EndSnappedNode;
			road_node.AddRoadSegment({ road_segment_index });

			u64 new_index = std::distance(
				road_node.roadSegments.begin(),
				std::find(
					road_node.roadSegments.begin(),
					road_node.roadSegments.end(),
					road_segment_index
				));
			update_path_of_walking_people_if_a_node_is_changed(m_EndSnappedNode, new_index, true);
		}
		else if (m_EndSnappedSegment != -1)
		{
			auto [new_road_segment, new_road_segment_index] = array_add_empty(&road_segments);
			auto [new_road_node, new_road_node_index] = array_add_empty(&road_nodes);
			RoadSegment& end_snapped_road_segment = road_segments[m_EndSnappedSegment];
			RoadSegment::construct(
				new_road_segment,
				m_Type,
				curvePoints,
				end_snapped_road_segment.elevation_type
			);

			f32 end_snapped_t = end_snapped_road_segment.curve_t_samples[end_snapped_index];
			std::array<v3, 4> curve1{
				end_snapped_road_segment.GetCurvePoint(0),
				end_snapped_road_segment.GetCurvePoint(1),
				v3(0.0f),
				curvePoints[3],
			};
			curve1[1] = curve1[0] + (curve1[1] - curve1[0]) * end_snapped_t;
			curve1[2] = Math::QuadraticCurve(std::array<v3, 3>{
				end_snapped_road_segment.GetCurvePoint(0),
					end_snapped_road_segment.GetCurvePoint(1),
					end_snapped_road_segment.GetCurvePoint(2)
			}, end_snapped_t);

			std::array<v3, 4> curve2 = {
				end_snapped_road_segment.GetCurvePoint(3),
				end_snapped_road_segment.GetCurvePoint(2),
				v3(0.0f),
				curvePoints[3],
			};
			curve2[1] = curve2[1] + (curve2[0] - curve2[1]) * end_snapped_t;
			curve2[2] = Math::QuadraticCurve(std::array<v3, 3>{
				end_snapped_road_segment.GetCurvePoint(1),
					end_snapped_road_segment.GetCurvePoint(2),
					end_snapped_road_segment.GetCurvePoint(3)
			}, end_snapped_t);
			new_road_segment->SetType(end_snapped_road_segment.type);
			new_road_segment->SetCurvePoints(curve2);

			end_snapped_road_segment.SetCurvePoints(curve1);

			RoadNode& endNode = road_nodes[end_snapped_road_segment.EndNode];
			RoadNode& startNode = road_nodes[end_snapped_road_segment.StartNode];

			new_road_segment->EndNode = new_road_node_index;
			new_road_segment->StartNode = end_snapped_road_segment.EndNode;

			endNode.RemoveRoadSegment(m_EndSnappedSegment);
			end_snapped_road_segment.EndNode = new_road_node_index;
			endNode.AddRoadSegment({ new_road_segment_index });

			road_segment->EndNode = new_road_node_index;
			RoadNode::construct(
				new_road_node,
				{ (u64)m_EndSnappedSegment, new_road_segment_index, road_segment_index },
				curvePoints[3],
				end_snapped_road_segment.elevation_type,
				new_road_node_index
			);

			resnapp_buildings(m_EndSnappedSegment, new_road_segment_index, end_snapped_index);
			reassign_people_on_the_road(m_EndSnappedSegment, new_road_segment_index, new_road_node_index, end_snapped_index);
		}
		else
		{
			auto [road_node, road_node_index] = array_add_empty(&road_nodes);
			RoadNode::construct(road_node, {}, curvePoints[3], road_segment->elevation_type, road_node_index);
			road_segment->EndNode = road_node_index;
			road_node->AddRoadSegment({ road_segment_index });
		}

		return road_segment_index;
	}
	void RoadManager::RemoveRoadSegment(u64 road_segment_index)
	{
		RoadSegment& road_segment{ road_segments[road_segment_index] };

		auto& buildings { m_Scene->m_BuildingManager.m_Buildings};
		auto& cars { m_Scene->m_CarManager.m_Cars};
		auto& people { m_Scene->m_PersonManager.m_People};
		const auto& people_on_the_road{ m_Scene->m_PersonManager.get_people_on_the_road() };

		if (road_segment.elevation_type == 0)
			Helper::UpdateTheTerrain(&road_segment, true);

		while (road_segment.buildings.size() > 0)
			remove_building(road_segment.buildings[0]);


		while (road_segment.people.size() > 0)
			reset_person_back_to_building_from(road_segment.people[0]);

		for (u64 i = people_on_the_road.size(); i > 0; i--)
		{
			Person* person{ people_on_the_road[i - 1] };
			u64 j{ 0 };
			u64 count{ 0 };
			switch (person->status)
			{
			case PersonStatus::AtHome:
			case PersonStatus::AtWork:
				// Do nothing
				break;
			case PersonStatus::Driving:
			case PersonStatus::DrivingForWork:
			{
				auto& path{ person->car_driving->path };
				count = path.size();
				for (; j < count; j++)
					if (path[j]->road_segment_index == road_segment_index)
						break;
				break;
			}
			case PersonStatus::Walking:
			case PersonStatus::WalkingDead:
			{
				auto& path{ person->path };
				count = path.size();
				if (count % 2 == 0)
					j = 1;

				for (; j < count; j += 2)
					if (((RS_Transition_For_Walking*)path[j])->road_segment_index == road_segment_index)
						break;
				break;
			}
			default:
				assert(false);
				break;
			}
			if (j < count)
				reset_person_back_to_building_from(person);
		}

		RoadNode& start_road_node = road_nodes[road_segment.StartNode];
		auto index = std::distance(start_road_node.roadSegments.begin(), std::find(
			start_road_node.roadSegments.begin(),
			start_road_node.roadSegments.end(),
			road_segment_index
		));
		start_road_node.roadSegments.erase(start_road_node.roadSegments.begin() + index);
		update_path_of_walking_people_if_a_node_is_changed(road_segment.StartNode, index, false);
		if (start_road_node.roadSegments.size() == 0)
		{
			Helper::UpdateTheTerrain(start_road_node.bounding_polygon, true);
			auto& people_on_road_node = start_road_node.people;
			while (people_on_road_node.size() > 0)
				remove_person(people_on_road_node[people_on_road_node.size() - 1]);
			array_remove(&road_nodes, road_segment.StartNode);
		}
		else
		{
			start_road_node.Reconstruct();
		}

		RoadNode& end_road_node = road_nodes[road_segment.EndNode];
		index = std::distance(end_road_node.roadSegments.begin(), std::find(
			end_road_node.roadSegments.begin(),
			end_road_node.roadSegments.end(),
			road_segment_index
		));
		end_road_node.roadSegments.erase(end_road_node.roadSegments.begin() + index);
		update_path_of_walking_people_if_a_node_is_changed(road_segment.EndNode, index, false);
		if (end_road_node.roadSegments.size() == 0)
		{
			Helper::UpdateTheTerrain(end_road_node.bounding_polygon, true);
			auto& people_on_road_node = end_road_node.people;
			while (people_on_road_node.size() > 0)
				remove_person(people_on_road_node[people_on_road_node.size() - 1]);
			array_remove(&road_nodes, road_segment.EndNode);
		}
		else
		{
			end_road_node.Reconstruct();
		}

		array_remove(&road_segments, road_segment_index);
	}

	SnapInformation RoadManager::CheckSnapping(const v3& prevLocation)
	{
		GameApp* app = m_Scene->MainApplication;
		SnapInformation results;
		Road_Type& type = app->road_types[m_Type];

		f32 min_distance_to_snap = -prevLocation.y > type.tunnel_height ? type.tunnel_width : type.road_width;

		results.location = prevLocation;
		u64 capacity = road_nodes.capacity;
		for (u64 i = 0; i < capacity; i++)
		{
			if (road_nodes.values[i].valid == false)
				continue;
			RoadNode& road_node = road_nodes[i];
			v2 distance_vector = (v2)(road_node.position - prevLocation);
			if (glm::length(distance_vector) < min_distance_to_snap)
			{
				results.location = road_node.position;
				results.snapped = true;
				results.node = i;
				results.elevation_type = road_node.elevation_type;
				return results;
			}
		}

		v2 point = (v2)prevLocation;
		capacity = road_segments.capacity;
		auto values = road_segments.values;
		for (u64 rsIndex = 0; rsIndex < capacity; rsIndex++)
		{
			if (values[rsIndex].valid == false)
				continue;
			RoadSegment& segment = values[rsIndex].value;

			if (Math::CheckPolygonPointCollision(segment.bounding_rect, point))
			{
				f32 width = segment.elevation_type == -1 ? app->road_types[segment.type].tunnel_width : app->road_types[segment.type].road_width;
				f32 snapDist = (min_distance_to_snap + width) * 0.5f;
				std::vector<v3> samples = segment.curve_samples;
				u64 curve_samples_size = samples.size();
				v3 point_0 = samples[0];
				for (u64 j = 1; j < curve_samples_size; j++)
				{

					v3 point_1 = samples[j];

					v3 dir_to_p_1 = point_1 - point_0;
					v3 dir_to_prev = prevLocation - point_0;
					f32 scaler = glm::dot(dir_to_p_1, dir_to_prev) / glm::length2(dir_to_p_1);
					if (scaler > 1.0f) {
						point_0 = point_1;
						continue;
					}
					if (scaler < 0.0f) scaler = 0.0f;
					f32 len = glm::length(dir_to_prev);
					f32 lenr = glm::length(dir_to_p_1);
					f32 angle = glm::acos(glm::dot(dir_to_p_1, dir_to_prev) / (lenr * len));
					angle = std::min(1.0f, std::max(-1.0f, angle));
					f32 dist = len * glm::sin(angle);
					if (dist > snapDist)
						break;

					results.location = point_0 + dir_to_p_1 * (scaler * lenr);
					results.segment = rsIndex;
					results.snapped = true;
					results.t = scaler;
					results.t_index = j - 1;
					results.elevation_type = segment.elevation_type;
					return results;
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
		start_snapped_index = (u64)-1;

		m_EndSnappedNode = -1;
		m_EndSnappedSegment = -1;
		end_snapped_index = (u64)-1;

		m_DestructionNode = -1;
		m_DestructionSegment = -1;

		selected_road_segment = -1;
		selected_road_node = -1;

		u64 capacity = road_segments.capacity;
		for (u64 road_node_index = 0; road_node_index < capacity; road_node_index++)
		{
			if (road_segments.values[road_node_index].valid == false)
				continue;
			RoadSegment& road_segment = road_segments[road_node_index];
			road_segment.object->enabled = true;	// Is this needed anymore??
			road_segment.object->SetTransform(road_segment.GetStartPosition());
		}
		capacity = road_nodes.capacity;
		for (u64 road_node_index = 0; road_node_index < capacity; road_node_index++)
		{
			if (road_nodes.values[road_node_index].valid == false)
				continue;
			RoadNode& road_node = road_nodes[road_node_index];
			road_node.object->enabled = true;	// Is this needed anymore??
			road_node.object->SetTransform(road_node.position);
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
				start_snapped_index = snapInformation.t_index;
				if (b_ConstructionStartSnapped)
					m_Elevationtypes[0] = snapInformation.elevation_type;
			}
			else
			{
				b_ConstructionEndSnapped = snapInformation.snapped;
				m_EndSnappedSegment = snapInformation.segment;
				m_EndSnappedNode = snapInformation.node;
				end_snapped_index = snapInformation.t_index;
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
	void RoadManager::SnapToHeight(const std::vector<u8>& indices, u8 index, const v3& AB)
	{
		if (snapFlags & (u8)RoadSnapOptions::SNAP_TO_HEIGHT)
		{
			for (u8 i = 0; i < indices.size(); i++)
				m_ConstructionPositions[indices[i]].z = m_ConstructionPositions[index].z;
		}
	}
	void RoadManager::SnapToAngle(v3& AB, s64 snappedNode, s64 snappedRoadSegment, u64 snapped_index)
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
					RoadNode& road_node = road_nodes[snappedNode];
					f32 minAngle = 180.0f;
					for (u64 rsIndex : road_node.roadSegments)
					{
						RoadSegment& road_segment = road_segments[rsIndex];
						v3 dir = road_segment.StartNode == snappedNode ? road_segment.GetStartDirection() : road_segment.GetEndDirection();

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
					RoadSegment& road_segment = road_segments[snappedRoadSegment];
					v3 tangent = Math::CubicCurveTangent(road_segment.GetCurvePoints(), road_segment.curve_t_samples[snapped_index]);
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
	bool RoadManager::RestrictSmallAngles(v2 direction, s64 snappedNode, s64 snappedRoadSegment, u64 snapped_index)
	{
		if (restrictionFlags & (u8)RoadRestrictions::RESTRICT_SMALL_ANGLES)
		{
			direction = glm::normalize(direction);

			if (snappedNode != -1)
			{
				RoadNode& road_node = road_nodes[snappedNode];
				for (u64 rsIndex : road_node.roadSegments)
				{
					RoadSegment& road_segment = road_segments[rsIndex];

					v2 dirToOldRS = (v2)(road_segment.StartNode == snappedNode ? road_segment.GetStartDirection() : road_segment.GetEndDirection());
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
				RoadSegment& road_segment = road_segments[snappedRoadSegment];
				v2 tangent = (v2)(v3)(Math::CubicCurveTangent(road_segment.GetCurvePoints(), road_segment.curve_t_samples[snapped_index]));
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

	void resnapp_buildings(u64 old_rs_index, u64 new_rs_index, u64 snapped_index)
	{
		auto& road_segments = GameScene::ActiveGameScene->m_RoadManager.road_segments;
		auto& building_types = GameScene::ActiveGameScene->MainApplication->building_types;
		auto& road_types = GameScene::ActiveGameScene->MainApplication->road_types;
		auto& snapped_buildings = road_segments[old_rs_index].buildings;
		for (u64 bIndex = 0; bIndex < snapped_buildings.size(); bIndex++)
		{
			Building* building = snapped_buildings[bIndex];
			f32 offset_from_side_road = -building_types[building->type].prefab->boundingBoxL.x;
			u64 t_index = building->snapped_t_index;
			u64 new_snapped_road_segment_index = t_index < snapped_index ? old_rs_index : new_rs_index;
			RoadSegment& new_snapped_road_segment = road_segments[new_snapped_road_segment_index];

			u64 curve_sample_count = new_snapped_road_segment.curve_samples.size() - 1;
			v3 p_0 = new_snapped_road_segment.curve_samples[0];
			f32 road_half_width = road_types[new_snapped_road_segment.type].road_width * 0.5f;
			for (u64 i = 1; i < curve_sample_count; i++)
			{
				v3 p_1 = new_snapped_road_segment.curve_samples[i];

				v3 dir_to_p_1 = p_1 - p_0;
				v3 dir_to_bulding_from_road_center = building->object->position - p_0;
				v3 dir_to_p_2 = (i < curve_sample_count - 1) ?
					new_snapped_road_segment.curve_samples[i + 1] - p_1 :
					new_snapped_road_segment.GetEndDirection() * -1.0f;

				dir_to_p_1.z = 0.0f;
				dir_to_bulding_from_road_center.z = 0.0f;
				dir_to_p_2.z = 0.0f;

				dir_to_p_1 = glm::normalize(dir_to_p_1);
				dir_to_bulding_from_road_center = glm::normalize(dir_to_bulding_from_road_center);
				dir_to_p_2 = glm::normalize(dir_to_p_2);

				v3 rotated_1 = glm::normalize(v3{ dir_to_p_1.y, -dir_to_p_1.x, 0.0f }) * road_half_width;
				v3 rotated_2 = glm::normalize(v3{ dir_to_p_2.y, -dir_to_p_2.x, 0.0f }) * road_half_width;

				bool snapped_to_right_side = glm::cross(dir_to_p_1, dir_to_bulding_from_road_center).z < 0.0f;
				if (snapped_to_right_side == false)
				{
					rotated_1 *= -1.0f;
					rotated_2 *= -1.0f;
				}

				v3 road_side_end_point_one = p_0 + rotated_1;
				v3 road_side_end_point_two = p_1 + rotated_2;

				v3 dir_side_road = road_side_end_point_two - road_side_end_point_one;
				v3 dir_to_building_from_side_road = building->object->position - road_side_end_point_one;
				f32 scaler = glm::dot(dir_side_road, dir_to_building_from_side_road) / glm::length2(dir_side_road);
				if (scaler > 1.0f) {
					p_0 = p_1;
					if (i < curve_sample_count - 1)
						continue;
					scaler = 1.0f;
				}
				else if (scaler < 0.0f) scaler = 0.0f;

				v3 rotated_3 = glm::normalize(v3{ dir_side_road.y, -dir_side_road.x, 0.0f }) * offset_from_side_road;
				if (snapped_to_right_side == false)
					rotated_3 *= -1.0f;
				v3 new_position = road_side_end_point_one + dir_side_road * scaler + rotated_3;
				f32 rotation_offset = (f32)(dir_side_road.x < 0.0f) * glm::radians(180.0f);
				f32 rotation = glm::atan(dir_side_road.y / dir_side_road.x) + rotation_offset;
				if (t_index >= snapped_index)
				{
					// TODO: instead reset people targeting this building
					for (Person* p : building->people)
						if (p->status == PersonStatus::Walking)
							reset_person_back_to_building_from(p);
					auto it = std::find(snapped_buildings.begin(), snapped_buildings.end(), building);
					assert(it != snapped_buildings.end());
					snapped_buildings.erase(it);
					bIndex--;
					new_snapped_road_segment.buildings.push_back(building);
				}
				building->connected_road_segment = new_snapped_road_segment_index;
				building->snapped_t = scaler;
				building->snapped_t_index = i;
				building->object->position = new_position;
				building->object->SetTransform(
					new_position,
					v3{
						0.0f,
						0.0f,
						((f32)snapped_to_right_side - 1.0f) * glm::radians(180.0f) + glm::radians(-90.0f) + rotation
					});
				break;
			}
		}
	}

	void reassign_people_on_the_road(u64 old_rs_index, u64 new_rs_index, u64 new_rn_index, u64 snapped_index)
	{
		auto& road_segments = GameScene::ActiveGameScene->m_RoadManager.road_segments;
		auto& road_nodes = GameScene::ActiveGameScene->m_RoadManager.road_nodes;
		RoadSegment& old_road_segment = road_segments[old_rs_index];
		RoadSegment& new_road_segment = road_segments[new_rs_index];
		RoadNode& new_road_node = road_nodes[new_rn_index];
		for (u64 person_index = 0; person_index < old_road_segment.people.size(); person_index++)
		{
			Person* person = old_road_segment.people[person_index];
			if (person->status == PersonStatus::Walking)
			{
				auto& path = person->path;
				u64 transition_index = 1;
				if (path.size() % 2 == 1)
				{
					auto rs_transition = (RS_Transition_For_Walking*)path[0];
					if (rs_transition->road_segment_index == old_rs_index)
					{
						reset_person_back_to_building_from(person);
						person_index--;
						continue;
					}
					else
					{
						transition_index = 2;
					}
				}
				for (; transition_index < path.size(); transition_index += 2)
				{
					auto rs_transition = (RS_Transition_For_Walking*)path[transition_index];
					if (rs_transition->road_segment_index != old_rs_index) continue;
					auto rn_transition = (RN_Transition_For_Walking*)path[transition_index - 1];

					if (old_road_segment.StartNode == rn_transition->road_node_index)
					{
						RN_Transition_For_Walking* new_rn_transition = new RN_Transition_For_Walking();
						new_rn_transition->from_road_segments_array_index = std::distance(
							new_road_node.roadSegments.begin(),
							std::find(
								new_road_node.roadSegments.begin(),
								new_road_node.roadSegments.end(),
								old_rs_index
							));
						new_rn_transition->to_road_segments_array_index = std::distance(
							new_road_node.roadSegments.begin(),
							std::find(
								new_road_node.roadSegments.begin(),
								new_road_node.roadSegments.end(),
								new_rs_index
							));
						new_rn_transition->sub_index = rn_transition->sub_index;
						new_rn_transition->road_node_index = new_rn_index;
						f32 diff = new_rn_transition->to_road_segments_array_index - new_rn_transition->from_road_segments_array_index;
						if (diff > 0.0f)
							new_rn_transition->accending = diff < (3.0f / 2.0f);
						else
							new_rn_transition->accending = diff * -1.0f > (3.0f / 2.0f);
						RS_Transition_For_Walking* new_rs_transition = new RS_Transition_For_Walking();
						new_rs_transition->from_start = false;
						new_rs_transition->road_segment_index = new_rs_index;
						new_rs_transition->from_right = rs_transition->from_right;
						path.insert(path.begin() + (transition_index + 1), (Transition*)new_rn_transition);
						path.insert(path.begin() + (transition_index + 2), (Transition*)new_rs_transition);
					}
					else
					{
						RS_Transition_For_Walking* new_rs_transition = new RS_Transition_For_Walking();
						new_rs_transition->from_start = true;
						new_rs_transition->road_segment_index = new_rs_index;
						new_rs_transition->from_right = rs_transition->from_right;
						RN_Transition_For_Walking* new_rn_transition = new RN_Transition_For_Walking();
						new_rn_transition->from_road_segments_array_index = std::distance(
							new_road_node.roadSegments.begin(),
							std::find(
								new_road_node.roadSegments.begin(),
								new_road_node.roadSegments.end(),
								new_rs_index
							));
						new_rn_transition->to_road_segments_array_index = std::distance(
							new_road_node.roadSegments.begin(),
							std::find(
								new_road_node.roadSegments.begin(),
								new_road_node.roadSegments.end(),
								old_rs_index
							));
						new_rn_transition->sub_index = rs_transition->from_right ? 1 : 2;
						new_rn_transition->road_node_index = new_rn_index;
						f32 diff = new_rn_transition->to_road_segments_array_index - new_rn_transition->from_road_segments_array_index;
						if (diff > 0.0f)
							new_rn_transition->accending = diff < (3.0f / 2.0f);
						else
							new_rn_transition->accending = diff * -1.0f > (3.0f / 2.0f);
						path.insert(path.begin() + (transition_index + 0), (Transition*)new_rs_transition);
						path.insert(path.begin() + (transition_index + 1), (Transition*)new_rn_transition);
						assert(false); // is this correct
					}
					transition_index += 2;
				}
			}
			else if (person->status == PersonStatus::Driving)
			{
				auto& path = person->car_driving->path;
				for (u64 index = 1; index < path.size(); index++)
				{
					auto transition = path[index];
					if (transition->road_segment_index != old_rs_index) continue;

					if (old_road_segment.StartNode == transition->next_road_node_index)
					{
						auto new_transition = new RS_Transition_For_Vehicle();
						new_transition->road_segment_index = new_rs_index;
						new_transition->next_road_node_index = new_rn_index;
						assert(false); // calculate lane index for new_rs_transition and points_stack
						path.insert(path.begin() + index, new_transition);
					}
					else
					{
						auto new_transition = new RS_Transition_For_Vehicle();
						new_transition->road_segment_index = new_rs_index;
						new_transition->next_road_node_index = new_road_segment.EndNode;
						new_transition->lane_index = transition->lane_index;
						path.insert(path.begin() + (index + 1), new_transition);

						transition->next_road_node_index = old_road_segment.EndNode;
						assert(false); // calculate lane index for rs_transition and points_stack
					}
					index++;
				}
			}
		}
	}
	void update_path_of_walking_people_if_a_node_is_changed(u64 road_node_index, u64 modified_index, bool isAdded)
	{
		u64 addition = isAdded ? +1 : -1;
		const auto& people_on_the_road = GameScene::ActiveGameScene->m_PersonManager.get_people_on_the_road();
		for (Person* person: people_on_the_road)
		{
			if (person->status != PersonStatus::Walking) continue;

			auto& path = person->path;
			u64 transition_index = 1;
			if (path.size() % 2 == 0)
			{
				auto rn_transition = (RN_Transition_For_Walking*)path[0];
				if (rn_transition->road_node_index == road_node_index)
					if (rn_transition->to_road_segments_array_index >= modified_index)
						rn_transition->to_road_segments_array_index += addition;
				transition_index = 2;
			}
			for (; transition_index < path.size(); transition_index += 2)
			{
				auto rn_transition = (RN_Transition_For_Walking*)path[transition_index];
				if (rn_transition->road_node_index == road_node_index)
				{
					if (rn_transition->from_road_segments_array_index >= modified_index)
						rn_transition->from_road_segments_array_index += addition;
					if (rn_transition->to_road_segments_array_index >= modified_index)
						rn_transition->to_road_segments_array_index += addition;
				}
			}
		}
	}
}