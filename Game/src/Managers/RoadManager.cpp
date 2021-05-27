#include "canpch.h"
#include "RoadManager.h"

#include "Types/RoadSegment.h"
#include "Types/RoadNode.h"
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
		m_GuidelinesStart = new Object(type.asymmetric ? type.end_mirror : type.end);
		m_GuidelinesStart->enabled = false;
		m_GuidelinesEnd = new Object(type.end);
		m_GuidelinesEnd->enabled = false;

		u64 roadTypeCount = m_Scene->MainApplication->road_types.size();
		for (u64 i = 0; i < roadTypeCount; i++)
		{
			m_GuidelinesInUse.push_back(0);
			m_Guidelines.push_back({});
			const RoadType& t = m_Scene->MainApplication->road_types[i];
			m_Guidelines[i].push_back(new Object(t.road));
			m_Guidelines[i][0]->enabled = false;
		}
	}
	RoadManager::~RoadManager()
	{
	}

	void RoadManager::OnUpdate(v3& prevLocation, const v3& cameraPosition, const v3& cameraDirection)
	{
		switch (m_ConstructionMode)
		{
		case RoadConstructionMode::None:
			break;
		case RoadConstructionMode::Straight:
			OnUpdate_Straight(prevLocation, cameraPosition, cameraDirection);
			break;
		case RoadConstructionMode::QuadraticCurve:
			OnUpdate_QuadraticCurve(prevLocation, cameraPosition, cameraDirection);
			break;
		case RoadConstructionMode::CubicCurve:
			OnUpdate_CubicCurve(prevLocation, cameraPosition, cameraDirection);
			break;
		case  RoadConstructionMode::Change:
			OnUpdate_Change(prevLocation, cameraPosition, cameraDirection);
			break;
		case  RoadConstructionMode::Destruct:
			OnUpdate_Destruction(prevLocation, cameraPosition, cameraDirection);
			break;
		}
	}
	void RoadManager::OnUpdate_Straight(v3& prevLocation, const v3& cameraPosition, const v3& cameraDirection)
	{
		Prefab* selectedRoad = m_Scene->MainApplication->road_types[m_Type].road;
		f32 roadPrefabWidth = selectedRoad->boundingBoxM.z - selectedRoad->boundingBoxL.z;
		f32 roadPrefabLength = selectedRoad->boundingBoxM.x - selectedRoad->boundingBoxL.x;

		if (m_ConstructionPhase == 0)
		{
			SnapToGrid(prevLocation);
			SnapToRoad(prevLocation, true);

			m_ConstructionPositions[0] = prevLocation;

			m_GuidelinesStart->SetTransform(prevLocation + v3{ 0.0f, 0.15f, 0.0f }, v3{ 0.0f, glm::radians(180.0f), 0.0f });
			m_GuidelinesEnd->SetTransform(prevLocation + v3{ 0.0f, 0.15f, 0.0f }, v3(0.0f));
		}
		else
		{
			ResetGuideLines();

			b_ConstructionRestricted = false;
			SnapToGrid(prevLocation);
			SnapToRoad(prevLocation, false);

			m_ConstructionPositions[3] = prevLocation;

			// TODO: After angle snapping 
			v3 dir = prevLocation - m_ConstructionPositions[0];
			bool angleIsRestricted = RestrictSmallAngles(dir, m_StartSnappedNode, m_StartSnappedSegment, m_StartSnappedT);
			angleIsRestricted |= RestrictSmallAngles(-dir, m_EndSnappedNode, m_EndSnappedSegment, m_EndSnappedT);

			v3 AB = m_ConstructionPositions[3] - m_ConstructionPositions[0];

			f32 rotOffset = (f32)(AB.x < 0.0f) * glm::radians(180.0f);
			f32 rotEnd = glm::atan(-AB.z / AB.x) + rotOffset;
			f32 rotStart = rotEnd + glm::radians(180.0f);


			if (!b_ConstructionEndSnapped)
			{
				f32 length = glm::length(AB);
				if ((snapFlags & SNAP_TO_LENGTH) && (length > 0.5f))
				{
					length = length - std::fmod(length, roadPrefabLength);
					AB = length * glm::normalize(AB);
					m_ConstructionPositions[3] = m_ConstructionPositions[0] + AB;
				}
				SnapToHeight({ 3 }, 0, AB);
				SnapToAngle(AB, m_StartSnappedNode, m_StartSnappedSegment, m_StartSnappedT);
				m_ConstructionPositions[3] = m_ConstructionPositions[0] + AB;
			}

			v2 A = v2{ m_ConstructionPositions[0].x, m_ConstructionPositions[0].z };
			v2 D = v2{ m_ConstructionPositions[3].x, m_ConstructionPositions[3].z };
			v2 AD = roadPrefabWidth * 0.5f * glm::normalize(D - A);

			if (!b_ConstructionStartSnapped) A -= AD;
			if (!b_ConstructionStartSnapped) D += AD;

			AD = v2{ -AD.y , AD.x };

			v2 P1 = A + AD;
			v2 P2 = A - AD;
			v2 P3 = D + AD;
			v2 P4 = D - AD;

			std::array<std::array<v2, 3>, 2> newRoadPolygon = {
					std::array<v2,3>{ P1, P2, P3},
					std::array<v2,3>{ P2, P3, P4}
			};

			bool lengthIsRestricted = (restrictionFlags & RESTRICT_SHORT_LENGTH) && (glm::length(AB) < (2.0f * roadPrefabLength));
			bool collisionIsRestricted = (restrictionFlags & RESTRICT_COLLISIONS) ? CheckStraightRoadRoadCollision(newRoadPolygon) : false;

			if (m_Scene->m_BuildingManager.restrictions[0] && (restrictionFlags & RESTRICT_COLLISIONS))
				CheckStraightRoadBuildingCollision(newRoadPolygon);
			if (m_Scene->m_TreeManager.restrictions[0] && (restrictionFlags & RESTRICT_COLLISIONS))
				CheckStraightRoadTreeCollision(newRoadPolygon);

			b_ConstructionRestricted |= angleIsRestricted;
			b_ConstructionRestricted |= lengthIsRestricted;
			b_ConstructionRestricted |= collisionIsRestricted;

			DrawStraightGuidelines(m_ConstructionPositions[0], m_ConstructionPositions[3]);
		}
	}
	void RoadManager::OnUpdate_QuadraticCurve(v3& prevLocation, const v3& cameraPosition, const v3& cameraDirection)
	{
		Prefab* selectedRoad = m_Scene->MainApplication->road_types[m_Type].road;
		f32 roadPrefabWidth = selectedRoad->boundingBoxM.z - selectedRoad->boundingBoxL.z;
		f32 roadPrefabLength = selectedRoad->boundingBoxM.x - selectedRoad->boundingBoxL.x;

		ResetGuideLines();

		if (m_ConstructionPhase == 0)
		{
			SnapToGrid(prevLocation);
			SnapToRoad(prevLocation, true);

			m_ConstructionPositions[0] = prevLocation;
			m_ConstructionPositions[1] = prevLocation;
			m_ConstructionPositions[2] = prevLocation;
			m_ConstructionPositions[3] = prevLocation;

			m_GuidelinesStart->SetTransform(prevLocation + v3{ 0.0f, 0.15f, 0.0f }, { 0.0f, glm::radians(180.0f), 0.0f });
			m_GuidelinesEnd->SetTransform(prevLocation + v3{ 0.0f, 0.15f, 0.0f }, v3(0.0f));
		}
		else if (m_ConstructionPhase == 1)
		{
			b_ConstructionRestricted = false;
			SnapToGrid(prevLocation);

			m_ConstructionPositions[1] = prevLocation;
			m_ConstructionPositions[2] = prevLocation;
			m_ConstructionPositions[3] = prevLocation;

			// TODO: After angle snapping
			v3 dir = prevLocation - m_ConstructionPositions[0];
			bool angleIsRestricted = RestrictSmallAngles(dir, m_StartSnappedNode, m_StartSnappedSegment, m_StartSnappedT);

			v3 AB = m_ConstructionPositions[3] - m_ConstructionPositions[0];

			f32 rotOffset = (f32)(AB.x < 0.0f) * glm::radians(180.0f);
			f32 rotEnd = glm::atan(-AB.z / AB.x) + rotOffset;
			f32 rotStart = rotEnd + glm::radians(180.0f);

			SnapToHeight({ 1, 2, 3 }, 0, AB);
			SnapToAngle(AB, m_StartSnappedNode, m_StartSnappedSegment, m_StartSnappedT);
			m_ConstructionPositions[1] = m_ConstructionPositions[0] + AB;
			m_ConstructionPositions[2] = m_ConstructionPositions[0] + AB;
			m_ConstructionPositions[3] = m_ConstructionPositions[0] + AB;

			v2 A = v2{ m_ConstructionPositions[0].x, m_ConstructionPositions[0].z };
			v2 D = v2{ m_ConstructionPositions[3].x, m_ConstructionPositions[3].z };
			v2 AD = roadPrefabWidth * 0.5f * glm::normalize(D - A);

			if (!b_ConstructionStartSnapped) A -= AD;
			D += AD;

			AD = v2{ -AD.y , AD.x };

			v2 P1 = A + AD;
			v2 P2 = A - AD;
			v2 P3 = D + AD;
			v2 P4 = D - AD;

			std::array<std::array<v2, 3>, 2> newRoadPolygon = {
					std::array<v2,3>{ P1, P2, P3},
					std::array<v2,3>{ P2, P3, P4}
			};

			bool collisionIsRestricted = (restrictionFlags & RESTRICT_COLLISIONS) ? CheckStraightRoadRoadCollision(newRoadPolygon) : false;

			if (m_Scene->m_BuildingManager.restrictions[0] && (restrictionFlags & RESTRICT_COLLISIONS))
				CheckStraightRoadBuildingCollision(newRoadPolygon);
			if (m_Scene->m_TreeManager.restrictions[0] && (restrictionFlags & RESTRICT_COLLISIONS))
				CheckStraightRoadTreeCollision(newRoadPolygon);

			b_ConstructionRestricted |= angleIsRestricted;
			b_ConstructionRestricted |= collisionIsRestricted;
			DrawStraightGuidelines(m_ConstructionPositions[0], m_ConstructionPositions[3]);
		}
		else if (m_ConstructionPhase == 2)
		{
			b_ConstructionRestricted = false;
			SnapToGrid(prevLocation);
			SnapToRoad(prevLocation, false);

			m_ConstructionPositions[3] = prevLocation;

			if (!b_ConstructionEndSnapped)
				SnapToHeight({ 3 }, 0, v3(0.0f));
			// SnapToAngle For center angle
			/***Magic***/ {
				v2 Cd{ m_ConstructionPositions[1].x, m_ConstructionPositions[1].z };
				v2 A{ m_ConstructionPositions[0].x, m_ConstructionPositions[0].z };
				v2 B{ m_ConstructionPositions[3].x, m_ConstructionPositions[3].z };
				v2 ray = glm::normalize(Cd - A);
				v2 AB = B - A;
				f32 d = glm::dot(AB, AB) / (2.0f * glm::dot(AB, ray));
				v2 C = A + d * ray;
				if (d < 200.0f && d > 0.0f)
				{
					m_ConstructionPositions[2].x = C.x;
					m_ConstructionPositions[2].z = C.y;
				}
			}
			// TODO: After angle snapping
			v3 dir1 = m_ConstructionPositions[1] - m_ConstructionPositions[0];
			bool angleIsRestricted = RestrictSmallAngles(dir1, m_StartSnappedNode, m_StartSnappedSegment, m_StartSnappedT);
			v3 dir2 = m_ConstructionPositions[2] - m_ConstructionPositions[3];
			angleIsRestricted |= RestrictSmallAngles(dir2, m_EndSnappedNode, m_EndSnappedSegment, m_EndSnappedT);
			if (restrictionFlags & RESTRICT_SMALL_ANGLES)
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
			std::array<std::array<v2, 3>, 2> newRoadBoundingBox = Math::GetBoundingBoxOfBezierCurve(cps, roadPrefabWidth * 0.5f);
			std::array<std::array<v2, 3>, (10 - 1) * 2> newRoadBoundingPolygon = Math::GetBoundingPolygonOfBezierCurve<10, 10>(cps, roadPrefabWidth * 0.5f);

			bool lengthIsRestricted = (restrictionFlags & RESTRICT_SHORT_LENGTH) && (glm::length(cps[0] - cps[3]) < (2.0f * roadPrefabLength));
			bool collisionIsRestricted = (restrictionFlags & RESTRICT_COLLISIONS) ? CheckRoadRoadCollision(newRoadBoundingBox, newRoadBoundingPolygon) : false;

			if (m_Scene->m_BuildingManager.restrictions[0] && (restrictionFlags & RESTRICT_COLLISIONS))
				CheckRoadBuildingCollision(newRoadBoundingBox, newRoadBoundingPolygon);
			if (m_Scene->m_TreeManager.restrictions[0] && (restrictionFlags & RESTRICT_COLLISIONS))
				CheckRoadTreeCollision(newRoadBoundingBox, newRoadBoundingPolygon);

			b_ConstructionRestricted |= angleIsRestricted;
			b_ConstructionRestricted |= lengthIsRestricted;
			b_ConstructionRestricted |= collisionIsRestricted;
			DrawCurvedGuidelines(cps);
		}
	}
	void RoadManager::OnUpdate_CubicCurve(v3& prevLocation, const v3& cameraPosition, const v3& cameraDirection)
	{
		Prefab* selectedRoad = m_Scene->MainApplication->road_types[m_Type].road;
		f32 roadPrefabWidth = selectedRoad->boundingBoxM.z - selectedRoad->boundingBoxL.z;
		f32 roadPrefabLength = selectedRoad->boundingBoxM.x - selectedRoad->boundingBoxL.x;

		ResetGuideLines();

		if (m_ConstructionPhase == 0)
		{
			SnapToGrid(prevLocation);
			SnapToRoad(prevLocation, true);

			m_ConstructionPositions[0] = prevLocation;

			m_GuidelinesStart->SetTransform(prevLocation + v3{ 0.0f, 0.15f, 0.0f }, v3{ 0.0f, glm::radians(180.0f), 0.0f });
			m_GuidelinesEnd->SetTransform(prevLocation + v3{ 0.0f, 0.15f, 0.0f }, v3(0.0f));
		}
		else if (m_ConstructionPhase == 1)
		{
			b_ConstructionRestricted = false;
			SnapToGrid(prevLocation);
			if (cubicCurveOrder[1] == 3)
				SnapToRoad(prevLocation, false);

			m_ConstructionPositions[cubicCurveOrder[1]] = prevLocation;

			// TODO: After angle snapping
			bool angleIsRestricted = false;
			if (cubicCurveOrder[1] == 1)
			{
				v3 dir = m_ConstructionPositions[1] - m_ConstructionPositions[0];
				angleIsRestricted = RestrictSmallAngles(dir, m_StartSnappedNode, m_StartSnappedSegment, m_StartSnappedT);
			}

			v3 AB = prevLocation - m_ConstructionPositions[0];

			f32 rotOffset = (f32)(AB.x < 0.0f) * glm::radians(180.0f);
			f32 rotEnd = glm::atan(-AB.z / AB.x) + rotOffset;
			f32 rotStart = rotEnd + glm::radians(180.0f);

			if ((snapFlags & SNAP_TO_LENGTH) && (cubicCurveOrder[1] == 1) && (glm::length(AB) > 0.5f))
			{
				f32 length = glm::length(AB);
				length = length - std::fmod(length, roadPrefabLength);
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

			v2 A = v2{ m_ConstructionPositions[cubicCurveOrder[0]].x, m_ConstructionPositions[cubicCurveOrder[0]].z };
			v2 D = v2{ m_ConstructionPositions[cubicCurveOrder[1]].x, m_ConstructionPositions[cubicCurveOrder[1]].z };
			v2 AD = roadPrefabWidth * 0.5f * glm::normalize(D - A);

			if (!b_ConstructionStartSnapped) A -= AD;
			D += AD;

			AD = v2{ -AD.y , AD.x };

			v2 P1 = A + AD;
			v2 P2 = A - AD;
			v2 P3 = D + AD;
			v2 P4 = D - AD;

			std::array<std::array<v2, 3>, 2> newRoadPolygon = {
					std::array<v2,3>{ P1, P2, P3},
					std::array<v2,3>{ P2, P3, P4}
			};

			bool lengthIsRestricted = (restrictionFlags & RESTRICT_SHORT_LENGTH) && glm::length(AB) < 2.0f * roadPrefabLength;
			bool collisionIsRestricted = (restrictionFlags & RESTRICT_COLLISIONS) ? CheckStraightRoadRoadCollision(newRoadPolygon) : false;

			if (m_Scene->m_BuildingManager.restrictions[0] && (restrictionFlags & RESTRICT_COLLISIONS))
				CheckStraightRoadBuildingCollision(newRoadPolygon);
			if (m_Scene->m_TreeManager.restrictions[0] && (restrictionFlags & RESTRICT_COLLISIONS))
				CheckStraightRoadTreeCollision(newRoadPolygon);

			b_ConstructionRestricted |= angleIsRestricted;
			b_ConstructionRestricted |= lengthIsRestricted;
			b_ConstructionRestricted |= collisionIsRestricted;
			DrawStraightGuidelines(m_ConstructionPositions[0], m_ConstructionPositions[cubicCurveOrder[1]]);

		}
		else if (m_ConstructionPhase == 2)
		{
			b_ConstructionRestricted = false;
			SnapToGrid(prevLocation);
			if (cubicCurveOrder[2] == 3)
				SnapToRoad(prevLocation, false);

			m_ConstructionPositions[cubicCurveOrder[2]] = prevLocation;
			m_ConstructionPositions[cubicCurveOrder[3]] = prevLocation;

			// TODO: After angle snapping
			bool angleIsRestricted = false;
			if (cubicCurveOrder[3] == 1)
			{
				v3 dir = m_ConstructionPositions[2] - m_ConstructionPositions[3];
				bool angleIsRestricted = RestrictSmallAngles(dir, m_EndSnappedNode, m_EndSnappedSegment, m_EndSnappedT);
			}
			else
			{
				v3 dir = m_ConstructionPositions[1] - m_ConstructionPositions[0];
				bool angleIsRestricted = RestrictSmallAngles(dir, m_StartSnappedNode, m_StartSnappedSegment, m_StartSnappedT);
			}
			if (snapFlags & SNAP_TO_LENGTH)
			{
				v3 AB1 = m_ConstructionPositions[1] - m_ConstructionPositions[0];
				v3 AB2 = m_ConstructionPositions[2] - m_ConstructionPositions[3];

				f32 length1 = glm::length(AB1);
				f32 length2 = glm::length(AB2);

				if (cubicCurveOrder[2] == 1 && length1 > 0.1f)
				{
					length1 = length1 - std::fmod(length1, roadPrefabLength);
					AB1 = length1 * glm::normalize(AB1);

					m_ConstructionPositions[1] = m_ConstructionPositions[0] + AB1;
				}
				else if (cubicCurveOrder[2] == 2 && length2 > 0.1f)
				{
					length2 = length2 - std::fmod(length2, roadPrefabLength);
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

			std::array<std::array<v2, 3>, 2> newRoadBoundingBox = Math::GetBoundingBoxOfBezierCurve(cps, roadPrefabWidth * 0.5f);
			std::array<std::array<v2, 3>, (10 - 1) * 2> newRoadBoundingPolygon = Math::GetBoundingPolygonOfBezierCurve<10, 10>(cps, roadPrefabWidth * 0.5f);

			bool lengthIsRestricted = (restrictionFlags & RESTRICT_SHORT_LENGTH) &&
				(((cubicCurveOrder[3] == 1) && (glm::length(cps[2] - cps[3]) < 2.0f * roadPrefabLength)) ||
					((cubicCurveOrder[3] != 1) && (glm::length(cps[1] - cps[0]) < 2.0f * roadPrefabLength)));
			bool collisionIsRestricted = (restrictionFlags & RESTRICT_COLLISIONS) ? CheckRoadRoadCollision(newRoadBoundingBox, newRoadBoundingPolygon) : false;

			if (m_Scene->m_BuildingManager.restrictions[0] && (restrictionFlags & RESTRICT_COLLISIONS))
				CheckRoadBuildingCollision(newRoadBoundingBox, newRoadBoundingPolygon);
			if (m_Scene->m_TreeManager.restrictions[0] && (restrictionFlags & RESTRICT_COLLISIONS))
				CheckRoadTreeCollision(newRoadBoundingBox, newRoadBoundingPolygon);

			b_ConstructionRestricted |= angleIsRestricted;
			b_ConstructionRestricted |= lengthIsRestricted;
			b_ConstructionRestricted |= collisionIsRestricted;

			DrawCurvedGuidelines(cps);
		}
		else if (m_ConstructionPhase == 3)
		{
			b_ConstructionRestricted = false;
			SnapToGrid(prevLocation);
			if (cubicCurveOrder[3] == 3)
				SnapToRoad(prevLocation, false);

			m_ConstructionPositions[cubicCurveOrder[3]] = prevLocation;

			// TODO: After angle snapping
			v3 dir1 = m_ConstructionPositions[1] - m_ConstructionPositions[0];
			bool angleIsRestricted = RestrictSmallAngles(dir1, m_StartSnappedNode, m_StartSnappedSegment, m_StartSnappedT);
			v3 dir2 = m_ConstructionPositions[2] - m_ConstructionPositions[3];
			angleIsRestricted |= RestrictSmallAngles(dir2, m_EndSnappedNode, m_EndSnappedSegment, m_EndSnappedT);

			if (snapFlags & SNAP_TO_LENGTH)
			{
				v3 AB1 = m_ConstructionPositions[1] - m_ConstructionPositions[0];
				v3 AB2 = m_ConstructionPositions[2] - m_ConstructionPositions[3];

				f32 length1 = glm::length(AB1);
				f32 length2 = glm::length(AB2);

				if (cubicCurveOrder[3] == 1 && length1 > 0.1f)
				{
					length1 = length1 - std::fmod(length1, roadPrefabLength);
					AB1 = length1 * glm::normalize(AB1);

					m_ConstructionPositions[1] = m_ConstructionPositions[0] + AB1;
				}
				else if (cubicCurveOrder[3] == 2 && length2 > 0.1f)
				{
					length2 = length2 - std::fmod(length2, roadPrefabLength);
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

			std::array<std::array<v2, 3>, 2> newRoadBoundingBox = Math::GetBoundingBoxOfBezierCurve(m_ConstructionPositions, roadPrefabWidth * 0.5f);
			std::array<std::array<v2, 3>, (10 - 1) * 2> newRoadBoundingPolygon = Math::GetBoundingPolygonOfBezierCurve<10, 10>(m_ConstructionPositions, roadPrefabWidth * 0.5f);

			bool lengthIsRestricted =
				(restrictionFlags & RESTRICT_SHORT_LENGTH) &&
				(glm::length(m_ConstructionPositions[0] - m_ConstructionPositions[1]) < 2.0f * roadPrefabLength) &&
				(glm::length(m_ConstructionPositions[3] - m_ConstructionPositions[2]) < 2.0f * roadPrefabLength);
			bool collisionIsRestricted = (restrictionFlags & RESTRICT_COLLISIONS) ? CheckRoadRoadCollision(newRoadBoundingBox, newRoadBoundingPolygon) : false;

			if (m_Scene->m_BuildingManager.restrictions[0] && (restrictionFlags & RESTRICT_COLLISIONS))
				CheckRoadBuildingCollision(newRoadBoundingBox, newRoadBoundingPolygon);
			if (m_Scene->m_TreeManager.restrictions[0] && (restrictionFlags & RESTRICT_COLLISIONS))
				CheckRoadTreeCollision(newRoadBoundingBox, newRoadBoundingPolygon);


			b_ConstructionRestricted |= angleIsRestricted;
			b_ConstructionRestricted |= lengthIsRestricted;
			b_ConstructionRestricted |= collisionIsRestricted;

			DrawCurvedGuidelines(m_ConstructionPositions);
		}
	}
	void RoadManager::OnUpdate_Change(v3& prevLocation, const v3& cameraPosition, const v3& cameraDirection)
	{
		if (selected_road_segment != -1)
			m_Segments[selected_road_segment].object->SetTransform(m_Segments[selected_road_segment].GetStartPosition());

		v2 prevLoc2D{ prevLocation.x, prevLocation.z };
		Prefab* roadType = m_Scene->MainApplication->road_types[m_Type].road;
		u64 size = m_Segments.size();
		for (u64 rsIndex = 0; rsIndex < size; rsIndex++)
		{
			RoadSegment& rs = m_Segments[rsIndex];
			if (rs.type.road == roadType)
				continue;

			f32 rsl = rs.type.road_length;
			f32 snapDist = rs.type.road_width * 0.5f;
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
					dirToP1.y = 0.0f;
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
							rs.object->SetTransform(rs.GetStartPosition() + v3{ 0.0f, 0.1f, 0.0f });
							return;
						}
					}
					point0 = point1;
				}
			}
		}
		selected_road_segment = -1;
	}
	void RoadManager::OnUpdate_Destruction(v3& prevLocation, const v3& cameraPosition, const v3& cameraDirection)
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

					rs.object->SetTransform(rs.GetStartPosition() + v3{ 0.0f, 0.1f, 0.0f });

					if (rs.StartNode == m_DestructionNode)
					{
						if (endNode.roadSegments.size() == 1)
							endNode.object->SetTransform(endNode.position + v3{ 0.0f, 0.1f, 0.0f });
					}
					else
					{
						if (startNode.roadSegments.size() == 1)
							startNode.object->SetTransform(startNode.position + v3{ 0.0f, 0.1f, 0.0f });
					}
				}
			}
			else if (m_DestructionSegment != -1)
			{
				RoadSegment& rs = m_Segments[m_DestructionSegment];
				RoadNode& startNode = m_Nodes[rs.StartNode];
				RoadNode& endNode = m_Nodes[rs.EndNode];

				rs.object->SetTransform(rs.GetStartPosition() + v3{ 0.0f, 0.1f, 0.0f });

				if (endNode.roadSegments.size() == 1)
					endNode.object->SetTransform(endNode.position + v3{ 0.0f, 0.1f, 0.0f });
				if (startNode.roadSegments.size() == 1)
					startNode.object->SetTransform(startNode.position + v3{ 0.0f, 0.1f, 0.0f });
			}
		}
	}

	void RoadManager::DrawStraightGuidelines(const v3& pointA, const v3& pointB)
	{
		Prefab* selectedRoad = m_Scene->MainApplication->road_types[m_Type].road;
		f32 roadPrefabLength = selectedRoad->boundingBoxM.x - selectedRoad->boundingBoxL.x;

		v3 AB = pointB - pointA;
		v3 normalizedAB = glm::normalize(AB);

		f32 rotationOffset = AB.x < 0.0f ? glm::radians(180.0f) : 0.0f;
		f32 rotationStart = glm::atan(-AB.z / AB.x) + glm::radians(180.0f) + rotationOffset;
		f32 rotationEnd = glm::atan(-AB.z / AB.x) + rotationOffset;

		f32 availableABLength = (
			glm::length(AB)
			- (b_ConstructionStartSnapped ? roadPrefabLength : 0.0f)
			- (b_ConstructionEndSnapped ? roadPrefabLength : 0.0f)
			);
		availableABLength = std::max(availableABLength, 0.0f);

		int countAB = (int)(availableABLength / roadPrefabLength);
		f32 scaleAB = (availableABLength / roadPrefabLength) / countAB;
		f32 scaledRoadLength = availableABLength / countAB;

		bool lengthIsRestricted = (restrictionFlags & RESTRICT_SHORT_LENGTH) && countAB < 1;

		int discountStart = (b_ConstructionStartSnapped ? 1 : 0);

		m_GuidelinesInUse[m_Type] += countAB;
		if (m_GuidelinesInUse[m_Type] > m_Guidelines[m_Type].size())
			for (u64 j = m_Guidelines[m_Type].size(); j < m_GuidelinesInUse[m_Type]; j++)
				m_Guidelines[m_Type].push_back(new Object(m_Scene->MainApplication->road_types[m_Type].road));

		for (u64 j = 0; j < countAB; j++)
		{
			Object* roadG = m_Guidelines[m_Type][j];
			roadG->enabled = true;
			roadG->SetTransform(
				pointA + (normalizedAB * ((j + discountStart) * scaledRoadLength)) + v3{ 0.0f, 0.15f, 0.0f },
				v3{ 0.0f, rotationEnd, 0.0f },
				v3{ 1.0f * scaleAB, 1.0f, 1.0f }
			);
		}

		b_ConstructionRestricted |= lengthIsRestricted;

		m_GuidelinesStart->enabled = !b_ConstructionStartSnapped;
		m_GuidelinesEnd->enabled = !b_ConstructionEndSnapped;

		m_GuidelinesStart->SetTransform(pointA + v3{ 0.0f, 0.15f, 0.0f }, { 0.0f, rotationStart, 0.0f });
		m_GuidelinesEnd->SetTransform(pointB + v3{ 0.0f, 0.15f, 0.0f }, { 0.0f, rotationEnd, 0.0f });

		m_GuidelinesStart->tintColor = b_ConstructionRestricted ? v4{ 1.0f, 0.3f, 0.2f, 1.0f } : v4(1.0f);
		m_GuidelinesEnd->tintColor = b_ConstructionRestricted ? v4{ 1.0f, 0.3f, 0.2f, 1.0f } : v4(1.0f);

		for (std::vector<Object*>& os : m_Guidelines)
			for (Object* rg : os)
				rg->tintColor = b_ConstructionRestricted ? v4{ 1.0f, 0.3f, 0.2f, 1.0f } : v4(1.0f);

	}
	void RoadManager::DrawCurvedGuidelines(const std::array<v3, 4>& curvePoints)
	{
		Prefab* selectedRoad = m_Scene->MainApplication->road_types[m_Type].road;
		f32 roadPrefabWidth = selectedRoad->boundingBoxM.z - selectedRoad->boundingBoxL.z;
		f32 roadPrefabLength = selectedRoad->boundingBoxM.x - selectedRoad->boundingBoxL.x;

		f32 l = glm::length(curvePoints[3] - curvePoints[0]);
		u64 count = 1;
		while (l > roadPrefabLength)
		{
			count *= 2;
			v3 p = Math::CubicCurve<f32>(curvePoints, 1.0f / count);
			l = glm::length(p - curvePoints[0]);
		}
		if (count > 1) count /= 2;
		while (l > roadPrefabLength)
		{
			count++;
			v3 p = Math::CubicCurve<f32>(curvePoints, 1.0f / count);
			l = glm::length(p - curvePoints[0]);
		}
		if (count > 1) count--;

		v3 AB1 = curvePoints[1] - curvePoints[0];
		v3 AB2 = curvePoints[2] - curvePoints[3];

		f32 rotationOffset1 = (f32)(AB1.x >= 0.0f) * glm::radians(180.0f);
		f32 rotationOffset2 = (f32)(AB2.x >= 0.0f) * glm::radians(180.0f);

		f32 rotationStart = glm::atan(-AB1.z / AB1.x) + rotationOffset1;
		f32 rotationEnd = glm::atan(-AB2.z / AB2.x) + rotationOffset2;

		m_GuidelinesStart->enabled = !b_ConstructionStartSnapped;
		m_GuidelinesEnd->enabled = !b_ConstructionEndSnapped;

		m_GuidelinesStart->SetTransform(curvePoints[0] + v3{ 0.0f, 0.15f, 0.0f }, { 0.0f, rotationStart, 0.0f });
		m_GuidelinesEnd->SetTransform(curvePoints[3] + v3{ 0.0f, 0.15f, 0.0f }, { 0.0f, rotationEnd, 0.0f });

		for (u64& inUse : m_GuidelinesInUse)
			inUse = 0;
		m_GuidelinesInUse[m_Type] += count;

		if (m_GuidelinesInUse[m_Type] > m_Guidelines[m_Type].size())
			for (u64 j = m_Guidelines[m_Type].size(); j < m_GuidelinesInUse[m_Type]; j++)
				m_Guidelines[m_Type].push_back(new Object(m_Scene->MainApplication->road_types[m_Type].road));

		v3 p1 = curvePoints[0];
		for (int c = 0; c < count; c++)
		{
			v3 p2 = Math::CubicCurve<f32>(std::array<v3, 4>{curvePoints}, (c + 1.0f) / count);
			v3 vec1 = p2 - p1;
			f32 length = glm::length(vec1);
			v3 dir1 = vec1 / length;

			f32 scale = length / roadPrefabLength;
			f32 rot1 = glm::acos(dir1.x) * ((f32)(dir1.z < 0.0f) * 2.0f - 1.0f);

			Object* roadSG = m_Guidelines[m_Type][c];
			roadSG->enabled = true;
			roadSG->SetTransform(
				p1 + v3{ 0.0f, 0.15f, 0.0f },
				v3{ 0.0f, rot1, 0.0f },
				v3{ scale, 1.0f, 1.0f }
			);

			p1 = p2;
		}

		m_GuidelinesStart->tintColor = b_ConstructionRestricted ? v4{ 1.0f, 0.3f, 0.2f, 1.0f } : v4(1.0f);
		m_GuidelinesEnd->tintColor = b_ConstructionRestricted ? v4{ 1.0f, 0.3f, 0.2f, 1.0f } : v4(1.0f);

		for (std::vector<Object*>& os : m_Guidelines)
			for (Object* rg : os)
				rg->tintColor = b_ConstructionRestricted ? v4{ 1.0f, 0.3f, 0.2f, 1.0f } : v4(1.0f);
	}

	bool RoadManager::CheckStraightRoadRoadCollision(const std::array<std::array<v2, 3>, 2>& polygon)
	{
		u64 segmentCount = m_Segments.size();
		for (u64 rsIndex = 0; rsIndex < segmentCount; rsIndex++)
		{
			if (rsIndex == m_StartSnappedSegment)
				continue;
			if (rsIndex == m_EndSnappedSegment)
				continue;
			if (m_StartSnappedNode != -1)
			{
				auto it = std::find(m_Nodes[m_StartSnappedNode].roadSegments.begin(), m_Nodes[m_StartSnappedNode].roadSegments.end(), rsIndex);
				if (it != m_Nodes[m_StartSnappedNode].roadSegments.end())
					continue;
			}
			if (m_EndSnappedNode != -1)
			{
				auto it = std::find(m_Nodes[m_EndSnappedNode].roadSegments.begin(), m_Nodes[m_EndSnappedNode].roadSegments.end(), rsIndex);
				if (it != m_Nodes[m_EndSnappedNode].roadSegments.end())
					continue;
			}
			RoadSegment& rs = m_Segments[rsIndex];
			f32 halfWidth = rs.type.road_width * 0.5f;

			std::array<std::array<v2, 3>, 2> oldRoadPolygon = Math::GetBoundingBoxOfBezierCurve(rs.GetCurvePoints(), halfWidth);

			if (Math::CheckPolygonCollision(polygon, oldRoadPolygon))
			{
				std::array<std::array<v2, 3>, (10 - 1) * 2> result = Math::GetBoundingPolygonOfBezierCurve<10, 10>(rs.GetCurvePoints(), halfWidth);
				if (Math::CheckPolygonCollision(result, polygon))
					return true;
			}
		}
		return false;
	}
	void RoadManager::CheckStraightRoadBuildingCollision(const std::array<std::array<v2, 3>, 2>& polygon)
	{
		for (Building* building : m_Scene->m_BuildingManager.GetBuildings())
		{
			Prefab* prefab = building->object->prefab;
			v2 pos{ building->object->position.x, building->object->position.z };
			v2 A = { prefab->boundingBoxL.x, prefab->boundingBoxL.z };
			v2 B = { prefab->boundingBoxL.x, prefab->boundingBoxM.z };
			v2 C = { prefab->boundingBoxM.x, prefab->boundingBoxL.z };
			v2 D = { prefab->boundingBoxM.x, prefab->boundingBoxM.z };

			f32 rot = building->object->rotation.y;
			A = Math::RotatePoint(A, rot) + pos;
			B = Math::RotatePoint(B, rot) + pos;
			C = Math::RotatePoint(C, rot) + pos;
			D = Math::RotatePoint(D, rot) + pos;

			std::array<std::array<v2, 3>, 2> polygonBuilding = {
				std::array<v2,3>{A, B, D},
				std::array<v2,3>{A, C, D}
			};
			building->object->tintColor = v4(1.0f);
			if (Math::CheckPolygonCollision(polygon, polygonBuilding))
				building->object->tintColor = v4{ 1.0f, 0.3f, 0.2f, 1.0f };
		}
	}
	void RoadManager::CheckStraightRoadTreeCollision(const std::array<std::array<v2, 3>, 2>& polygon)
	{
		for (Object* tree : m_Scene->m_TreeManager.GetTrees())
		{
			Prefab* prefab = tree->prefab;
			v2 pos{ tree->position.x, tree->position.z };
			v2 A{ prefab->boundingBoxL.x * tree->scale.x, prefab->boundingBoxL.z * tree->scale.z };
			v2 B{ prefab->boundingBoxL.x * tree->scale.x, prefab->boundingBoxM.z * tree->scale.z };
			v2 C{ prefab->boundingBoxM.x * tree->scale.x, prefab->boundingBoxL.z * tree->scale.z };
			v2 D{ prefab->boundingBoxM.x * tree->scale.x, prefab->boundingBoxM.z * tree->scale.z };

			f32 rot = tree->rotation.y;
			A = Math::RotatePoint(A, rot) + pos;
			B = Math::RotatePoint(B, rot) + pos;
			C = Math::RotatePoint(C, rot) + pos;
			D = Math::RotatePoint(D, rot) + pos;

			std::array<std::array<v2, 3>, 2> polygonTree = {
				std::array<v2,3>{A, B, D},
				std::array<v2,3>{A, C, D}
			};

			tree->tintColor = v4(1.0f);
			if (Math::CheckPolygonCollision(polygon, polygonTree))
				tree->tintColor = v4{ 1.0f, 0.3f, 0.2f, 1.0f };
		}

	}

	bool RoadManager::CheckRoadRoadCollision(const std::array<std::array<v2, 3>, 2>& box, const std::array<std::array<v2, 3>, (10 - 1) * 2>& polygon)
	{
		u64 segmentCount = m_Segments.size();
		for (u64 rsIndex = 0; rsIndex < segmentCount; rsIndex++)
		{
			if (rsIndex == m_StartSnappedSegment)
				continue;
			if (rsIndex == m_EndSnappedSegment)
				continue;
			if (m_StartSnappedNode != -1)
			{
				auto it = std::find(m_Nodes[m_StartSnappedNode].roadSegments.begin(), m_Nodes[m_StartSnappedNode].roadSegments.end(), rsIndex);
				if (it != m_Nodes[m_StartSnappedNode].roadSegments.end())
					continue;
			}
			if (m_EndSnappedNode != -1)
			{
				auto it = std::find(m_Nodes[m_EndSnappedNode].roadSegments.begin(), m_Nodes[m_EndSnappedNode].roadSegments.end(), rsIndex);
				if (it != m_Nodes[m_EndSnappedNode].roadSegments.end())
					continue;
			}
			RoadSegment& rs = m_Segments[rsIndex];
			f32 halfWidth = rs.type.road_width * 0.5f;

			std::array<std::array<v2, 3>, 2> oldRoadBoundingBox = Math::GetBoundingBoxOfBezierCurve(rs.GetCurvePoints(), halfWidth);

			if (Math::CheckPolygonCollision(box, oldRoadBoundingBox))
			{
				if (Math::CheckPolygonCollision(polygon, oldRoadBoundingBox))
				{
					std::array<std::array<v2, 3>, (10 - 1) * 2> oldRoadBoundingPolygon = Math::GetBoundingPolygonOfBezierCurve<10, 10>(rs.GetCurvePoints(), halfWidth);
					if (Math::CheckPolygonCollision(box, oldRoadBoundingPolygon))
					{
						if (Math::CheckPolygonCollision(polygon, oldRoadBoundingPolygon))
						{
							return true;
						}
					}
				}
			}
		}
		return false;
	}
	void RoadManager::CheckRoadBuildingCollision(const std::array<std::array<v2, 3>, 2>& box, const std::array<std::array<v2, 3>, (10 - 1) * 2>& polygon)
	{
		for (Building* building : m_Scene->m_BuildingManager.GetBuildings())
		{
			Prefab* prefab = building->object->prefab;
			v2 pos{ building->object->position.x, building->object->position.z };
			v2 A = { prefab->boundingBoxL.x, prefab->boundingBoxL.z };
			v2 B = { prefab->boundingBoxL.x, prefab->boundingBoxM.z };
			v2 C = { prefab->boundingBoxM.x, prefab->boundingBoxL.z };
			v2 D = { prefab->boundingBoxM.x, prefab->boundingBoxM.z };

			f32 rot = building->object->rotation.y;
			A = Math::RotatePoint(A, rot) + pos;
			B = Math::RotatePoint(B, rot) + pos;
			C = Math::RotatePoint(C, rot) + pos;
			D = Math::RotatePoint(D, rot) + pos;

			std::array<std::array<v2, 3>, 2> polygonBuilding = {
				std::array<v2,3>{A, B, D},
				std::array<v2,3>{A, C, D}
			};
			building->object->tintColor = v4(1.0f);
			if (Math::CheckPolygonCollision(box, polygonBuilding))
				if (Math::CheckPolygonCollision(polygon, polygonBuilding))
					building->object->tintColor = v4{ 1.0f, 0.3f, 0.2f, 1.0f };
		}
	}
	void RoadManager::CheckRoadTreeCollision(const std::array<std::array<v2, 3>, 2>& box, const std::array<std::array<v2, 3>, (10 - 1) * 2>& polygon)
	{
		for (Object* tree : m_Scene->m_TreeManager.GetTrees())
		{
			Prefab* prefab = tree->prefab;
			v2 pos{ tree->position.x, tree->position.z };
			v2 A = { prefab->boundingBoxL.x * tree->scale.x, prefab->boundingBoxL.z * tree->scale.z };
			v2 B = { prefab->boundingBoxL.x * tree->scale.x, prefab->boundingBoxM.z * tree->scale.z };
			v2 C = { prefab->boundingBoxM.x * tree->scale.x, prefab->boundingBoxL.z * tree->scale.z };
			v2 D = { prefab->boundingBoxM.x * tree->scale.x, prefab->boundingBoxM.z * tree->scale.z };

			f32 rot = tree->rotation.y;
			A = Math::RotatePoint(A, rot) + pos;
			B = Math::RotatePoint(B, rot) + pos;
			C = Math::RotatePoint(C, rot) + pos;
			D = Math::RotatePoint(D, rot) + pos;

			std::array<std::array<v2, 3>, 2> polygonTree = {
				std::array<v2,3>{A, B, D},
				std::array<v2,3>{A, C, D}
			};

			tree->tintColor = v4(1.0f);
			if (Math::CheckPolygonCollision(box, polygonTree))
				if (Math::CheckPolygonCollision(polygon, polygonTree))
					tree->tintColor = v4{ 1.0f, 0.3f, 0.2f, 1.0f };
		}
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
				m_GuidelinesStart->enabled = true;
				m_GuidelinesEnd->enabled = true;
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
				m_GuidelinesStart->enabled = true;
				m_GuidelinesEnd->enabled = true;
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
				m_GuidelinesStart->enabled = true;
				m_GuidelinesEnd->enabled = true;
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
			m_ConstructionPositions[1] = (m_ConstructionPositions[0] + m_ConstructionPositions[3]) / 2.0f;
			m_ConstructionPositions[2] = (m_ConstructionPositions[0] + m_ConstructionPositions[3]) / 2.0f;

			for (std::vector<Object*>& os : m_Guidelines)
				for (Object* rg : os)
					rg->enabled = false;
			for (u64& inUse : m_GuidelinesInUse)
				inUse = 0;
			m_ConstructionPhase = 0;

			AddRoadSegment({
					m_ConstructionPositions[0],
					(3.0f * m_ConstructionPositions[0] + m_ConstructionPositions[3]) * 0.25f,
					(3.0f * m_ConstructionPositions[3] + m_ConstructionPositions[0]) * 0.25f,
					m_ConstructionPositions[3],
				});

			ResetStates();
			m_Scene->m_TreeManager.ResetStates();
			m_Scene->m_BuildingManager.ResetStates();

			m_GuidelinesStart->enabled = true;
			m_GuidelinesEnd->enabled = true;
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
			for (std::vector<Object*>& os : m_Guidelines)
				for (Object* rg : os)
					rg->enabled = false;
			for (u64& inUse : m_GuidelinesInUse)
				inUse = 0;
			m_ConstructionPhase = 0;

			AddRoadSegment({
					m_ConstructionPositions[0],
					(m_ConstructionPositions[2] + m_ConstructionPositions[0]) * 0.5f,
					(m_ConstructionPositions[2] + m_ConstructionPositions[3]) * 0.5f,
					m_ConstructionPositions[3],
				});

			ResetStates();
			m_Scene->m_TreeManager.ResetStates();
			m_Scene->m_BuildingManager.ResetStates();

			m_GuidelinesStart->enabled = true;
			m_GuidelinesEnd->enabled = true;
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
			for (std::vector<Object*>& os : m_Guidelines)
				for (Object* rg : os)
					rg->enabled = false;
			for (u64& inUse : m_GuidelinesInUse)
				inUse = 0;
			m_ConstructionPhase = 0;

			AddRoadSegment(m_ConstructionPositions);

			ResetStates();
			m_Scene->m_TreeManager.ResetStates();
			m_Scene->m_BuildingManager.ResetStates();

			m_GuidelinesStart->enabled = true;
			m_GuidelinesEnd->enabled = true;
		}
		return false;
	}
	bool RoadManager::OnMousePressed_Change()
	{
		if (selected_road_segment != -1)
			return false;
		RoadSegment& rs = m_Segments[selected_road_segment];
		rs.SetType(m_Scene->MainApplication->road_types[m_Type]);

		m_Nodes[rs.StartNode].Reconstruct();
		m_Nodes[rs.EndNode].Reconstruct();

		return false;
	}
	bool RoadManager::OnMousePressed_Destruction()
	{
		if (m_DestructionNode != -1)
		{
			RoadNode& node = m_Nodes[m_DestructionNode];
			for (u64 rsIndex : node.roadSegments)
				RemoveRoadSegment(rsIndex);
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
		delete m_GuidelinesEnd;
		delete m_GuidelinesStart;

		const RoadType& t = m_Scene->MainApplication->road_types[m_Type];
		m_GuidelinesStart = new Object(t.asymmetric ? t.end_mirror : t.end);
		m_GuidelinesEnd = new Object(t.end);
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
			m_GuidelinesStart->enabled = true;
			m_GuidelinesEnd->enabled = true;
			break;
		case RoadConstructionMode::Change:
			break;
		case RoadConstructionMode::Destruct:
			break;
		default:
			break;
		}
	}

	void RoadManager::AddRoadSegment(const std::array<v3, 4>& curvePoints)
	{
		Prefab* selectedRoad = m_Scene->MainApplication->road_types[m_Type].road;
		f32 roadPrefabWidth = selectedRoad->boundingBoxM.z - selectedRoad->boundingBoxL.z;
		m_Segments.push_back(RoadSegment(
			m_Scene->MainApplication->road_types[m_Type],
			curvePoints
		));
		u64 rsIndex = m_Segments.size() - 1;

		std::array<std::array<v2, 3>, 2> newRoadBoundingBox = Math::GetBoundingBoxOfBezierCurve(curvePoints, roadPrefabWidth * 0.5f);
		std::array<std::array<v2, 3>, (10 - 1) * 2> newRoadBoundingPolygon = Math::GetBoundingPolygonOfBezierCurve<10, 10>(curvePoints, roadPrefabWidth * 0.5f);

		/* check collisions for End s if not snapped
		if (m_StartSnappedEnd || m_StartSnappedJunction || m_StartSnappedRoadSegment)
			least.x = 0.0f;
		if (m_EndSnappedEnd || m_EndSnappedJunction || m_EndSnappedRoadSegment)
			most.x = glm::length(AB);*/

		auto& buildings = m_Scene->m_BuildingManager.GetBuildings();
		if (m_Scene->m_BuildingManager.restrictions[0] && (restrictionFlags & RESTRICT_COLLISIONS))
			for (u64 i = 0; i < buildings.size(); i++)
			{
				Building* building = buildings[i];

				Prefab* prefab = building->object->prefab;
				v2 pos{ building->object->position.x, building->object->position.z };
				v2 A = { prefab->boundingBoxL.x, prefab->boundingBoxL.z };
				v2 B = { prefab->boundingBoxL.x, prefab->boundingBoxM.z };
				v2 C = { prefab->boundingBoxM.x, prefab->boundingBoxL.z };
				v2 D = { prefab->boundingBoxM.x, prefab->boundingBoxM.z };

				f32 rot = building->object->rotation.y;
				A = Math::RotatePoint(A, rot) + pos;
				B = Math::RotatePoint(B, rot) + pos;
				C = Math::RotatePoint(C, rot) + pos;
				D = Math::RotatePoint(D, rot) + pos;

				std::array<std::array<v2, 3>, 2> polygonBuilding = {
					std::array<v2,3>{A, B, D},
					std::array<v2,3>{A, C, D}
				};

				building->object->tintColor = v4(1.0f);
				if (Math::CheckPolygonCollision(newRoadBoundingBox, polygonBuilding))
					if (Math::CheckPolygonCollision(newRoadBoundingPolygon, polygonBuilding))
					{
						if (building->connectedRoadSegment)
						{
							RoadSegment& rs = m_Segments[building->connectedRoadSegment];
							auto it = std::find(
								rs.Buildings.begin(),
								rs.Buildings.end(),
								building
							);
							rs.Buildings.erase(it);
						}
						buildings.erase(buildings.begin() + i);
						delete building;
						i--;
					}
			}

		auto& trees = m_Scene->m_TreeManager.GetTrees();
		if (m_Scene->m_TreeManager.restrictions[0] && (restrictionFlags & RESTRICT_COLLISIONS))
			for (u64 i = 0; i < trees.size(); i++)
			{
				Object* tree = trees[i];
				Prefab* prefab = tree->prefab;
				v2 pos{ tree->position.x, tree->position.z };
				v2 A = { prefab->boundingBoxL.x * tree->scale.x, prefab->boundingBoxL.z * tree->scale.z };
				v2 B = { prefab->boundingBoxL.x * tree->scale.x, prefab->boundingBoxM.z * tree->scale.z };
				v2 C = { prefab->boundingBoxM.x * tree->scale.x, prefab->boundingBoxL.z * tree->scale.z };
				v2 D = { prefab->boundingBoxM.x * tree->scale.x, prefab->boundingBoxM.z * tree->scale.z };

				f32 rot = tree->rotation.y;
				A = Math::RotatePoint(A, rot) + pos;
				B = Math::RotatePoint(B, rot) + pos;
				C = Math::RotatePoint(C, rot) + pos;
				D = Math::RotatePoint(D, rot) + pos;

				std::array<std::array<v2, 3>, 2> polygonTree = {
					std::array<v2,3>{A, B, D},
					std::array<v2,3>{A, C, D}
				};

				tree->tintColor = v4(1.0f);
				if (Math::CheckPolygonCollision(newRoadBoundingBox, polygonTree))
					if (Math::CheckPolygonCollision(newRoadBoundingPolygon, polygonTree))
					{
						trees.erase(trees.begin() + i);
						delete tree;
						i--;
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
				m_Scene->MainApplication->road_types[m_Type],
				curvePoints
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

			m_Nodes.push_back(RoadNode({}, curvePoints[0]));
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
				f32 t = building->snappedT;
				if (t < m_StartSnappedT)
				{
					building->snappedT = t / m_StartSnappedT;
				}
				else
				{
					building->snappedT = (1.0f - t) / (1.0f - m_StartSnappedT);
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
				f32 lengthRoad = segment.type.road_length;
				std::vector<v3> samples = Math::GetCubicCurveSamples(segment.GetCurvePoints(), lengthRoad, ts);
				if (t_index >= ts.size())
				{
					t_index = ts.size() - 1;
				}
				f32 t = ts[t_index];
				if (t < m_StartSnappedT)
				{
					std::vector<f32> ts2{ 0 };
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
			m_Nodes.push_back(RoadNode({}, curvePoints[0]));
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
				m_Scene->MainApplication->road_types[m_Type],
				curvePoints
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

			m_Nodes.push_back(RoadNode({ }, curvePoints[3]));
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
				f32 t = building->snappedT;
				if (t < m_EndSnappedT)
				{
					building->snappedT = t / m_EndSnappedT;
				}
				else
				{
					building->snappedT = (1.0f - t) / (1.0f - m_EndSnappedT);
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
				f32 lengthRoad = segment.type.road_length;
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
			m_Nodes.push_back(RoadNode({}, curvePoints[3]));
			u64 nodeIndex = m_Nodes.size() - 1;
			RoadNode& node = m_Nodes[nodeIndex];
			rs.EndNode = nodeIndex;
			node.index = nodeIndex;
			node.AddRoadSegment({ rsIndex });
		}
	}
	void RoadManager::RemoveRoadSegment(u64 roadSegment)
	{
		RoadSegment& rs = m_Segments[roadSegment];

		for (Building* building : rs.Buildings)
		{
			m_Scene->m_BuildingManager.GetBuildings().erase(std::find(m_Scene->m_BuildingManager.GetBuildings().begin(), m_Scene->m_BuildingManager.GetBuildings().end(), building));
			delete building;
		}
		for (Car* car : rs.Cars)
		{
			m_Scene->m_CarManager.GetCars().erase(std::find(m_Scene->m_CarManager.GetCars().begin(), m_Scene->m_CarManager.GetCars().end(), car));
			delete car;
		}

		RoadNode& startNode = m_Nodes[rs.StartNode];
		startNode.RemoveRoadSegment(roadSegment);
		if (startNode.roadSegments.size() == 0)
		{
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
		auto& cars = m_Scene->m_CarManager.GetCars();
		count = cars.size();
		for (u64 cIndex = 0; cIndex < count; cIndex++)
		{
			Car* car = cars[cIndex]; // well be changed to indices instead of pointers
			if ((u64)car->roadSegment > roadSegment)
				car->roadSegment--;
		}
		auto& buildings = m_Scene->m_BuildingManager.GetBuildings();
		count = buildings.size();
		for (u64 bIndex = 0; bIndex < count; bIndex++)
		{
			Building* building = buildings[bIndex];
			if ((u64)building->connectedRoadSegment > roadSegment)
				building->connectedRoadSegment--;
		}
		m_Segments.erase(std::find(m_Segments.begin(), m_Segments.end(), rs));
	}

	SnapInformation RoadManager::CheckSnapping(const v3& prevLocation)
	{
		Prefab* selectedRoad = m_Scene->MainApplication->road_types[m_Type].road;
		f32 roadSegmentPrefabWidth = selectedRoad->boundingBoxM.z - selectedRoad->boundingBoxL.z;
		f32 roadPrefabLength = selectedRoad->boundingBoxM.x - selectedRoad->boundingBoxL.x;

		SnapInformation results;
		results.location = prevLocation;
		v2 P{ prevLocation.x, prevLocation.z };

		for (u64 i = 0; i < m_Nodes.size(); i++)
			if (glm::length(m_Nodes[i].position - prevLocation) < roadSegmentPrefabWidth)
			{
				results.location = m_Nodes[i].position;
				results.snapped = true;
				results.node = i;
				return results;
			}

		for (u64 i = 0; i < m_Segments.size(); i++)
			if (Math::CheckPolygonPointCollision(m_Segments[i].bounding_box, P))
			{
				f32 snapDist = (roadSegmentPrefabWidth + m_Segments[i].type.road_width) * 0.5f;
				u64 curve_samples_size = m_Segments[i].curve_samples.size();
				CAN_ASSERT(curve_samples_size > 1, "Samples size can't be smaller than 2");
				v3 point0 = m_Segments[i].curve_samples[0];
				for (u64 j = 1; j < curve_samples_size; j++)
				{
					v3 point1 = m_Segments[i].curve_samples[j];
					v3 dirToP1 = point1 - point0;
					dirToP1.y = 0.0f;
					f32 lenr = glm::length(dirToP1);
					dirToP1 = dirToP1 / lenr;

					v3 dirToPrev = prevLocation - point0;
					f32 len = glm::length(dirToPrev);

					f32 angle = glm::acos(glm::dot(dirToP1, dirToPrev) / len);
					f32 dist = len * glm::sin(angle);

					if (dist < snapDist)
					{
						f32 c = len * glm::cos(angle);
						if (c >= -0.5f * m_Segments[i].type.road_width && c <= lenr + 0.5f * m_Segments[i].type.road_width)
						{

							f32 t = std::max(0.0f, std::min(1.0f, glm::cos(angle)));
							results.location = point0 + t * lenr * dirToP1;
							results.segment = i;
							results.snapped = true;
							results.T = Math::Lerp(m_Segments[i].curve_t_samples[j - 1], m_Segments[i].curve_t_samples[j], t);
							return results;
						}
					}
					point0 = point1;
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

		m_GuidelinesStart->enabled = false;
		m_GuidelinesEnd->enabled = false;

		m_GuidelinesStart->tintColor = v4(1.0f);
		m_GuidelinesEnd->tintColor = v4(1.0f);
	}

	void RoadManager::SnapToGrid(v3& prevLocation)
	{
		if (snapFlags & SNAP_TO_GRID)
		{
			prevLocation.x = prevLocation.x - std::fmod(prevLocation.x, 0.5f) + 0.25f;
			prevLocation.z = prevLocation.z - std::fmod(prevLocation.z, 0.5f) - 0.25f;
		}
	}
	void RoadManager::SnapToRoad(v3& prevLocation, bool isStart)
	{
		if (snapFlags & SNAP_TO_ROAD)
		{
			SnapInformation snapInformation = CheckSnapping(prevLocation);
			prevLocation = snapInformation.location;
			if (isStart)
			{
				b_ConstructionStartSnapped = snapInformation.snapped;
				m_StartSnappedSegment = snapInformation.segment;
				m_StartSnappedNode = snapInformation.node;
				m_StartSnappedT = snapInformation.T;
			}
			else
			{
				b_ConstructionEndSnapped = snapInformation.snapped;
				m_EndSnappedSegment = snapInformation.segment;
				m_EndSnappedNode = snapInformation.node;
				m_EndSnappedT = snapInformation.T;
			}
		}
	}
	void RoadManager::SnapToHeight(const std::vector<u8>& indices, u8 index, v3& AB)
	{
		if (snapFlags & SNAP_TO_HEIGHT)
		{
			for (u8 i = 0; i < indices.size(); i++)
				m_ConstructionPositions[indices[i]].y = m_ConstructionPositions[index].y;
			AB.y = 0.0f;
		}
	}
	void RoadManager::SnapToAngle(v3& AB, s64 snappedNode, s64 snappedRoadSegment, f32 snappedT)
	{
		if (snapFlags & SNAP_TO_ANGLE)
		{
			f32 rotation1 = glm::atan(-AB.z / AB.x) + (f32)(AB.x < 0.0f) * glm::radians(180.0f);
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
				f32 angleDiff = crossProduct.y > 0.0 ? glm::radians(newAngle - angle) : glm::radians(angle - newAngle);
				AB = glm::rotateY(AB, angleDiff);
			}
		}
	}
	void RoadManager::ResetGuideLines()
	{
		for (std::vector<Object*>& os : m_Guidelines)
			for (Object* rsg : os)
				rsg->enabled = false;
		for (u64& inUse : m_GuidelinesInUse)
			inUse = 0;
	}
	bool RoadManager::RestrictSmallAngles(v3 direction, s64 snappedNode, s64 snappedRoadSegment, f32 snappedT)
	{
		if (restrictionFlags & RESTRICT_SMALL_ANGLES)
		{
			//v3 dirToNewRS = locStart - m_ConstructionPositions[0];
			direction.y = 0;
			direction = glm::normalize(direction);

			if (snappedNode != -1)
			{
				RoadNode& node = m_Nodes[snappedNode];
				for (u64 rsIndex : node.roadSegments)
				{
					RoadSegment& rs = m_Segments[rsIndex];

					v3 dirToOldRS = rs.StartNode == snappedNode ? rs.GetStartDirection() : rs.GetEndDirection();
					dirToOldRS.y = 0.0f;
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
				v3 tangent = Math::CubicCurveTangent(rs.GetCurvePoints(), snappedT);

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