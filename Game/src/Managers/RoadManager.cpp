#include "canpch.h"
#include "RoadManager.h"

#include "Types/RoadSegment.h"
#include "Junction.h"
#include "Building.h"
#include "End.h"

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
		m_GuidelinesStart = new Object(m_Scene->MainApplication->roads[m_Type][2], m_Scene->MainApplication->roads[m_Type][2], glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), false);
		m_GuidelinesEnd = new Object(m_Scene->MainApplication->roads[m_Type][2], m_Scene->MainApplication->roads[m_Type][2], glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), false);

		size_t roadTypeCount = m_Scene->MainApplication->roads.size();
		for (size_t i = 0; i < roadTypeCount; i++)
		{
			m_GuidelinesInUse.push_back(0);
			m_Guidelines.push_back({});
			m_Guidelines[i].push_back(new Object(m_Scene->MainApplication->roads[i][0], m_Scene->MainApplication->roads[i][0], glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), false));
		}
	}
	RoadManager::~RoadManager()
	{
	}

	void RoadManager::OnUpdate(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
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
		case  RoadConstructionMode::Upgrade:
			break;
		case  RoadConstructionMode::Destruct:
			OnUpdate_Destruction(prevLocation, cameraPosition, cameraDirection);
			break;
		}
	}
	void RoadManager::OnUpdate_Straight(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
	{
		Prefab* selectedRoad = m_Scene->MainApplication->roads[m_Type][0];
		float roadPrefabWidth = selectedRoad->boundingBoxM.z - selectedRoad->boundingBoxL.z;
		float roadPrefabLength = selectedRoad->boundingBoxM.x - selectedRoad->boundingBoxL.x;

		if (m_ConstructionPhase == 0)
		{
			if (snapOptions[4])
			{
				prevLocation.x = prevLocation.x - std::fmod(prevLocation.x + 0.25f, 0.5f) + 0.25f;
				prevLocation.z = prevLocation.z - std::fmod(prevLocation.z + 0.25f, 0.5f) - 0.25f;
			}
			if (snapOptions[0])
			{
				SnapInformation snapInformation = CheckSnapping(cameraPosition, cameraDirection);
				prevLocation = snapInformation.snapped ? snapInformation.location : prevLocation;
				b_ConstructionStartSnapped = snapInformation.snapped;
				m_StartSnappedJunction = snapInformation.junction;
				m_StartSnappedEnd = snapInformation.end;
				m_StartSnappedRoadSegment = snapInformation.roadSegment;
				m_StartSnappedRoadSegmentT = snapInformation.roadT;
				m_StartSnappedRoadSegmentTDelta = snapInformation.roadTDelta;
			}
			m_ConstructionPositions[0] = prevLocation;

			m_GuidelinesStart->SetTransform(prevLocation + glm::vec3{ 0.0f, 0.15f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, glm::radians(180.0f), 0.0f });
			m_GuidelinesEnd->SetTransform(prevLocation + glm::vec3{ 0.0f, 0.15f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f });
		}
		else
		{
			for (std::vector<Object*>& os : m_Guidelines)
				for (Object* rsg : os)
					rsg->enabled = false;

			for (size_t& inUse : m_GuidelinesInUse)
				inUse = 0;

			b_ConstructionRestricted = false;
			if (snapOptions[4])
			{
				prevLocation.x = prevLocation.x - std::fmod(prevLocation.x + 0.25f, 0.5f) + 0.25f;
				prevLocation.z = prevLocation.z - std::fmod(prevLocation.z + 0.25f, 0.5f) + 0.25f;
			}
			if (snapOptions[0])
			{
				SnapInformation snapInformation = CheckSnapping(cameraPosition, cameraDirection);
				prevLocation = snapInformation.snapped ? snapInformation.location : prevLocation;
				b_ConstructionEndSnapped = snapInformation.snapped;
				m_EndSnappedJunction = snapInformation.junction;
				m_EndSnappedEnd = snapInformation.end;
				m_EndSnappedRoadSegment = snapInformation.roadSegment;
				m_EndSnappedRoadSegmentT = snapInformation.roadT;
				m_EndSnappedRoadSegmentTDelta = snapInformation.roadTDelta;
			}
			m_ConstructionPositions[1] = prevLocation;
			m_ConstructionPositions[2] = prevLocation;
			m_ConstructionPositions[3] = prevLocation;

			bool angleIsRestricted = false;
			if (restrictions[0])
			{
				if (m_StartSnappedJunction)
				{
					for (RoadSegment* roadSegment : m_StartSnappedJunction->connectedRoadSegments)
					{
						glm::vec3 directionOldRoadSegment = roadSegment->ConnectedObjectAtStart.junction == m_StartSnappedJunction ? roadSegment->GetStartDirection() : roadSegment->GetEndDirection();
						directionOldRoadSegment.y = 0;
						directionOldRoadSegment = glm::normalize(directionOldRoadSegment);

						glm::vec3 directionNewRoadSegment = prevLocation - m_ConstructionPositions[0];
						directionNewRoadSegment.y = 0;
						directionNewRoadSegment = glm::normalize(directionNewRoadSegment);

						float angle = glm::acos(glm::dot(directionOldRoadSegment, directionNewRoadSegment));

						if (angle < 0.5f)
						{
							angleIsRestricted = true;
							break;
						}
					}
				}
				else if (m_StartSnappedEnd)
				{
					RoadSegment* roadSegment = m_StartSnappedEnd->connectedRoadSegment;

					glm::vec3 directionOldRoadSegment = roadSegment->ConnectedObjectAtStart.end == m_StartSnappedEnd ? roadSegment->GetStartDirection() : roadSegment->GetEndDirection();
					directionOldRoadSegment.y = 0;
					directionOldRoadSegment = glm::normalize(directionOldRoadSegment);

					glm::vec3 directionNewRoadSegment = prevLocation - m_ConstructionPositions[0];
					directionNewRoadSegment.y = 0;
					directionNewRoadSegment = glm::normalize(directionNewRoadSegment);

					float angle = glm::acos(glm::dot(directionOldRoadSegment, directionNewRoadSegment));

					angleIsRestricted = angle < 0.5f;
				}
				else if (m_StartSnappedRoadSegment)
				{
					glm::vec3 prevPoint = Math::CubicCurve(m_StartSnappedRoadSegment->GetCurvePoints(), m_StartSnappedRoadSegmentT - m_StartSnappedRoadSegmentTDelta);
					glm::vec3 directionOldRoadSegment = prevLocation - prevPoint;

					directionOldRoadSegment.y = 0;
					directionOldRoadSegment = glm::normalize(directionOldRoadSegment);

					glm::vec3 directionNewRoadSegment = prevLocation - m_ConstructionPositions[0];
					directionNewRoadSegment.y = 0;
					directionNewRoadSegment = glm::normalize(directionNewRoadSegment);

					float angle = glm::acos(glm::dot(directionOldRoadSegment, directionNewRoadSegment));

					angleIsRestricted = angle < 0.5f || angle > 2.63f;
				}

				if (m_EndSnappedJunction)
				{
					for (RoadSegment* roadSegment : m_EndSnappedJunction->connectedRoadSegments)
					{
						glm::vec3 directionOldRoadSegment = roadSegment->ConnectedObjectAtStart.junction == m_EndSnappedJunction ? roadSegment->GetStartDirection() : roadSegment->GetEndDirection();
						directionOldRoadSegment.y = 0;
						directionOldRoadSegment = glm::normalize(directionOldRoadSegment);

						glm::vec3 directionNewRoadSegment = m_ConstructionPositions[0] - prevLocation;
						directionNewRoadSegment.y = 0;
						directionNewRoadSegment = glm::normalize(directionNewRoadSegment);

						float angle = glm::acos(glm::dot(directionOldRoadSegment, directionNewRoadSegment));

						if (angle < 0.5f)
						{
							angleIsRestricted = true;
							break;
						}
					}
				}
				else if (m_EndSnappedEnd)
				{
					RoadSegment* roadSegment = m_EndSnappedEnd->connectedRoadSegment;

					glm::vec3 directionOldRoadSegment = roadSegment->ConnectedObjectAtStart.end == m_EndSnappedEnd ? roadSegment->GetStartDirection() : roadSegment->GetEndDirection();
					directionOldRoadSegment.y = 0;
					directionOldRoadSegment = glm::normalize(directionOldRoadSegment);

					glm::vec3 directionNewRoadSegment = m_ConstructionPositions[0] - prevLocation;
					directionNewRoadSegment.y = 0;
					directionNewRoadSegment = glm::normalize(directionNewRoadSegment);

					float angle = glm::acos(glm::dot(directionOldRoadSegment, directionNewRoadSegment));

					angleIsRestricted |= angle < 0.5f;
				}
				else if (m_EndSnappedRoadSegment)
				{
					glm::vec3 prevPoint = Math::CubicCurve(m_EndSnappedRoadSegment->GetCurvePoints(), m_EndSnappedRoadSegmentT - m_EndSnappedRoadSegmentTDelta);
					glm::vec3 directionOldRoadSegment = prevLocation - prevPoint;

					directionOldRoadSegment.y = 0;
					directionOldRoadSegment = glm::normalize(directionOldRoadSegment);

					glm::vec3 directionNewRoadSegment = m_ConstructionPositions[0] - prevLocation;
					directionNewRoadSegment.y = 0;
					directionNewRoadSegment = glm::normalize(directionNewRoadSegment);

					float angle = glm::acos(glm::dot(directionOldRoadSegment, directionNewRoadSegment));

					angleIsRestricted |= angle < 0.5f || angle > 2.63f;
				}

			}

			glm::vec3 AB = m_ConstructionPositions[3] - m_ConstructionPositions[0];

			float rotationOffset = AB.x < 0.0f ? 180.0f : 0.0f;
			float rotationStart = glm::atan(-AB.z / AB.x) + glm::radians(180.0f + rotationOffset);
			float rotationEnd = glm::atan(-AB.z / AB.x) + glm::radians(rotationOffset);

			if (snapOptions[1] && glm::length(AB) > 0.5f)
			{
				float length = glm::length(AB);
				length = length - std::fmod(length, roadPrefabLength);
				AB = length * glm::normalize(AB);
				m_ConstructionPositions[3] = m_ConstructionPositions[0] + AB;
			}

			if (snapOptions[2] && glm::length(AB) > 0.5f)
			{
				if (
					!m_EndSnappedJunction &&
					!m_EndSnappedRoadSegment &&
					!m_EndSnappedEnd
					)
					m_ConstructionPositions[3].y = m_ConstructionPositions[0].y;
			}

			if (snapOptions[3] && glm::length(AB) > 0.5f)
			{
				if (m_StartSnappedEnd)
				{
					float snappedRoadRotationY = glm::degrees(m_StartSnappedEnd->object->rotation.y) + 180.0f;
					float newRoadRotationY = glm::degrees(rotationEnd);
					float angle = std::fmod(snappedRoadRotationY - newRoadRotationY + 720.0f, 360.0f);

					float newAngle = 0.0f;
					if (angle < 32.0f)
						newAngle = 30.0f;
					else if (angle > 80.0f && angle < 100.0f)
						newAngle = 90.0f;
					else if (angle > 170.0f && angle < 190.0f)
						newAngle = 180.0f;
					else if (angle > 260.0f && angle < 280.0f)
						newAngle = 270.0f;
					else if (angle > 328.0f)
						newAngle = 330.0f;
					else if (Input::IsKeyPressed(KeyCode::LeftControl))
						newAngle = angle + 2.5f - std::fmod(angle + 2.5f, 5.0f);
					else
						newAngle = angle;

					AB = glm::rotate(AB, glm::radians(angle - newAngle), { 0.0f, 1.0f, 0.0f });
					m_ConstructionPositions[3] = m_ConstructionPositions[0] + AB;
				}
				else if (m_StartSnappedRoadSegment)
				{
					float snappedRoadRotationY = glm::degrees(m_StartSnappedRoadSegment->GetStartRotation().y);
					float newRoadRotationY = glm::degrees(rotationEnd);
					float angle = std::fmod(snappedRoadRotationY - newRoadRotationY + 720.0f, 180.0f);

					float newAngle = 0.0f;
					if (angle < 32.0f)
						newAngle = 30.0f;
					else if (angle > 80.0f && angle < 100.0f)
						newAngle = 90.0f;
					else if (angle > 148.0f && angle < 160.0f)
						newAngle = 150.0f;
					else if (Input::IsKeyPressed(KeyCode::LeftControl))
						newAngle = angle + 2.5f - std::fmod(angle + 2.5f, 5.0f);
					else
						newAngle = angle;

					AB = glm::rotate(AB, -glm::radians(angle - newAngle), { 0.0f, 1.0f, 0.0f });
					m_ConstructionPositions[3] = m_ConstructionPositions[0] + AB;
				}
				else if (m_StartSnappedJunction)
				{
					float newRoadRotationY = glm::degrees(rotationEnd);
					float smallestAngle = 180.0f;
					for (RoadSegment* roadSegment : m_StartSnappedJunction->connectedRoadSegments)
					{
						float snappedRoadRotationY = m_StartSnappedJunction == roadSegment->ConnectedObjectAtStart.junction ? glm::degrees(roadSegment->GetStartRotation().y) : glm::degrees(roadSegment->GetEndRotation().y);
						float angle = std::fmod(snappedRoadRotationY - newRoadRotationY + 720.0f, 180.0f);
						smallestAngle = std::min(smallestAngle, angle);
					}
					float newAngle = 0.0f;
					if (smallestAngle < 32.0f)
						newAngle = 30.0f;
					else if (smallestAngle > 80.0f && smallestAngle < 100.0f)
						newAngle = 90.0f;
					else if (Input::IsKeyPressed(KeyCode::LeftControl))
						newAngle = smallestAngle + 2.5f - std::fmod(smallestAngle + 2.5f, 5.0f);
					else
						newAngle = smallestAngle;

					AB = glm::rotate(AB, -glm::radians(smallestAngle - newAngle), { 0.0f, 1.0f, 0.0f });
					m_ConstructionPositions[3] = m_ConstructionPositions[0] + AB;
				}
				else if (Input::IsKeyPressed(KeyCode::LeftControl))
				{
					float angle = std::fmod(glm::degrees(rotationEnd) + 720.0f, 360.0f);
					float newAngle = angle + 2.5f - std::fmod(angle + 2.5f, 5.0f);

					AB = glm::rotate(AB, -glm::radians(angle - newAngle), { 0.0f, 1.0f, 0.0f });
					m_ConstructionPositions[3] = m_ConstructionPositions[0] + AB;
				}
			}

			if (glm::length(AB) > 0.5f)
			{
				if (m_EndSnappedRoadSegment)
				{
					glm::vec3 n = { -m_EndSnappedRoadSegment->GetStartDirection().z, 0.0f, m_EndSnappedRoadSegment->GetStartDirection().x };
					m_ConstructionPositions[3] = Helper::RayPlaneIntersection(
						m_ConstructionPositions[0],
						AB,
						m_EndSnappedRoadSegment->GetStartPosition(),
						n
					);
					AB = m_ConstructionPositions[3] - m_ConstructionPositions[0];
				}
				else if (m_EndSnappedEnd)
				{
					m_ConstructionPositions[3] = m_EndSnappedEnd->position;
					AB = m_ConstructionPositions[3] - m_ConstructionPositions[0];
				}
				else if (m_EndSnappedJunction)
				{
					m_ConstructionPositions[3] = m_EndSnappedJunction->position;
					AB = m_ConstructionPositions[3] - m_ConstructionPositions[0];
				}
			}

			glm::vec2 A = glm::vec2{ m_ConstructionPositions[0].x, m_ConstructionPositions[0].z };
			glm::vec2 D = glm::vec2{ m_ConstructionPositions[3].x, m_ConstructionPositions[3].z };
			glm::vec2 AD = roadPrefabWidth * 0.5f * glm::normalize(D - A);

			if (!(m_StartSnappedEnd || m_StartSnappedJunction || m_StartSnappedRoadSegment))
				A -= AD;
			if (!(m_EndSnappedEnd || m_EndSnappedJunction || m_EndSnappedRoadSegment))
				D += AD;

			AD = glm::vec2{ -AD.y , AD.x };

			glm::vec2 P1 = A + AD;
			glm::vec2 P2 = A - AD;
			glm::vec2 P3 = D + AD;
			glm::vec2 P4 = D - AD;

			std::array<std::array<glm::vec2, 3>, 2> newRoadPolygon = {
					std::array<glm::vec2,3>{ P1, P2, P3},
					std::array<glm::vec2,3>{ P2, P3, P4}
			};
			bool collisionIsRestricted = false;
			if (restrictions[2])
			{
				for (RoadSegment* roadSegment : m_RoadSegments)
				{
					if (roadSegment == m_EndSnappedRoadSegment || roadSegment == m_StartSnappedRoadSegment)
						continue;
					if (m_StartSnappedEnd && roadSegment == m_StartSnappedEnd->connectedRoadSegment)
						continue;
					if (m_EndSnappedEnd && roadSegment == m_EndSnappedEnd->connectedRoadSegment)
						continue;
					if (m_StartSnappedJunction)
					{
						auto it = std::find(m_StartSnappedJunction->connectedRoadSegments.begin(), m_StartSnappedJunction->connectedRoadSegments.end(), roadSegment);
						if (it != m_StartSnappedJunction->connectedRoadSegments.end())
							continue;
					}
					if (m_EndSnappedJunction)
					{
						auto it = std::find(m_EndSnappedJunction->connectedRoadSegments.begin(), m_EndSnappedJunction->connectedRoadSegments.end(), roadSegment);
						if (it != m_EndSnappedJunction->connectedRoadSegments.end())
							continue;
					}

					float halfWidth = 0.5f * (roadSegment->Type[0]->boundingBoxM.z - roadSegment->Type[0]->boundingBoxL.z);

					std::array<std::array<glm::vec2, 3>, 2> oldRoadPolygon = Math::GetBoundingBoxOfBezierCurve(roadSegment->GetCurvePoints(), halfWidth);

					if (Math::CheckPolygonCollision(newRoadPolygon, oldRoadPolygon))
					{
						std::array<std::array<glm::vec2, 3>, (10 - 1) * 2> result = Math::GetBoundingPolygonOfBezierCurve<10, 10>(roadSegment->GetCurvePoints(), halfWidth);
						if (Math::CheckPolygonCollision(result, newRoadPolygon))
						{
							collisionIsRestricted = true;
							break;
						}
					}
				}
			}
			if (m_Scene->m_BuildingManager.restrictions[0] && restrictions[2])
			{
				for (Building* building : m_Scene->m_BuildingManager.GetBuildings())
				{
					Prefab* prefab = building->object->prefab;
					glm::vec2 pos{ building->object->position.x, building->object->position.z };
					glm::vec2 A = { prefab->boundingBoxL.x, prefab->boundingBoxL.z };
					glm::vec2 B = { prefab->boundingBoxL.x, prefab->boundingBoxM.z };
					glm::vec2 C = { prefab->boundingBoxM.x, prefab->boundingBoxL.z };
					glm::vec2 D = { prefab->boundingBoxM.x, prefab->boundingBoxM.z };

					float rot = building->object->rotation.y;
					A = Math::RotatePoint(A, rot) + pos;
					B = Math::RotatePoint(B, rot) + pos;
					C = Math::RotatePoint(C, rot) + pos;
					D = Math::RotatePoint(D, rot) + pos;

					std::array<std::array<glm::vec2, 3>, 2> polygonBuilding = {
						std::array<glm::vec2,3>{A, B, D},
						std::array<glm::vec2,3>{A, C, D}
					};
					building->object->tintColor = glm::vec4(1.0f);
					if (Math::CheckPolygonCollision(newRoadPolygon, polygonBuilding))
						building->object->tintColor = glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f };
				}
			}
			if (m_Scene->m_TreeManager.restrictions[0] && restrictions[2])
			{
				for (Object* tree : m_Scene->m_TreeManager.GetTrees())
				{
					Prefab* prefab = tree->prefab;
					glm::vec2 pos{ tree->position.x, tree->position.z };
					glm::vec2 A{ prefab->boundingBoxL.x * tree->scale.x, prefab->boundingBoxL.z * tree->scale.z };
					glm::vec2 B{ prefab->boundingBoxL.x * tree->scale.x, prefab->boundingBoxM.z * tree->scale.z };
					glm::vec2 C{ prefab->boundingBoxM.x * tree->scale.x, prefab->boundingBoxL.z * tree->scale.z };
					glm::vec2 D{ prefab->boundingBoxM.x * tree->scale.x, prefab->boundingBoxM.z * tree->scale.z };

					float rot = tree->rotation.y;
					A = Math::RotatePoint(A, rot) + pos;
					B = Math::RotatePoint(B, rot) + pos;
					C = Math::RotatePoint(C, rot) + pos;
					D = Math::RotatePoint(D, rot) + pos;

					std::array<std::array<glm::vec2, 3>, 2> polygonTree = {
						std::array<glm::vec2,3>{A, B, D},
						std::array<glm::vec2,3>{A, C, D}
					};

					tree->tintColor = glm::vec4(1.0f);
					if (Math::CheckPolygonCollision(newRoadPolygon, polygonTree))
						tree->tintColor = glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f };
				}
			}

			b_ConstructionRestricted |= angleIsRestricted;
			b_ConstructionRestricted |= collisionIsRestricted;
			DrawStraightGuidelines(m_ConstructionPositions[0], m_ConstructionPositions[3]);

			// do something about these
			/*if (m_StartSnappedRoadSegment != nullptr)
			{
				float snappedRoadSegmentPrefabLength = m_StartSnappedRoadSegment->Type[0]->boundingBoxM.x - m_StartSnappedRoadSegment->Type[0]->boundingBoxL.x;

				auto& rs = m_Scene->MainApplication->roads;
				size_t snappedRoadSegmentTypeIndex = std::distance(rs.begin(), std::find(rs.begin(), rs.end(), m_StartSnappedRoadSegment->Type[0]));

				m_StartSnappedRoadSegment->object->enabled = false;
				glm::vec3 R0I = m_ConstructionPositions[0] - m_StartSnappedRoad->GetStartPosition();
				glm::vec3 R1I = m_ConstructionPositions[0] - m_StartSnappedRoad->GetEndPosition();

				glm::vec3 normalizedR0I = glm::normalize(R0I);
				glm::vec3 normalizedR1I = glm::normalize(R1I);

				glm::vec3 rotationR0I = m_StartSnappedRoad->rotation;
				glm::vec3 rotationR1I = {
					0.0f,
					m_StartSnappedRoad->rotation.y + glm::radians(180.0f),
					-m_StartSnappedRoad->rotation.z
				};

				float availableR0ILength = std::max(glm::length(R0I) - snappedRoadPrefabLength, 0.0f);
				float availableR1ILength = std::max(glm::length(R1I) - snappedRoadPrefabLength, 0.0f);

				int countR0I = (int)(availableR0ILength / snappedRoadPrefabLength);
				int countR1I = (int)(availableR1ILength / snappedRoadPrefabLength);

				lengthIsRestricted |= restrictions[1] && countR0I < 2;
				lengthIsRestricted |= restrictions[1] && countR1I < 2;

				float scaleR0I = (availableR0ILength / snappedRoadPrefabLength) / countR0I;
				float scaleR1I = (availableR1ILength / snappedRoadPrefabLength) / countR1I;

				float scaledR0IRoadLength = availableR0ILength / countR0I;
				float scaledR1IRoadLength = availableR1ILength / countR1I;

				size_t prevIndex = m_GuidelinesInUse[snappedRoadTypeIndex];
				m_GuidelinesInUse[snappedRoadTypeIndex] += countR0I;
				m_GuidelinesInUse[snappedRoadTypeIndex] += countR1I;
				if (m_GuidelinesInUse[snappedRoadTypeIndex] > m_Guidelines[snappedRoadTypeIndex].size())
					for (size_t j = m_Guidelines[snappedRoadTypeIndex].size(); j < m_GuidelinesInUse[snappedRoadTypeIndex]; j++)
						m_Guidelines[snappedRoadTypeIndex].push_back(new Object(m_Scene->MainApplication->roads[snappedRoadTypeIndex][0], m_Scene->MainApplication->roads[snappedRoadTypeIndex][0]));

				for (size_t j = 0; j < countR0I; j++)
				{
					Object* roadG = m_Guidelines[snappedRoadTypeIndex][j + prevIndex];
					roadG->enabled = true;
					roadG->SetTransform(
						m_StartSnappedRoad->GetStartPosition() + normalizedR0I * (j * scaledR0IRoadLength) + glm::vec3{ 0.0f, 0.15f, 0.0f },
						glm::vec3{ 1.0f * scaleR0I, 1.0f, 1.0f },
						rotationR0I
					);
				}

				for (size_t j = 0; j < countR1I; j++)
				{
					Object* roadG = m_Guidelines[snappedRoadTypeIndex][j + prevIndex + countR0I];
					roadG->enabled = true;
					roadG->SetTransform(
						m_StartSnappedRoad->GetEndPosition() + normalizedR1I * (j * scaledR1IRoadLength) + glm::vec3{ 0.0f, 0.15f, 0.0f },
						glm::vec3{ 1.0f * scaleR1I, 1.0f, 1.0f },
						rotationR1I
					);
				}
			}
			*/
			/*if (m_EndSnappedRoadSegment != nullptr)
			{
				float snappedRoadPrefabLength = m_EndSnappedRoad->type[0]->boundingBoxM.x - m_EndSnappedRoad->type[0]->boundingBoxL.x;
				size_t snappedRoadTypeIndex = m_EndSnappedRoad->typeIndex;
				m_EndSnappedRoad->object->enabled = false;
				glm::vec3 R0I = m_ConstructionPositions[3] - m_EndSnappedRoad->GetStartPosition();
				glm::vec3 R1I = m_ConstructionPositions[3] - m_EndSnappedRoad->GetEndPosition();

				glm::vec3 normalizedR0I = glm::normalize(R0I);
				glm::vec3 normalizedR1I = glm::normalize(R1I);

				glm::vec3 rotationR0I = m_EndSnappedRoad->rotation;
				glm::vec3 rotationR1I = {
					0.0f,
					m_EndSnappedRoad->rotation.y + glm::radians(180.0f),
					-m_EndSnappedRoad->rotation.z
				};

				float availableR0ILength = std::max(glm::length(R0I) - snappedRoadPrefabLength, 0.0f);
				float availableR1ILength = std::max(glm::length(R1I) - snappedRoadPrefabLength, 0.0f);

				int countR0I = (int)(availableR0ILength / snappedRoadPrefabLength);
				int countR1I = (int)(availableR1ILength / snappedRoadPrefabLength);

				lengthIsRestricted |= restrictions[1] && countR0I < 2;
				lengthIsRestricted |= restrictions[1] && countR1I < 2;

				float scaleR0I = (availableR0ILength / snappedRoadPrefabLength) / countR0I;
				float scaleR1I = (availableR1ILength / snappedRoadPrefabLength) / countR1I;

				float scaledR0IRoadLength = availableR0ILength / countR0I;
				float scaledR1IRoadLength = availableR1ILength / countR1I;


				size_t prevIndex = m_GuidelinesInUse[snappedRoadTypeIndex];
				m_GuidelinesInUse[snappedRoadTypeIndex] += countR0I;
				m_GuidelinesInUse[snappedRoadTypeIndex] += countR1I;
				if (m_GuidelinesInUse[snappedRoadTypeIndex] > m_Guidelines[snappedRoadTypeIndex].size())
					for (size_t j = m_Guidelines[snappedRoadTypeIndex].size(); j < m_GuidelinesInUse[snappedRoadTypeIndex]; j++)
						m_Guidelines[snappedRoadTypeIndex].push_back(new Object(m_Scene->MainApplication->roads[snappedRoadTypeIndex][0], m_Scene->MainApplication->roads[snappedRoadTypeIndex][0]));

				for (size_t j = 0; j < countR0I; j++)
				{
					Object* roadG = m_Guidelines[snappedRoadTypeIndex][j + prevIndex];
					roadG->enabled = true;
					roadG->SetTransform(
						m_EndSnappedRoad->GetStartPosition() + normalizedR0I * (j * scaledR0IRoadLength) + glm::vec3{ 0.0f, 0.15f, 0.0f },
						glm::vec3{ 1.0f * scaleR0I, 1.0f, 1.0f },
						rotationR0I
					);
				}

				for (size_t j = 0; j < countR1I; j++)
				{
					Object* roadG = m_Guidelines[snappedRoadTypeIndex][j + prevIndex + countR0I];
					roadG->enabled = true;
					roadG->SetTransform(
						m_EndSnappedRoad->GetEndPosition() + normalizedR1I * (j * scaledR1IRoadLength) + glm::vec3{ 0.0f, 0.15f, 0.0f },
						glm::vec3{ 1.0f * scaleR1I, 1.0f, 1.0f },
						rotationR1I
					);
				}
			}
			*/
		}
	}
	void RoadManager::OnUpdate_QuadraticCurve(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
	{
		Prefab* selectedRoad = m_Scene->MainApplication->roads[m_Type][0];
		float roadPrefabWidth = selectedRoad->boundingBoxM.z - selectedRoad->boundingBoxL.z;
		float roadPrefabLength = selectedRoad->boundingBoxM.x - selectedRoad->boundingBoxL.x;

		if (m_ConstructionPhase == 0)
		{
			if (snapOptions[4])
			{
				prevLocation.x = prevLocation.x - std::fmod(prevLocation.x + 0.25f, 0.5f) + 0.25f;
				prevLocation.z = prevLocation.z - std::fmod(prevLocation.z + 0.25f, 0.5f) - 0.25f;
			}
			if (snapOptions[0])
			{
				SnapInformation snapInformation = CheckSnapping(cameraPosition, cameraDirection);
				prevLocation = snapInformation.snapped ? snapInformation.location : prevLocation;
				b_ConstructionStartSnapped = snapInformation.snapped;
				m_StartSnappedJunction = snapInformation.junction;
				m_StartSnappedEnd = snapInformation.end;
				m_StartSnappedRoadSegment = snapInformation.roadSegment;
				m_StartSnappedRoadSegmentT = snapInformation.roadT;
				m_StartSnappedRoadSegmentTDelta = snapInformation.roadTDelta;
			}
			m_ConstructionPositions[0] = prevLocation;

			m_GuidelinesStart->SetTransform(prevLocation + glm::vec3{ 0.0f, 0.15f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, glm::radians(180.0f), 0.0f });
			m_GuidelinesEnd->SetTransform(prevLocation + glm::vec3{ 0.0f, 0.15f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f });
		}
		else if (m_ConstructionPhase == 1)
		{
			for (std::vector<Object*>& os : m_Guidelines)
				for (Object* rsg : os)
					rsg->enabled = false;

			for (size_t& inUse : m_GuidelinesInUse)
				inUse = 0;

			b_ConstructionRestricted = false;
			if (snapOptions[4])
			{
				prevLocation.x = prevLocation.x - std::fmod(prevLocation.x + 0.25f, 0.5f) + 0.25f;
				prevLocation.z = prevLocation.z - std::fmod(prevLocation.z + 0.25f, 0.5f) + 0.25f;
			}

			m_ConstructionPositions[1] = prevLocation;
			m_ConstructionPositions[2] = prevLocation;
			m_ConstructionPositions[3] = prevLocation;

			bool angleIsRestricted = false;
			if (restrictions[0])
			{
				if (m_StartSnappedJunction)
				{
					for (RoadSegment* roadSegment : m_StartSnappedJunction->connectedRoadSegments)
					{
						glm::vec3 directionOldRoadSegment = roadSegment->ConnectedObjectAtStart.junction == m_StartSnappedJunction ? roadSegment->GetStartDirection() : roadSegment->GetEndDirection();
						directionOldRoadSegment.y = 0;
						directionOldRoadSegment = glm::normalize(directionOldRoadSegment);

						glm::vec3 directionNewRoadSegment = prevLocation - m_ConstructionPositions[0];
						directionNewRoadSegment.y = 0;
						directionNewRoadSegment = glm::normalize(directionNewRoadSegment);

						float angle = glm::acos(glm::dot(directionOldRoadSegment, directionNewRoadSegment));

						if (angle < 0.5f)
						{
							angleIsRestricted = true;
							break;
						}
					}
				}
				else if (m_StartSnappedEnd)
				{
					RoadSegment* roadSegment = m_StartSnappedEnd->connectedRoadSegment;

					glm::vec3 directionOldRoadSegment = roadSegment->ConnectedObjectAtStart.end == m_StartSnappedEnd ? roadSegment->GetStartDirection() : roadSegment->GetEndDirection();
					directionOldRoadSegment.y = 0;
					directionOldRoadSegment = glm::normalize(directionOldRoadSegment);

					glm::vec3 directionNewRoadSegment = prevLocation - m_ConstructionPositions[0];
					directionNewRoadSegment.y = 0;
					directionNewRoadSegment = glm::normalize(directionNewRoadSegment);

					float angle = glm::acos(glm::dot(directionOldRoadSegment, directionNewRoadSegment));

					angleIsRestricted = angle < 0.5f;
				}
				else if (m_StartSnappedRoadSegment)
				{
					glm::vec3 prevPoint = Math::CubicCurve(m_StartSnappedRoadSegment->GetCurvePoints(), m_StartSnappedRoadSegmentT - m_StartSnappedRoadSegmentTDelta);
					glm::vec3 directionOldRoadSegment = prevLocation - prevPoint;

					directionOldRoadSegment.y = 0;
					directionOldRoadSegment = glm::normalize(directionOldRoadSegment);

					glm::vec3 directionNewRoadSegment = prevLocation - m_ConstructionPositions[0];
					directionNewRoadSegment.y = 0;
					directionNewRoadSegment = glm::normalize(directionNewRoadSegment);

					float angle = glm::acos(glm::dot(directionOldRoadSegment, directionNewRoadSegment));

					angleIsRestricted = angle < 0.5f || angle > 2.63f;
				}
			}

			glm::vec3 AB = m_ConstructionPositions[3] - m_ConstructionPositions[0];

			float rotationOffset = AB.x < 0.0f ? 180.0f : 0.0f;
			float rotationStart = glm::atan(-AB.z / AB.x) + glm::radians(180.0f + rotationOffset);
			float rotationEnd = glm::atan(-AB.z / AB.x) + glm::radians(rotationOffset);

			//use this to make roundabouts
			if (snapOptions[1] && glm::length(AB) > 0.5f)
			{
				float length = glm::length(AB);
				length = length - std::fmod(length, roadPrefabLength);
				AB = length * glm::normalize(AB);
				m_ConstructionPositions[1] = m_ConstructionPositions[0] + AB;
				m_ConstructionPositions[2] = m_ConstructionPositions[0] + AB;
				m_ConstructionPositions[3] = m_ConstructionPositions[0] + AB;
			}

			if (snapOptions[2] && glm::length(AB) > 0.5f)
			{
				if (
					!m_EndSnappedJunction &&
					!m_EndSnappedRoadSegment &&
					!m_EndSnappedEnd
					)
				{
					m_ConstructionPositions[1].y = m_ConstructionPositions[0].y;
					m_ConstructionPositions[2].y = m_ConstructionPositions[0].y;
					m_ConstructionPositions[3].y = m_ConstructionPositions[0].y;
				}
			}

			if (snapOptions[3] && glm::length(AB) > 0.5f)
			{
				if (m_StartSnappedEnd)
				{
					float snappedRoadRotationY = glm::degrees(m_StartSnappedEnd->object->rotation.y) + 180.0f;
					float newRoadRotationY = glm::degrees(rotationEnd);
					float angle = std::fmod(snappedRoadRotationY - newRoadRotationY + 720.0f, 360.0f);

					float newAngle = 0.0f;
					if (angle < 32.0f)
						newAngle = 30.0f;
					else if (angle > 80.0f && angle < 100.0f)
						newAngle = 90.0f;
					else if (angle > 170.0f && angle < 190.0f)
						newAngle = 180.0f;
					else if (angle > 260.0f && angle < 280.0f)
						newAngle = 270.0f;
					else if (angle > 328.0f)
						newAngle = 330.0f;
					else if (Input::IsKeyPressed(KeyCode::LeftControl))
						newAngle = angle + 2.5f - std::fmod(angle + 2.5f, 5.0f);
					else
						newAngle = angle;

					AB = glm::rotate(AB, glm::radians(angle - newAngle), { 0.0f, 1.0f, 0.0f });
					m_ConstructionPositions[3] = m_ConstructionPositions[0] + AB;
				}
				else if (m_StartSnappedRoadSegment)
				{
					float snappedRoadRotationY = glm::degrees(m_StartSnappedRoadSegment->GetStartRotation().y);
					float newRoadRotationY = glm::degrees(rotationEnd);
					float angle = std::fmod(snappedRoadRotationY - newRoadRotationY + 720.0f, 180.0f);

					float newAngle = 0.0f;
					if (angle < 32.0f)
						newAngle = 30.0f;
					else if (angle > 80.0f && angle < 100.0f)
						newAngle = 90.0f;
					else if (angle > 148.0f && angle < 160.0f)
						newAngle = 150.0f;
					else if (Input::IsKeyPressed(KeyCode::LeftControl))
						newAngle = angle + 2.5f - std::fmod(angle + 2.5f, 5.0f);
					else
						newAngle = angle;

					AB = glm::rotate(AB, -glm::radians(angle - newAngle), { 0.0f, 1.0f, 0.0f });
					m_ConstructionPositions[3] = m_ConstructionPositions[0] + AB;
				}
				else if (m_StartSnappedJunction)
				{
					float newRoadRotationY = glm::degrees(rotationEnd);
					float smallestAngle = 180.0f;
					for (RoadSegment* roadSegment : m_StartSnappedJunction->connectedRoadSegments)
					{
						float snappedRoadRotationY = m_StartSnappedJunction == roadSegment->ConnectedObjectAtStart.junction ? glm::degrees(roadSegment->GetStartRotation().y) : glm::degrees(roadSegment->GetEndRotation().y);
						float angle = std::fmod(snappedRoadRotationY - newRoadRotationY + 720.0f, 180.0f);
						smallestAngle = std::min(smallestAngle, angle);
					}
					float newAngle = 0.0f;
					if (smallestAngle < 32.0f)
						newAngle = 30.0f;
					else if (smallestAngle > 80.0f && smallestAngle < 100.0f)
						newAngle = 90.0f;
					else if (Input::IsKeyPressed(KeyCode::LeftControl))
						newAngle = smallestAngle + 2.5f - std::fmod(smallestAngle + 2.5f, 5.0f);
					else
						newAngle = smallestAngle;

					AB = glm::rotate(AB, -glm::radians(smallestAngle - newAngle), { 0.0f, 1.0f, 0.0f });
					m_ConstructionPositions[3] = m_ConstructionPositions[0] + AB;
				}
				else if (Input::IsKeyPressed(KeyCode::LeftControl))
				{
					float angle = std::fmod(glm::degrees(rotationEnd) + 720.0f, 360.0f);
					float newAngle = angle + 2.5f - std::fmod(angle + 2.5f, 5.0f);

					AB = glm::rotate(AB, -glm::radians(angle - newAngle), { 0.0f, 1.0f, 0.0f });
					m_ConstructionPositions[3] = m_ConstructionPositions[0] + AB;
				}
			}

			glm::vec2 A = glm::vec2{ m_ConstructionPositions[0].x, m_ConstructionPositions[0].z };
			glm::vec2 D = glm::vec2{ m_ConstructionPositions[3].x, m_ConstructionPositions[3].z };
			glm::vec2 AD = roadPrefabWidth * 0.5f * glm::normalize(D - A);
			D += AD;
			if (!(m_StartSnappedEnd || m_StartSnappedJunction || m_StartSnappedRoadSegment))
				A -= AD;

			AD = glm::vec2{ -AD.y , AD.x };

			glm::vec2 P1 = A + AD;
			glm::vec2 P2 = A - AD;
			glm::vec2 P3 = D + AD;
			glm::vec2 P4 = D - AD;

			std::array<std::array<glm::vec2, 3>, 2> newRoadPolygon = {
					std::array<glm::vec2,3>{ P1, P2, P3},
					std::array<glm::vec2,3>{ P2, P3, P4}
			};
			bool collisionIsRestricted = false; // Just for visual
			if (restrictions[2])
			{
				for (RoadSegment* roadSegment : m_RoadSegments)
				{
					if (roadSegment == m_StartSnappedRoadSegment)
						continue;
					if (m_StartSnappedEnd && roadSegment == m_StartSnappedEnd->connectedRoadSegment)
						continue;
					if (m_StartSnappedJunction)
					{
						auto it = std::find(m_StartSnappedJunction->connectedRoadSegments.begin(), m_StartSnappedJunction->connectedRoadSegments.end(), roadSegment);
						if (it != m_StartSnappedJunction->connectedRoadSegments.end())
							continue;
					}

					float halfWidth = 0.5f * (roadSegment->Type[0]->boundingBoxM.z - roadSegment->Type[0]->boundingBoxL.z);

					std::array<std::array<glm::vec2, 3>, 2> oldRoadPolygon = Math::GetBoundingBoxOfBezierCurve(roadSegment->GetCurvePoints(), halfWidth);

					if (Math::CheckPolygonCollision(newRoadPolygon, oldRoadPolygon))
					{
						std::array<std::array<glm::vec2, 3>, (10 - 1) * 2> result = Math::GetBoundingPolygonOfBezierCurve<10, 10>(roadSegment->GetCurvePoints(), halfWidth);
						if (Math::CheckPolygonCollision(result, newRoadPolygon))
						{
							collisionIsRestricted = true;
							break;
						}
					}
				}
			}
			if (m_Scene->m_BuildingManager.restrictions[0] && restrictions[2])
			{
				for (Building* building : m_Scene->m_BuildingManager.GetBuildings())
				{
					Prefab* prefab = building->object->prefab;
					glm::vec2 pos{ building->object->position.x, building->object->position.z };
					glm::vec2 A = { prefab->boundingBoxL.x, prefab->boundingBoxL.z };
					glm::vec2 B = { prefab->boundingBoxL.x, prefab->boundingBoxM.z };
					glm::vec2 C = { prefab->boundingBoxM.x, prefab->boundingBoxL.z };
					glm::vec2 D = { prefab->boundingBoxM.x, prefab->boundingBoxM.z };

					float rot = building->object->rotation.y;
					A = Math::RotatePoint(A, rot) + pos;
					B = Math::RotatePoint(B, rot) + pos;
					C = Math::RotatePoint(C, rot) + pos;
					D = Math::RotatePoint(D, rot) + pos;

					std::array<std::array<glm::vec2, 3>, 2> polygonBuilding = {
						std::array<glm::vec2,3>{A, B, D},
						std::array<glm::vec2,3>{A, C, D}
					};
					building->object->tintColor = glm::vec4(1.0f);
					if (Math::CheckPolygonCollision(newRoadPolygon, polygonBuilding))
						building->object->tintColor = glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f };
				}
			}
			if (m_Scene->m_TreeManager.restrictions[0] && restrictions[2])
			{
				for (Object* tree : m_Scene->m_TreeManager.GetTrees())
				{
					Prefab* prefab = tree->prefab;
					glm::vec2 pos{ tree->position.x, tree->position.z };
					glm::vec2 A = { prefab->boundingBoxL.x * tree->scale.x, prefab->boundingBoxL.z * tree->scale.z };
					glm::vec2 B = { prefab->boundingBoxL.x * tree->scale.x, prefab->boundingBoxM.z * tree->scale.z };
					glm::vec2 C = { prefab->boundingBoxM.x * tree->scale.x, prefab->boundingBoxL.z * tree->scale.z };
					glm::vec2 D = { prefab->boundingBoxM.x * tree->scale.x, prefab->boundingBoxM.z * tree->scale.z };

					float rot = tree->rotation.y;
					A = Math::RotatePoint(A, rot) + pos;
					B = Math::RotatePoint(B, rot) + pos;
					C = Math::RotatePoint(C, rot) + pos;
					D = Math::RotatePoint(D, rot) + pos;

					std::array<std::array<glm::vec2, 3>, 2> polygonTree = {
						std::array<glm::vec2,3>{A, B, D},
						std::array<glm::vec2,3>{A, C, D}
					};

					tree->tintColor = glm::vec4(1.0f);
					if (Math::CheckPolygonCollision(newRoadPolygon, polygonTree))
						tree->tintColor = glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f };
				}
			}

			b_ConstructionRestricted |= angleIsRestricted;
			b_ConstructionRestricted |= collisionIsRestricted;
			DrawStraightGuidelines(m_ConstructionPositions[0], m_ConstructionPositions[3]);

			// do something about this
			/*if (m_StartSnappedRoadSegment != nullptr)
			{
				float snappedRoadSegmentPrefabLength = m_StartSnappedRoadSegment->Type[0]->boundingBoxM.x - m_StartSnappedRoadSegment->Type[0]->boundingBoxL.x;

				auto& rs = m_Scene->MainApplication->roads;
				size_t snappedRoadSegmentTypeIndex = std::distance(rs.begin(), std::find(rs.begin(), rs.end(), m_StartSnappedRoadSegment->Type[0]));

				m_StartSnappedRoadSegment->object->enabled = false;
				glm::vec3 R0I = m_ConstructionPositions[0] - m_StartSnappedRoad->GetStartPosition();
				glm::vec3 R1I = m_ConstructionPositions[0] - m_StartSnappedRoad->GetEndPosition();

				glm::vec3 normalizedR0I = glm::normalize(R0I);
				glm::vec3 normalizedR1I = glm::normalize(R1I);

				glm::vec3 rotationR0I = m_StartSnappedRoad->rotation;
				glm::vec3 rotationR1I = {
					0.0f,
					m_StartSnappedRoad->rotation.y + glm::radians(180.0f),
					-m_StartSnappedRoad->rotation.z
				};

				float availableR0ILength = std::max(glm::length(R0I) - snappedRoadPrefabLength, 0.0f);
				float availableR1ILength = std::max(glm::length(R1I) - snappedRoadPrefabLength, 0.0f);

				int countR0I = (int)(availableR0ILength / snappedRoadPrefabLength);
				int countR1I = (int)(availableR1ILength / snappedRoadPrefabLength);

				lengthIsRestricted |= restrictions[1] && countR0I < 2;
				lengthIsRestricted |= restrictions[1] && countR1I < 2;

				float scaleR0I = (availableR0ILength / snappedRoadPrefabLength) / countR0I;
				float scaleR1I = (availableR1ILength / snappedRoadPrefabLength) / countR1I;

				float scaledR0IRoadLength = availableR0ILength / countR0I;
				float scaledR1IRoadLength = availableR1ILength / countR1I;

				size_t prevIndex = m_GuidelinesInUse[snappedRoadTypeIndex];
				m_GuidelinesInUse[snappedRoadTypeIndex] += countR0I;
				m_GuidelinesInUse[snappedRoadTypeIndex] += countR1I;
				if (m_GuidelinesInUse[snappedRoadTypeIndex] > m_Guidelines[snappedRoadTypeIndex].size())
					for (size_t j = m_Guidelines[snappedRoadTypeIndex].size(); j < m_GuidelinesInUse[snappedRoadTypeIndex]; j++)
						m_Guidelines[snappedRoadTypeIndex].push_back(new Object(m_Scene->MainApplication->roads[snappedRoadTypeIndex][0], m_Scene->MainApplication->roads[snappedRoadTypeIndex][0]));

				for (size_t j = 0; j < countR0I; j++)
				{
					Object* roadG = m_Guidelines[snappedRoadTypeIndex][j + prevIndex];
					roadG->enabled = true;
					roadG->SetTransform(
						m_StartSnappedRoad->GetStartPosition() + normalizedR0I * (j * scaledR0IRoadLength) + glm::vec3{ 0.0f, 0.15f, 0.0f },
						glm::vec3{ 1.0f * scaleR0I, 1.0f, 1.0f },
						rotationR0I
					);
				}

				for (size_t j = 0; j < countR1I; j++)
				{
					Object* roadG = m_Guidelines[snappedRoadTypeIndex][j + prevIndex + countR0I];
					roadG->enabled = true;
					roadG->SetTransform(
						m_StartSnappedRoad->GetEndPosition() + normalizedR1I * (j * scaledR1IRoadLength) + glm::vec3{ 0.0f, 0.15f, 0.0f },
						glm::vec3{ 1.0f * scaleR1I, 1.0f, 1.0f },
						rotationR1I
					);
				}
			}
			*/
		}
		else if (m_ConstructionPhase == 2)
		{
			for (std::vector<Object*>& os : m_Guidelines)
				for (Object* rsg : os)
					rsg->enabled = false;

			for (size_t& inUse : m_GuidelinesInUse)
				inUse = 0;
			b_ConstructionRestricted = false;
			m_ConstructionPositions[3] = prevLocation;
			{
				glm::vec2 Cd{ m_ConstructionPositions[1].x, m_ConstructionPositions[1].z };
				glm::vec2 A{ m_ConstructionPositions[0].x, m_ConstructionPositions[0].z };
				glm::vec2 B{ m_ConstructionPositions[3].x, m_ConstructionPositions[3].z };
				glm::vec2 ray = glm::normalize(Cd - A);
				glm::vec2 AB = B - A;
				float d = glm::dot(AB, AB) / (2.0f * glm::dot(AB, ray));
				glm::vec2 C = A + d * ray;
				std::cout << glm::length(C - A) << ", " << glm::length(C - B) << std::endl;
				if (d < 200.0f && d > 0.0f)
				{
					m_ConstructionPositions[2].x = C.x;
					m_ConstructionPositions[2].z = C.y;
				}
			}

			float l = glm::length(m_ConstructionPositions[3] - m_ConstructionPositions[0]);
			size_t count = 1;
			while (l > roadPrefabLength)
			{
				count *= 2;
				glm::vec3 p = Math::CubicCurve<float>(m_ConstructionPositions, 1.0f / count);
				l = glm::length(p - m_ConstructionPositions[0]);
			}
			if (count > 1) count /= 2;
			while (l > roadPrefabLength)
			{
				count++;
				glm::vec3 p = Math::CubicCurve<float>(m_ConstructionPositions, 1.0f / count);
				l = glm::length(p - m_ConstructionPositions[0]);
			}
			if (count > 1) count--;

			std::array<std::array<glm::vec2, 3>, 2> newRoadBoundingBox = Math::GetBoundingBoxOfBezierCurve(m_ConstructionPositions, roadPrefabWidth * 0.5f);
			std::array<std::array<glm::vec2, 3>, (10 - 1) * 2> newRoadBoundingPolygon = Math::GetBoundingPolygonOfBezierCurve<10, 10>(m_ConstructionPositions, roadPrefabWidth * 0.5f);

			bool collisionIsRestricted = false;
			if (restrictions[2])
			{
				for (RoadSegment* roadSegment : m_RoadSegments)
				{
					if (roadSegment == m_EndSnappedRoadSegment || roadSegment == m_StartSnappedRoadSegment)
						continue;
					if (m_StartSnappedEnd && roadSegment == m_StartSnappedEnd->connectedRoadSegment)
						continue;
					if (m_EndSnappedEnd && roadSegment == m_EndSnappedEnd->connectedRoadSegment)
						continue;
					if (m_StartSnappedJunction)
					{
						auto it = std::find(m_StartSnappedJunction->connectedRoadSegments.begin(), m_StartSnappedJunction->connectedRoadSegments.end(), roadSegment);
						if (it != m_StartSnappedJunction->connectedRoadSegments.end())
							continue;
					}
					if (m_EndSnappedJunction)
					{
						auto it = std::find(m_EndSnappedJunction->connectedRoadSegments.begin(), m_EndSnappedJunction->connectedRoadSegments.end(), roadSegment);
						if (it != m_EndSnappedJunction->connectedRoadSegments.end())
							continue;
					}

					float halfWidth = 0.5f * (roadSegment->Type[0]->boundingBoxM.z - roadSegment->Type[0]->boundingBoxL.z);

					std::array<std::array<glm::vec2, 3>, 2> oldRoadBoundingBox = Math::GetBoundingBoxOfBezierCurve(roadSegment->GetCurvePoints(), halfWidth);

					if (Math::CheckPolygonCollision(newRoadBoundingBox, oldRoadBoundingBox))
					{
						if (Math::CheckPolygonCollision(newRoadBoundingPolygon, oldRoadBoundingBox))
						{
							std::array<std::array<glm::vec2, 3>, (10 - 1) * 2> oldRoadBoundingPolygon = Math::GetBoundingPolygonOfBezierCurve<10, 10>(roadSegment->GetCurvePoints(), halfWidth);
							if (Math::CheckPolygonCollision(newRoadBoundingBox, oldRoadBoundingPolygon))
							{
								if (Math::CheckPolygonCollision(newRoadBoundingPolygon, oldRoadBoundingPolygon))
								{
									collisionIsRestricted = true;
									break;
								}
							}
						}
					}
				}
			}
			if (m_Scene->m_BuildingManager.restrictions[0] && restrictions[2])
			{
				for (Building* building : m_Scene->m_BuildingManager.GetBuildings())
				{
					Prefab* prefab = building->object->prefab;
					glm::vec2 pos{ building->object->position.x, building->object->position.z };
					glm::vec2 A = { prefab->boundingBoxL.x, prefab->boundingBoxL.z };
					glm::vec2 B = { prefab->boundingBoxL.x, prefab->boundingBoxM.z };
					glm::vec2 C = { prefab->boundingBoxM.x, prefab->boundingBoxL.z };
					glm::vec2 D = { prefab->boundingBoxM.x, prefab->boundingBoxM.z };

					float rot = building->object->rotation.y;
					A = Math::RotatePoint(A, rot) + pos;
					B = Math::RotatePoint(B, rot) + pos;
					C = Math::RotatePoint(C, rot) + pos;
					D = Math::RotatePoint(D, rot) + pos;

					std::array<std::array<glm::vec2, 3>, 2> polygonBuilding = {
						std::array<glm::vec2,3>{A, B, D},
						std::array<glm::vec2,3>{A, C, D}
					};
					building->object->tintColor = glm::vec4(1.0f);
					if (Math::CheckPolygonCollision(newRoadBoundingBox, polygonBuilding))
						if (Math::CheckPolygonCollision(newRoadBoundingPolygon, polygonBuilding))
							building->object->tintColor = glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f };
				}
			}
			if (m_Scene->m_TreeManager.restrictions[0] && restrictions[2])
			{
				for (Object* tree : m_Scene->m_TreeManager.GetTrees())
				{
					Prefab* prefab = tree->prefab;
					glm::vec2 pos{ tree->position.x, tree->position.z };
					glm::vec2 A = { prefab->boundingBoxL.x * tree->scale.x, prefab->boundingBoxL.z * tree->scale.z };
					glm::vec2 B = { prefab->boundingBoxL.x * tree->scale.x, prefab->boundingBoxM.z * tree->scale.z };
					glm::vec2 C = { prefab->boundingBoxM.x * tree->scale.x, prefab->boundingBoxL.z * tree->scale.z };
					glm::vec2 D = { prefab->boundingBoxM.x * tree->scale.x, prefab->boundingBoxM.z * tree->scale.z };

					float rot = tree->rotation.y;
					A = Math::RotatePoint(A, rot) + pos;
					B = Math::RotatePoint(B, rot) + pos;
					C = Math::RotatePoint(C, rot) + pos;
					D = Math::RotatePoint(D, rot) + pos;

					std::array<std::array<glm::vec2, 3>, 2> polygonTree = {
						std::array<glm::vec2,3>{A, B, D},
						std::array<glm::vec2,3>{A, C, D}
					};

					tree->tintColor = glm::vec4(1.0f);
					if (Math::CheckPolygonCollision(newRoadBoundingBox, polygonTree))
						if (Math::CheckPolygonCollision(newRoadBoundingPolygon, polygonTree))
							tree->tintColor = glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f };
				}
			}

			b_ConstructionRestricted |= collisionIsRestricted;

			glm::vec3 AB1 = m_ConstructionPositions[1] - m_ConstructionPositions[0];
			glm::vec3 AB2 = m_ConstructionPositions[2] - m_ConstructionPositions[3];

			float rotationOffset1 = AB1.x < 0.0f ? 180.0f : 0.0f;
			float rotationOffset2 = AB2.x < 0.0f ? 180.0f : 0.0f;

			float rotationStart = glm::atan(-AB1.z / AB1.x) + glm::radians(rotationOffset1 + 180.0f);
			float rotationEnd = glm::atan(-AB2.z / AB2.x) + glm::radians(rotationOffset2 + 180.0f);

			m_GuidelinesStart->enabled = !b_ConstructionStartSnapped;
			m_GuidelinesEnd->enabled = !b_ConstructionEndSnapped;

			m_GuidelinesStart->SetTransform(m_ConstructionPositions[0] + glm::vec3{ 0.0f, 0.15f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, rotationStart, 0.0f });
			m_GuidelinesEnd->SetTransform(m_ConstructionPositions[3] + glm::vec3{ 0.0f, 0.15f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, rotationEnd, 0.0f });

			for (size_t& inUse : m_GuidelinesInUse)
				inUse = 0;
			m_GuidelinesInUse[m_Type] += count;

			if (m_GuidelinesInUse[m_Type] > m_Guidelines[m_Type].size())
				for (size_t j = m_Guidelines[m_Type].size(); j < m_GuidelinesInUse[m_Type]; j++)
					m_Guidelines[m_Type].push_back(new Object(m_Scene->MainApplication->roads[m_Type][0], m_Scene->MainApplication->roads[m_Type][0]));

			glm::vec3 p1 = m_ConstructionPositions[0];
			for (int c = 0; c < count; c++)
			{
				glm::vec3 p2 = Math::CubicCurve<float>(std::array<glm::vec3, 4>{
					m_ConstructionPositions[0],
						(m_ConstructionPositions[2] + m_ConstructionPositions[0]) * 0.5f,
						(m_ConstructionPositions[2] + m_ConstructionPositions[3]) * 0.5f,
						m_ConstructionPositions[3],
				},
					(c + 1.0f) / count);
				glm::vec3 vec1 = p2 - p1;
				float length = glm::length(vec1);
				glm::vec3 dir1 = vec1 / length;

				float scale = length / roadPrefabLength;
				float rot1 = glm::acos(dir1.x) * ((float)(dir1.z < 0.0f) * 2.0f - 1.0f);

				Object* roadSG = m_Guidelines[m_Type][c];
				roadSG->enabled = true;
				roadSG->SetTransform(
					p1 + glm::vec3{ 0.0f, 0.15f, 0.0f },
					glm::vec3{ scale, 1.0f, 1.0f },
					glm::vec3{ 0.0f, rot1, 0.0f }
				);

				p1 = p2;
			}

			for (std::vector<Object*>& os : m_Guidelines)
				for (Object* rg : os)
					rg->tintColor = b_ConstructionRestricted ? glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f } : glm::vec4(1.0f);
		}
	}
	void RoadManager::OnUpdate_CubicCurve(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
	{
		Prefab* selectedRoad = m_Scene->MainApplication->roads[m_Type][0];
		float roadPrefabWidth = selectedRoad->boundingBoxM.z - selectedRoad->boundingBoxL.z;
		float roadPrefabLength = selectedRoad->boundingBoxM.x - selectedRoad->boundingBoxL.x;

		for (std::vector<Object*>& os : m_Guidelines)
			for (Object* rg : os)
				rg->enabled = false;

		if (m_ConstructionPhase == 0)
		{
			if (snapOptions[4])
			{
				prevLocation.x = prevLocation.x - std::fmod(prevLocation.x + 0.25f, 0.5f) + 0.25f;
				prevLocation.z = prevLocation.z - std::fmod(prevLocation.z + 0.25f, 0.5f) - 0.25f;
			}
			if (snapOptions[0])
			{
				SnapInformation snapInformation = CheckSnapping(cameraPosition, cameraDirection);
				prevLocation = snapInformation.snapped ? snapInformation.location : prevLocation;
				b_ConstructionStartSnapped = snapInformation.snapped;
				m_StartSnappedJunction = snapInformation.junction;
				m_StartSnappedEnd = snapInformation.end;
				m_StartSnappedRoadSegment = snapInformation.roadSegment;
				m_StartSnappedRoadSegmentT = snapInformation.roadT;
				m_StartSnappedRoadSegmentTDelta = snapInformation.roadTDelta;
			}
			m_ConstructionPositions[cubicCurveOrder[0]] = prevLocation;

			m_GuidelinesStart->SetTransform(prevLocation + glm::vec3{ 0.0f, 0.15f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, glm::radians(180.0f), 0.0f });
			m_GuidelinesEnd->SetTransform(prevLocation + glm::vec3{ 0.0f, 0.15f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f });
		}
		else if (m_ConstructionPhase == 1)
		{
			b_ConstructionRestricted = false;
			if (snapOptions[4])
			{
				prevLocation.x = prevLocation.x - std::fmod(prevLocation.x + 0.25f, 0.5f) + 0.25f;
				prevLocation.z = prevLocation.z - std::fmod(prevLocation.z + 0.25f, 0.5f) + 0.25f;
			}

			m_ConstructionPositions[cubicCurveOrder[1]] = prevLocation;

			bool collisionIsRestricted = false;

			glm::vec3 AB = m_ConstructionPositions[cubicCurveOrder[1]] - m_ConstructionPositions[0];

			float rotationOffset = AB.x < 0.0f ? 180.0f : 0.0f;
			float rotationStart = glm::atan(-AB.z / AB.x) + glm::radians(180.0f + rotationOffset);
			float rotationEnd = glm::atan(-AB.z / AB.x) + glm::radians(rotationOffset);

			if (snapOptions[1] && glm::length(AB) > 0.5f)
			{
				float length = glm::length(AB);
				length = length - std::fmod(length, roadPrefabLength);
				AB = length * glm::normalize(AB);
				m_ConstructionPositions[cubicCurveOrder[1]] = m_ConstructionPositions[cubicCurveOrder[0]] + AB;
			}

			if (snapOptions[2] && glm::length(AB) > 0.5f)
			{
				if (
					!m_EndSnappedJunction &&
					!m_EndSnappedRoadSegment &&
					!m_EndSnappedEnd
					)
					m_ConstructionPositions[cubicCurveOrder[1]].y = m_ConstructionPositions[cubicCurveOrder[0]].y;
			}

			if (snapOptions[3] && glm::length(AB) > 0.5f)
			{
				if (m_StartSnappedEnd)
				{
					float snappedRoadRotationY = glm::degrees(m_StartSnappedEnd->object->rotation.y) + 180.0f;
					float newRoadRotationY = glm::degrees(rotationEnd);
					float angle = std::fmod(snappedRoadRotationY - newRoadRotationY + 720.0f, 360.0f);

					float newAngle = 0.0f;
					if (angle < 32.0f)
						newAngle = 30.0f;
					else if (angle > 80.0f && angle < 100.0f)
						newAngle = 90.0f;
					else if (angle > 170.0f && angle < 190.0f)
						newAngle = 180.0f;
					else if (angle > 260.0f && angle < 280.0f)
						newAngle = 270.0f;
					else if (angle > 328.0f)
						newAngle = 330.0f;
					else if (Input::IsKeyPressed(KeyCode::LeftControl))
						newAngle = angle + 2.5f - std::fmod(angle + 2.5f, 5.0f);
					else
						newAngle = angle;

					AB = glm::rotate(AB, glm::radians(angle - newAngle), { 0.0f, 1.0f, 0.0f });
					m_ConstructionPositions[cubicCurveOrder[1]] = m_ConstructionPositions[cubicCurveOrder[0]] + AB;
				}
				else if (m_StartSnappedRoadSegment)
				{
					float snappedRoadRotationY = glm::degrees(m_StartSnappedRoadSegment->GetStartRotation().y);
					float newRoadRotationY = glm::degrees(rotationEnd);
					float angle = std::fmod(snappedRoadRotationY - newRoadRotationY + 720.0f, 180.0f);

					float newAngle = 0.0f;
					if (angle < 32.0f)
						newAngle = 30.0f;
					else if (angle > 80.0f && angle < 100.0f)
						newAngle = 90.0f;
					else if (angle > 148.0f && angle < 160.0f)
						newAngle = 150.0f;
					else if (Input::IsKeyPressed(KeyCode::LeftControl))
						newAngle = angle + 2.5f - std::fmod(angle + 2.5f, 5.0f);
					else
						newAngle = angle;

					AB = glm::rotate(AB, -glm::radians(angle - newAngle), { 0.0f, 1.0f, 0.0f });
					m_ConstructionPositions[cubicCurveOrder[1]] = m_ConstructionPositions[cubicCurveOrder[0]] + AB;
				}
				else if (m_StartSnappedJunction)
				{
					float newRoadRotationY = glm::degrees(rotationEnd);
					float smallestAngle = 180.0f;
					for (RoadSegment* roadSegment : m_StartSnappedJunction->connectedRoadSegments)
					{
						float snappedRoadRotationY = m_StartSnappedJunction == roadSegment->ConnectedObjectAtStart.junction ? glm::degrees(roadSegment->GetStartRotation().y) : glm::degrees(roadSegment->GetEndRotation().y);
						float angle = std::fmod(snappedRoadRotationY - newRoadRotationY + 720.0f, 180.0f);
						smallestAngle = std::min(smallestAngle, angle);
					}
					float newAngle = 0.0f;
					if (smallestAngle < 32.0f)
						newAngle = 30.0f;
					else if (smallestAngle > 80.0f && smallestAngle < 100.0f)
						newAngle = 90.0f;
					else if (Input::IsKeyPressed(KeyCode::LeftControl))
						newAngle = smallestAngle + 2.5f - std::fmod(smallestAngle + 2.5f, 5.0f);
					else
						newAngle = smallestAngle;

					AB = glm::rotate(AB, -glm::radians(smallestAngle - newAngle), { 0.0f, 1.0f, 0.0f });
					m_ConstructionPositions[cubicCurveOrder[1]] = m_ConstructionPositions[cubicCurveOrder[0]] + AB;
				}
				else if (Input::IsKeyPressed(KeyCode::LeftControl))
				{
					float angle = std::fmod(glm::degrees(rotationEnd) + 720.0f, 360.0f);
					float newAngle = angle + 2.5f - std::fmod(angle + 2.5f, 5.0f);

					AB = glm::rotate(AB, -glm::radians(angle - newAngle), { 0.0f, 1.0f, 0.0f });
					m_ConstructionPositions[cubicCurveOrder[1]] = m_ConstructionPositions[cubicCurveOrder[0]] + AB;
				}
			}

			if (glm::length(AB) > 0.5f)
			{
				if (m_EndSnappedRoadSegment)
				{
					glm::vec3 n = { -m_EndSnappedRoadSegment->GetStartDirection().z, 0.0f, m_EndSnappedRoadSegment->GetStartDirection().x };
					m_ConstructionPositions[cubicCurveOrder[1]] = Helper::RayPlaneIntersection(
						m_ConstructionPositions[cubicCurveOrder[0]],
						AB,
						m_EndSnappedRoadSegment->GetStartPosition(),
						n
					);
					AB = m_ConstructionPositions[cubicCurveOrder[1]] - m_ConstructionPositions[cubicCurveOrder[0]];
				}
				else if (m_EndSnappedEnd)
				{
					m_ConstructionPositions[cubicCurveOrder[1]] = m_EndSnappedEnd->position;
					AB = m_ConstructionPositions[cubicCurveOrder[1]] - m_ConstructionPositions[cubicCurveOrder[0]];
				}
				else if (m_EndSnappedJunction)
				{
					m_ConstructionPositions[cubicCurveOrder[1]] = m_EndSnappedJunction->position;
					AB = m_ConstructionPositions[cubicCurveOrder[1]] - m_ConstructionPositions[cubicCurveOrder[0]];
				}
			}

			glm::vec3 normalizedAB = glm::normalize(AB);

			rotationOffset = (AB.x < 0.0f) * 180.0f;
			rotationStart = glm::atan(-AB.z / AB.x) + glm::radians(180.0f + rotationOffset);
			rotationEnd = glm::atan(-AB.z / AB.x) + glm::radians(rotationOffset);

			glm::vec2 least = { -roadPrefabWidth / 2.0f, -roadPrefabWidth / 2.0f };
			glm::vec2 most = { glm::length(AB) + roadPrefabWidth / 2.0f, roadPrefabWidth / 2.0f };
			if (m_StartSnappedEnd || m_StartSnappedJunction || m_StartSnappedRoadSegment)
				least.x = 0.0f;
			if (m_EndSnappedEnd || m_EndSnappedJunction || m_EndSnappedRoadSegment)
				most.x = glm::length(AB);

			m_GuidelinesStart->enabled = !b_ConstructionStartSnapped;
			m_GuidelinesEnd->enabled = !b_ConstructionEndSnapped;

			m_GuidelinesStart->SetTransform(m_ConstructionPositions[cubicCurveOrder[0]] + glm::vec3{ 0.0f, 0.15f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, rotationStart, 0.0f });
			m_GuidelinesEnd->SetTransform(m_ConstructionPositions[cubicCurveOrder[1]] + glm::vec3{ 0.0f, 0.15f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, rotationEnd, 0.0f });

			float availableABLength = (
				glm::length(AB)
				- (b_ConstructionStartSnapped ? roadPrefabLength : 0.0f)
				- (b_ConstructionEndSnapped ? roadPrefabLength : 0.0f)
				);
			availableABLength = std::max(availableABLength, 0.0f);

			int countAB = (int)(availableABLength / roadPrefabLength);
			float scaleAB = (availableABLength / roadPrefabLength) / countAB;
			float scaledRoadLength = availableABLength / countAB;

			bool lengthIsRestricted = restrictions[1] && countAB < 1;


			for (size_t& inUse : m_GuidelinesInUse)
				inUse = 0;

			int discountStart = (b_ConstructionStartSnapped ? 1 : 0);

			m_GuidelinesInUse[m_Type] += countAB;
			if (m_GuidelinesInUse[m_Type] > m_Guidelines[m_Type].size())
				for (size_t j = m_Guidelines[m_Type].size(); j < m_GuidelinesInUse[m_Type]; j++)
					m_Guidelines[m_Type].push_back(new Object(m_Scene->MainApplication->roads[m_Type][0], m_Scene->MainApplication->roads[m_Type][0]));

			for (size_t j = 0; j < countAB; j++)
			{
				Object* roadG = m_Guidelines[m_Type][j];
				roadG->enabled = true;
				roadG->SetTransform(
					m_ConstructionPositions[0] + (normalizedAB * ((j + discountStart) * scaledRoadLength)) + glm::vec3{ 0.0f, 0.15f, 0.0f },
					glm::vec3{ 1.0f * scaleAB, 1.0f, 1.0f },
					glm::vec3{ 0.0f, rotationEnd, 0.0f }
				);
			}

			// Do Something about this
			/*if (m_StartSnappedRoadSegment != nullptr)
			{
				float snappedRoadPrefabLength = m_StartSnappedRoad->type[0]->boundingBoxM.x - m_StartSnappedRoad->type[0]->boundingBoxL.x;
				size_t snappedRoadTypeIndex = m_StartSnappedRoad->typeIndex;
				m_StartSnappedRoad->object->enabled = false;
				glm::vec3 R0I = m_ConstructionPositions[0] - m_StartSnappedRoad->GetStartPosition();
				glm::vec3 R1I = m_ConstructionPositions[0] - m_StartSnappedRoad->GetEndPosition();

				glm::vec3 normalizedR0I = glm::normalize(R0I);
				glm::vec3 normalizedR1I = glm::normalize(R1I);

				glm::vec3 rotationR0I = m_StartSnappedRoad->rotation;
				glm::vec3 rotationR1I = {
					0.0f,
					m_StartSnappedRoad->rotation.y + glm::radians(180.0f),
					-m_StartSnappedRoad->rotation.z
				};

				float availableR0ILength = std::max(glm::length(R0I) - snappedRoadPrefabLength, 0.0f);
				float availableR1ILength = std::max(glm::length(R1I) - snappedRoadPrefabLength, 0.0f);

				int countR0I = (int)(availableR0ILength / snappedRoadPrefabLength);
				int countR1I = (int)(availableR1ILength / snappedRoadPrefabLength);

				lengthIsRestricted |= restrictions[1] && countR0I < 2;
				lengthIsRestricted |= restrictions[1] && countR1I < 2;

				float scaleR0I = (availableR0ILength / snappedRoadPrefabLength) / countR0I;
				float scaleR1I = (availableR1ILength / snappedRoadPrefabLength) / countR1I;

				float scaledR0IRoadLength = availableR0ILength / countR0I;
				float scaledR1IRoadLength = availableR1ILength / countR1I;

				size_t prevIndex = m_GuidelinesInUse[snappedRoadTypeIndex];
				m_GuidelinesInUse[snappedRoadTypeIndex] += countR0I;
				m_GuidelinesInUse[snappedRoadTypeIndex] += countR1I;
				if (m_GuidelinesInUse[snappedRoadTypeIndex] > m_Guidelines[snappedRoadTypeIndex].size())
					for (size_t j = m_Guidelines[snappedRoadTypeIndex].size(); j < m_GuidelinesInUse[snappedRoadTypeIndex]; j++)
						m_Guidelines[snappedRoadTypeIndex].push_back(new Object(m_Scene->MainApplication->roads[snappedRoadTypeIndex][0], m_Scene->MainApplication->roads[snappedRoadTypeIndex][0]));

				for (size_t j = 0; j < countR0I; j++)
				{
					Object* roadG = m_Guidelines[snappedRoadTypeIndex][j + prevIndex];
					roadG->enabled = true;
					roadG->SetTransform(
						m_StartSnappedRoad->GetStartPosition() + normalizedR0I * (j * scaledR0IRoadLength) + glm::vec3{ 0.0f, 0.15f, 0.0f },
						glm::vec3{ 1.0f * scaleR0I, 1.0f, 1.0f },
						rotationR0I
					);
				}

				for (size_t j = 0; j < countR1I; j++)
				{
					Object* roadG = m_Guidelines[snappedRoadTypeIndex][j + prevIndex + countR0I];
					roadG->enabled = true;
					roadG->SetTransform(
						m_StartSnappedRoad->GetEndPosition() + normalizedR1I * (j * scaledR1IRoadLength) + glm::vec3{ 0.0f, 0.15f, 0.0f },
						glm::vec3{ 1.0f * scaleR1I, 1.0f, 1.0f },
						rotationR1I
					);
				}
			}
			*/

			b_ConstructionRestricted |= lengthIsRestricted;
			b_ConstructionRestricted |= collisionIsRestricted;

			m_GuidelinesStart->tintColor = b_ConstructionRestricted ? glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f } : glm::vec4(1.0f);
			m_GuidelinesEnd->tintColor = b_ConstructionRestricted ? glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f } : glm::vec4(1.0f);

			for (std::vector<Object*>& os : m_Guidelines)
				for (Object* rg : os)
					rg->tintColor = b_ConstructionRestricted ? glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f } : glm::vec4(1.0f);

		}
		else if (m_ConstructionPhase == 2)
		{
			b_ConstructionRestricted = false;

			m_ConstructionPositions[cubicCurveOrder[2]] = prevLocation;

			float l = glm::length(m_ConstructionPositions[cubicCurveOrder[2]] - m_ConstructionPositions[cubicCurveOrder[0]]);
			size_t count = 1;
			while (l > roadPrefabLength)
			{
				count *= 2;
				glm::vec3 p = Math::QuadraticCurve<float>({
					m_ConstructionPositions[cubicCurveOrder[0]],
					m_ConstructionPositions[cubicCurveOrder[1]],
					m_ConstructionPositions[cubicCurveOrder[2]]
					}, 1.0f / count);
				l = glm::length(p - m_ConstructionPositions[cubicCurveOrder[0]]);
			}
			if (count > 1) count /= 2;

			while (l > roadPrefabLength)
			{
				count++;
				glm::vec3 p = Math::QuadraticCurve<float>({
					m_ConstructionPositions[cubicCurveOrder[0]],
					m_ConstructionPositions[cubicCurveOrder[1]],
					m_ConstructionPositions[cubicCurveOrder[2]]
					}, 1.0f / count);
				l = glm::length(p - m_ConstructionPositions[cubicCurveOrder[0]]);
			}
			if (count > 1) count--;


			glm::vec3 AB1 = m_ConstructionPositions[cubicCurveOrder[1]] - m_ConstructionPositions[cubicCurveOrder[0]];
			glm::vec3 AB2 = m_ConstructionPositions[cubicCurveOrder[1]] - m_ConstructionPositions[cubicCurveOrder[2]];

			float rotationOffset1 = AB1.x < 0.0f ? 180.0f : 0.0f;
			float rotationOffset2 = AB2.x < 0.0f ? 180.0f : 0.0f;

			float rotationStart = glm::atan(-AB1.z / AB1.x) + glm::radians(rotationOffset1 + 180.0f);
			float rotationEnd = glm::atan(-AB2.z / AB2.x) + glm::radians(rotationOffset2 + 180.0f);

			m_GuidelinesStart->enabled = !b_ConstructionStartSnapped;
			m_GuidelinesEnd->enabled = !b_ConstructionEndSnapped;

			m_GuidelinesStart->SetTransform(m_ConstructionPositions[cubicCurveOrder[0]] + glm::vec3{ 0.0f, 0.15f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, rotationStart, 0.0f });
			m_GuidelinesEnd->SetTransform(m_ConstructionPositions[cubicCurveOrder[2]] + glm::vec3{ 0.0f, 0.15f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, rotationEnd, 0.0f });

			for (size_t& inUse : m_GuidelinesInUse)
				inUse = 0;
			m_GuidelinesInUse[m_Type] += count;

			if (m_GuidelinesInUse[m_Type] > m_Guidelines[m_Type].size())
				for (size_t j = m_Guidelines[m_Type].size(); j < m_GuidelinesInUse[m_Type]; j++)
					m_Guidelines[m_Type].push_back(new Object(m_Scene->MainApplication->roads[m_Type][0], m_Scene->MainApplication->roads[m_Type][0]));


			glm::vec3 p1 = m_ConstructionPositions[0];
			for (int c = 0; c < count; c++)
			{
				glm::vec3 p2 = Math::QuadraticCurve<float>({
					m_ConstructionPositions[cubicCurveOrder[0]],
					m_ConstructionPositions[cubicCurveOrder[1]],
					m_ConstructionPositions[cubicCurveOrder[2]]
					}, (c + 1.0f) / count);
				glm::vec3 vec1 = p2 - p1;
				float length = glm::length(vec1);
				glm::vec3 dir1 = vec1 / length;

				float scale = length / roadPrefabLength;
				float rot1 = glm::acos(dir1.x) * ((float)(dir1.z < 0.0f) * 2.0f - 1.0f);

				Object* roadG = m_Guidelines[m_Type][c];
				roadG->enabled = true;
				roadG->SetTransform(
					p1 + glm::vec3{ 0.0f, 0.15f, 0.0f },
					glm::vec3{ scale, 1.0f, 1.0f },
					glm::vec3{ 0.0f, rot1, 0.0f }
				);

				p1 = p2;
			}
		}
		else if (m_ConstructionPhase == 3)
		{

			b_ConstructionRestricted = false;
			m_ConstructionPositions[cubicCurveOrder[3]] = prevLocation;

			bool collisionIsRestricted = false;
			if (restrictions[2])
			{
				glm::vec2 A1 = glm::vec2{ m_ConstructionPositions[0].x, m_ConstructionPositions[0].z };
				glm::vec2 B1 = glm::vec2{ m_ConstructionPositions[1].x, m_ConstructionPositions[1].z };
				glm::vec2 C1 = glm::vec2{ m_ConstructionPositions[2].x, m_ConstructionPositions[2].z };
				glm::vec2 D1 = glm::vec2{ m_ConstructionPositions[3].x, m_ConstructionPositions[3].z };

				std::array<std::array<glm::vec2, 3>, 2> newRoadPolygon = {
					std::array<glm::vec2,3>{ A1, B1, glm::vec2(0.0f)},
					std::array<glm::vec2,3>{ A1, C1, D1}
				};

				if (Math::CheckLineSegmentLineSegmentCollision(std::array<glm::vec2, 2>{ A1, D1 }, std::array<glm::vec2, 2>{ B1, C1 }))
					newRoadPolygon[0][2] = D1;
				else
					newRoadPolygon[0][2] = C1;

				for (RoadSegment* roadSegment : m_RoadSegments)
				{
					if (roadSegment == m_EndSnappedRoadSegment || roadSegment == m_StartSnappedRoadSegment)
						continue;
					if (m_StartSnappedEnd && roadSegment == m_StartSnappedEnd->connectedRoadSegment)
						continue;
					if (m_EndSnappedEnd && roadSegment == m_EndSnappedEnd->connectedRoadSegment)
						continue;
					if (m_StartSnappedJunction)
					{
						auto it = std::find(m_StartSnappedJunction->connectedRoadSegments.begin(), m_StartSnappedJunction->connectedRoadSegments.end(), roadSegment);
						if (it != m_StartSnappedJunction->connectedRoadSegments.end())
							continue;
					}
					if (m_EndSnappedJunction)
					{
						auto it = std::find(m_EndSnappedJunction->connectedRoadSegments.begin(), m_EndSnappedJunction->connectedRoadSegments.end(), roadSegment);
						if (it != m_EndSnappedJunction->connectedRoadSegments.end())
							continue;
					}
					auto cp = roadSegment->GetCurvePoints();
					glm::vec2 A2 = glm::vec2{ cp[0].x, cp[0].z };
					glm::vec2 B2 = glm::vec2{ cp[1].x, cp[1].z };
					glm::vec2 C2 = glm::vec2{ cp[2].x, cp[2].z };
					glm::vec2 D2 = glm::vec2{ cp[3].x, cp[3].z };

					std::array<std::array<glm::vec2, 3>, 2> oldRoadPolygon = {
						std::array<glm::vec2,3>{ A2, B2, glm::vec2(0.0f)},
						std::array<glm::vec2,3>{ A2, C2, D2}
					};

					if (Math::CheckLineSegmentLineSegmentCollision(std::array<glm::vec2, 2>{ A2, D2 }, std::array<glm::vec2, 2>{ B2, C2 }))
						oldRoadPolygon[0][2] = D2;
					else
						oldRoadPolygon[0][2] = C2;

					if (Math::CheckPolygonCollision(newRoadPolygon, oldRoadPolygon))
					{
						collisionIsRestricted = true;
						break;
					}
				}
			}
			float l = glm::length(m_ConstructionPositions[3] - m_ConstructionPositions[0]);
			size_t count = 1;
			while (l > roadPrefabLength)
			{
				count *= 2;
				glm::vec3 p = Math::CubicCurve<float>(m_ConstructionPositions, 1.0f / count);
				l = glm::length(p - m_ConstructionPositions[0]);
			}
			if (count > 1) count /= 2;

			while (l > roadPrefabLength)
			{
				count++;
				glm::vec3 p = Math::CubicCurve<float>(m_ConstructionPositions, 1.0f / count);
				l = glm::length(p - m_ConstructionPositions[0]);
			}
			if (count > 1) count--;

			glm::vec3 AB1 = m_ConstructionPositions[cubicCurveOrder[1]] - m_ConstructionPositions[cubicCurveOrder[0]];
			glm::vec3 AB2 = m_ConstructionPositions[cubicCurveOrder[2]] - m_ConstructionPositions[cubicCurveOrder[3]];

			float rotationOffset1 = AB1.x < 0.0f ? 180.0f : 0.0f;
			float rotationOffset2 = AB2.x < 0.0f ? 180.0f : 0.0f;

			float rotationStart = glm::atan(-AB1.z / AB1.x) + glm::radians(rotationOffset1 + 180.0f);
			float rotationEnd = glm::atan(-AB2.z / AB2.x) + glm::radians(rotationOffset2 + 180.0f);

			m_GuidelinesStart->enabled = !b_ConstructionStartSnapped;
			m_GuidelinesEnd->enabled = !b_ConstructionEndSnapped;

			m_GuidelinesStart->SetTransform(m_ConstructionPositions[cubicCurveOrder[0]] + glm::vec3{ 0.0f, 0.15f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, rotationStart, 0.0f });
			m_GuidelinesEnd->SetTransform(m_ConstructionPositions[cubicCurveOrder[2]] + glm::vec3{ 0.0f, 0.15f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, rotationEnd, 0.0f });

			for (size_t& inUse : m_GuidelinesInUse)
				inUse = 0;
			m_GuidelinesInUse[m_Type] += count;

			if (m_GuidelinesInUse[m_Type] > m_Guidelines[m_Type].size())
				for (size_t j = m_Guidelines[m_Type].size(); j < m_GuidelinesInUse[m_Type]; j++)
					m_Guidelines[m_Type].push_back(new Object(m_Scene->MainApplication->roads[m_Type][0], m_Scene->MainApplication->roads[m_Type][0]));


			glm::vec3 p1 = m_ConstructionPositions[0];
			for (int c = 0; c < count; c++)
			{
				glm::vec3 p2 = Math::CubicCurve<float>(m_ConstructionPositions, (c + 1.0f) / count);
				glm::vec3 vec1 = p2 - p1;
				float length = glm::length(vec1);
				glm::vec3 dir1 = vec1 / length;

				float scale = length / roadPrefabLength;
				float rot1 = glm::acos(dir1.x) * ((float)(dir1.z < 0.0f) * 2.0f - 1.0f);

				Object* roadG = m_Guidelines[m_Type][c];
				roadG->enabled = true;
				roadG->SetTransform(
					p1 + glm::vec3{ 0.0f, 0.15f, 0.0f },
					glm::vec3{ scale, 1.0f, 1.0f },
					glm::vec3{ 0.0f, rot1, 0.0f }
				);

				p1 = p2;
			}

			b_ConstructionRestricted |= collisionIsRestricted;

			m_GuidelinesStart->tintColor = b_ConstructionRestricted ? glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f } : glm::vec4(1.0f);
			m_GuidelinesEnd->tintColor = b_ConstructionRestricted ? glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f } : glm::vec4(1.0f);

			for (std::vector<Object*>& os : m_Guidelines)
				for (Object* rg : os)
					rg->tintColor = b_ConstructionRestricted ? glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f } : glm::vec4(1.0f);

		}
	}
	void RoadManager::OnUpdate_Destruction(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
	{
		SnapInformation snapInformation = CheckSnapping(cameraPosition, cameraDirection);
		m_DestructionSnappedJunction = snapInformation.junction;
		m_DestructionSnappedEnd = snapInformation.end;
		m_DestructionSnappedRoadSegment = snapInformation.roadSegment;

		for (Junction* junction : m_Junctions)
			junction->object->SetTransform(junction->position);

		for (End* end : m_Ends)
			end->object->SetTransform(end->position);

		for (RoadSegment* roadSegment : m_RoadSegments)
			roadSegment->object->SetTransform(roadSegment->GetStartPosition());

		if (snapInformation.snapped)
		{
			if (m_DestructionSnappedJunction != nullptr)
			{
				m_DestructionSnappedJunction->object->SetTransform(m_DestructionSnappedJunction->position + glm::vec3{ 0.0f, 0.1f, 0.0f });

				for (RoadSegment* roadSegment : m_DestructionSnappedJunction->connectedRoadSegments)
				{
					if (roadSegment->ConnectedObjectAtStart.end != nullptr)
						roadSegment->ConnectedObjectAtStart.end->object->SetTransform(roadSegment->ConnectedObjectAtStart.end->position + glm::vec3{ 0.0f, 0.1f, 0.0f });
					if (roadSegment->ConnectedObjectAtEnd.end != nullptr)
						roadSegment->ConnectedObjectAtEnd.end->object->SetTransform(roadSegment->ConnectedObjectAtEnd.end->position + glm::vec3{ 0.0f, 0.1f, 0.0f });
					roadSegment->object->SetTransform(roadSegment->GetStartPosition() + glm::vec3{ 0.0f, 0.1f, 0.0f });
				}
			}
			else if (m_DestructionSnappedEnd != nullptr)
			{
				RoadSegment* roadSegment = m_DestructionSnappedEnd->connectedRoadSegment;

				if (roadSegment->ConnectedObjectAtStart.end != nullptr)
					roadSegment->ConnectedObjectAtStart.end->object->SetTransform(roadSegment->ConnectedObjectAtStart.end->position + glm::vec3{ 0.0f, 0.1f, 0.0f });
				if (roadSegment->ConnectedObjectAtEnd.end != nullptr)
					roadSegment->ConnectedObjectAtEnd.end->object->SetTransform(roadSegment->ConnectedObjectAtEnd.end->position + glm::vec3{ 0.0f, 0.1f, 0.0f });
				roadSegment->object->SetTransform(roadSegment->GetStartPosition() + glm::vec3{ 0.0f, 0.1f, 0.0f });
			}
			else
			{
				if (m_DestructionSnappedRoadSegment->ConnectedObjectAtStart.end != nullptr)
					m_DestructionSnappedRoadSegment->ConnectedObjectAtStart.end->object->SetTransform(m_DestructionSnappedRoadSegment->ConnectedObjectAtStart.end->position + glm::vec3{ 0.0f, 0.1f, 0.0f });
				if (m_DestructionSnappedRoadSegment->ConnectedObjectAtEnd.end != nullptr)
					m_DestructionSnappedRoadSegment->ConnectedObjectAtEnd.end->object->SetTransform(m_DestructionSnappedRoadSegment->ConnectedObjectAtEnd.end->position + glm::vec3{ 0.0f, 0.1f, 0.0f });
				m_DestructionSnappedRoadSegment->object->SetTransform(m_DestructionSnappedRoadSegment->GetStartPosition() + glm::vec3{ 0.0f, 0.1f, 0.0f });
			}
		}
	}

	void RoadManager::DrawStraightGuidelines(const glm::vec3& pointA, const glm::vec3& pointB)
	{
		Prefab* selectedRoad = m_Scene->MainApplication->roads[m_Type][0];
		float roadPrefabLength = selectedRoad->boundingBoxM.x - selectedRoad->boundingBoxL.x;

		glm::vec3 AB = pointB - pointA;
		glm::vec3 normalizedAB = glm::normalize(AB);

		float rotationOffset = AB.x < 0.0f ? glm::radians(180.0f) : 0.0f;
		float rotationStart = glm::atan(-AB.z / AB.x) + glm::radians(180.0f) + rotationOffset;
		float rotationEnd = glm::atan(-AB.z / AB.x) + rotationOffset;

		float availableABLength = (
			glm::length(AB)
			- (b_ConstructionStartSnapped ? roadPrefabLength : 0.0f)
			- (b_ConstructionEndSnapped ? roadPrefabLength : 0.0f)
			);
		availableABLength = std::max(availableABLength, 0.0f);

		int countAB = (int)(availableABLength / roadPrefabLength);
		float scaleAB = (availableABLength / roadPrefabLength) / countAB;
		float scaledRoadLength = availableABLength / countAB;

		bool lengthIsRestricted = restrictions[1] && countAB < 1;

		int discountStart = (b_ConstructionStartSnapped ? 1 : 0);

		m_GuidelinesInUse[m_Type] += countAB;
		if (m_GuidelinesInUse[m_Type] > m_Guidelines[m_Type].size())
			for (size_t j = m_Guidelines[m_Type].size(); j < m_GuidelinesInUse[m_Type]; j++)
				m_Guidelines[m_Type].push_back(new Object(m_Scene->MainApplication->roads[m_Type][0], m_Scene->MainApplication->roads[m_Type][0]));

		for (size_t j = 0; j < countAB; j++)
		{
			Object* roadG = m_Guidelines[m_Type][j];
			roadG->enabled = true;
			roadG->SetTransform(
				pointA + (normalizedAB * ((j + discountStart) * scaledRoadLength)) + glm::vec3{ 0.0f, 0.15f, 0.0f },
				glm::vec3{ 1.0f * scaleAB, 1.0f, 1.0f },
				glm::vec3{ 0.0f, rotationEnd, 0.0f }
			);
		}

		b_ConstructionRestricted |= lengthIsRestricted;

		m_GuidelinesStart->enabled = !b_ConstructionStartSnapped;
		m_GuidelinesEnd->enabled = !b_ConstructionEndSnapped;

		m_GuidelinesStart->SetTransform(pointA + glm::vec3{ 0.0f, 0.15f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, rotationStart, 0.0f });
		m_GuidelinesEnd->SetTransform(pointB + glm::vec3{ 0.0f, 0.15f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, rotationEnd, 0.0f });

		m_GuidelinesStart->tintColor = b_ConstructionRestricted ? glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f } : glm::vec4(1.0f);
		m_GuidelinesEnd->tintColor = b_ConstructionRestricted ? glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f } : glm::vec4(1.0f);

		for (std::vector<Object*>& os : m_Guidelines)
			for (Object* rg : os)
				rg->tintColor = b_ConstructionRestricted ? glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f } : glm::vec4(1.0f);

	}

	bool RoadManager::OnMousePressed(MouseCode button, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
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
			OnMousePressed_Straight(cameraPosition, cameraDirection);
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
			OnMousePressed_QuadraticCurve(cameraPosition, cameraDirection);
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
			OnMousePressed_CubicCurve(cameraPosition, cameraDirection);
			break;
		case RoadConstructionMode::Upgrade:
			break;
		case RoadConstructionMode::Destruct:
			if (button != MouseCode::Button0)
				return false;
			OnMousePressed_Destruction();
			break;
		}
	}
	bool RoadManager::OnMousePressed_Straight(const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
	{
		glm::vec3 intersection = Helper::GetRayHitPointOnTerrain(m_Scene, cameraPosition, cameraDirection);

		if (intersection.y >= -0.5f)
		{
			if (m_ConstructionPhase == 0)
			{
				m_ConstructionPhase++;
			}
			else
			{
				m_ConstructionPositions[1] = (m_ConstructionPositions[0] + m_ConstructionPositions[3]) / 2.0f;
				m_ConstructionPositions[2] = (m_ConstructionPositions[0] + m_ConstructionPositions[3]) / 2.0f;
				m_ConstructionPhase++;
			}
		}

		// take this put it into else part of the prev if else
		if (m_ConstructionPhase > 1)
		{
			for (std::vector<Object*>& os : m_Guidelines)
				for (Object* rg : os)
					rg->enabled = false;
			for (size_t& inUse : m_GuidelinesInUse)
				inUse = 0;
			m_ConstructionPhase = 0;

			AddRoadSegment();

			ResetStates();
			m_GuidelinesStart->enabled = true;
			m_GuidelinesEnd->enabled = true;
		}
		return false;
	}
	bool RoadManager::OnMousePressed_QuadraticCurve(const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
	{
		glm::vec3 intersection = Helper::GetRayHitPointOnTerrain(m_Scene, cameraPosition, cameraDirection);
		if (intersection.y > -0.5f)
		{
			m_ConstructionPositions[m_ConstructionPhase] = intersection;
			m_ConstructionPhase++;
		}
		if (m_ConstructionPhase > 2)
		{
			for (std::vector<Object*>& os : m_Guidelines)
				for (Object* rg : os)
					rg->enabled = false;
			for (size_t& inUse : m_GuidelinesInUse)
				inUse = 0;
			m_ConstructionPhase = 0;

			AddRoadSegment();

			ResetStates();
			m_GuidelinesStart->enabled = true;
			m_GuidelinesEnd->enabled = true;
		}
		return false;
	}
	bool RoadManager::OnMousePressed_CubicCurve(const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
	{
		glm::vec3 intersection = Helper::GetRayHitPointOnTerrain(m_Scene, cameraPosition, cameraDirection);
		if (intersection.y > -0.5f)
		{
			m_ConstructionPositions[cubicCurveOrder[m_ConstructionPhase]] = intersection;
			m_ConstructionPhase++;
		}
		if (m_ConstructionPhase > 3)
		{
			for (std::vector<Object*>& os : m_Guidelines)
				for (Object* rg : os)
					rg->enabled = false;
			for (size_t& inUse : m_GuidelinesInUse)
				inUse = 0;
			m_ConstructionPhase = 0;

			AddRoadSegment();

			ResetStates();
			m_GuidelinesStart->enabled = true;
			m_GuidelinesEnd->enabled = true;
		}
		return false;
	}
	bool RoadManager::OnMousePressed_Destruction()
	{
		if (m_DestructionSnappedJunction != nullptr)
		{
			std::vector<RoadSegment*> roads_copy = m_DestructionSnappedJunction->connectedRoadSegments;
			for (RoadSegment* roadSegment : roads_copy)
				RemoveRoadSegment(roadSegment);
		}
		else if (m_DestructionSnappedEnd != nullptr)
		{
			RemoveRoadSegment(m_DestructionSnappedEnd->connectedRoadSegment);
		}
		else if (m_DestructionSnappedRoadSegment != nullptr)
		{
			RemoveRoadSegment(m_DestructionSnappedRoadSegment);
		}
		return false;
	}

	void RoadManager::SetType(size_t type)
	{
		m_Type = type;
		delete m_GuidelinesEnd;
		delete m_GuidelinesStart;
		m_GuidelinesStart = new Object(m_Scene->MainApplication->roads[m_Type][2], m_Scene->MainApplication->roads[m_Type][2], glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f));
		m_GuidelinesEnd = new Object(m_Scene->MainApplication->roads[m_Type][2], m_Scene->MainApplication->roads[m_Type][2], glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f));
	}

	void RoadManager::SetConstructionMode(RoadConstructionMode mode)
	{
		ResetStates();
		m_ConstructionMode = mode;

		switch (m_ConstructionMode)
		{
		case Can::RoadConstructionMode::None:
			break;
		case Can::RoadConstructionMode::Straight:
			m_GuidelinesStart->enabled = true;
			m_GuidelinesEnd->enabled = true;
			break;
		case Can::RoadConstructionMode::QuadraticCurve:
			m_GuidelinesStart->enabled = true;
			m_GuidelinesEnd->enabled = true;
			break;
		case Can::RoadConstructionMode::CubicCurve:
			m_GuidelinesStart->enabled = true;
			m_GuidelinesEnd->enabled = true;
			break;
		case Can::RoadConstructionMode::Upgrade:
			break;
		case Can::RoadConstructionMode::Destruct:
			break;
		default:
			break;
		}
	}

	void RoadManager::AddRoadSegment()
	{
		RoadSegment* roadSegment = new RoadSegment(
			m_Scene->MainApplication->roads[m_Type],
			m_ConstructionPositions
		);
		m_RoadSegments.push_back(roadSegment);

		/////////////////// Move somewhere else
		float roadPrefabWidth = m_Scene->MainApplication->roads[m_Type][0]->boundingBoxM.z - m_Scene->MainApplication->roads[m_Type][0]->boundingBoxL.z;
		glm::vec3 AB = m_ConstructionPositions[3] - m_ConstructionPositions[0];
		float rotation = glm::atan(-AB.z / AB.x) + glm::radians((AB.x < 0.0f) * 180.0f);

		glm::vec2 least = { -roadPrefabWidth / 2.0f, -roadPrefabWidth / 2.0f };
		glm::vec2 most = { glm::length(AB) + roadPrefabWidth / 2.0f, roadPrefabWidth / 2.0f };
		if (m_StartSnappedEnd || m_StartSnappedJunction || m_StartSnappedRoadSegment)
			least.x = 0.0f;
		if (m_EndSnappedEnd || m_EndSnappedJunction || m_EndSnappedRoadSegment)
			most.x = glm::length(AB);

		auto& buildings = m_Scene->m_BuildingManager.GetBuildings();
		if (m_Scene->m_BuildingManager.restrictions[0] && restrictions[2])
			for (size_t i = 0; i < buildings.size(); i++)
			{
				Building* building = buildings[i];
				glm::vec2 mtv = Helper::CheckRotatedRectangleCollision(
					least,
					most,
					rotation,
					glm::vec2{ m_ConstructionPositions[0].x, m_ConstructionPositions[0].z },
					glm::vec2{ building->object->prefab->boundingBoxL.x ,building->object->prefab->boundingBoxL.z },
					glm::vec2{ building->object->prefab->boundingBoxM.x ,building->object->prefab->boundingBoxM.z },
					building->object->rotation.y,
					glm::vec2{ building->position.x,building->position.z }
				);

				if (mtv.x != 0.0f || mtv.y != 0.0f)
				{
					if (building->connectedRoadSegment)
					{
						auto it = std::find(
							building->connectedRoadSegment->Buildings.begin(),
							building->connectedRoadSegment->Buildings.end(),
							building
						);
						building->connectedRoadSegment->Buildings.erase(it);
					}
					buildings.erase(buildings.begin() + i);
					delete building;
					i--;
				}
			}

		auto& trees = m_Scene->m_TreeManager.GetTrees();
		if (m_Scene->m_TreeManager.restrictions[0] && restrictions[2])
			for (size_t i = 0; i < trees.size(); i++)
			{
				Object* tree = trees[i];
				glm::vec2 mtv = Helper::CheckRotatedRectangleCollision(
					least,
					most,
					rotation,
					glm::vec2{ m_ConstructionPositions[0].x, m_ConstructionPositions[0].z },
					glm::vec2{ tree->prefab->boundingBoxL.x ,tree->prefab->boundingBoxL.z },
					glm::vec2{ tree->prefab->boundingBoxM.x ,tree->prefab->boundingBoxM.z },
					tree->rotation.y,
					glm::vec2{ tree->position.x, tree->position.z }
				);

				if (mtv.x != 0.0f || mtv.y != 0.0f)
				{
					trees.erase(trees.begin() + i);
					delete tree;
					i--;
				}
			}
		///////////////////

		if (m_StartSnappedJunction != nullptr)
		{
			roadSegment->ConnectedObjectAtStart.junction = m_StartSnappedJunction;
			m_StartSnappedJunction->connectedRoadSegments.push_back(roadSegment);
			m_StartSnappedJunction->ReconstructObject();
		}
		else if (m_StartSnappedEnd != nullptr)
		{
			// Check angle if bigger than x
			// Create Junction
			// Else
			// Connected 2 road Segment

			RoadSegment* connectedRoadSegment = m_StartSnappedEnd->connectedRoadSegment;
			if (connectedRoadSegment->ConnectedObjectAtStart.end == m_StartSnappedEnd)
				connectedRoadSegment->ConnectedObjectAtStart.end = nullptr;
			else
				connectedRoadSegment->ConnectedObjectAtEnd.end = nullptr;

			Junction* newJunction = new Junction(std::vector<RoadSegment*>{ connectedRoadSegment, roadSegment }, m_StartSnappedEnd->object->position);
			roadSegment->ConnectedObjectAtStart.junction = newJunction;

			if (connectedRoadSegment->GetStartPosition() == m_StartSnappedEnd->object->position)
				roadSegment->ConnectedObjectAtStart.junction = newJunction;
			else
				roadSegment->ConnectedObjectAtEnd.junction = newJunction;


			auto position = std::find(m_Ends.begin(), m_Ends.end(), m_StartSnappedEnd);
			m_Ends.erase(position);
			delete m_StartSnappedEnd;

			m_Junctions.push_back(newJunction);
			newJunction->ConstructObject();
		}
		else if (m_StartSnappedRoadSegment != nullptr)
		{
			std::array<glm::vec3, 4> curve{
				m_StartSnappedRoadSegment->GetCurvePoint(0),
				m_StartSnappedRoadSegment->GetCurvePoint(1),
				glm::vec3(0.0f),
				m_ConstructionPositions[0],
			};
			curve[1] = (curve[0] + curve[1]) / 2.0f;
			float length = glm::length(curve[1] - curve[0]);
			glm::vec3 prevPointOnCurve = Math::CubicCurve(m_StartSnappedRoadSegment->GetCurvePoints(), m_StartSnappedRoadSegmentT - m_StartSnappedRoadSegmentTDelta);
			glm::vec3 vec = glm::normalize(prevPointOnCurve - curve[3]) * length;
			curve[2] = curve[3] + vec;

			RoadSegment* rs1 = new RoadSegment(
				m_StartSnappedRoadSegment->Type,
				curve
			);
			Junction* roadSegmentStartSnappedJunction = m_StartSnappedRoadSegment->ConnectedObjectAtStart.junction;
			if (roadSegmentStartSnappedJunction != nullptr)
			{
				auto it = std::find(
					roadSegmentStartSnappedJunction->connectedRoadSegments.begin(),
					roadSegmentStartSnappedJunction->connectedRoadSegments.end(),
					m_StartSnappedRoadSegment
				);
				roadSegmentStartSnappedJunction->connectedRoadSegments.erase(it);
				roadSegmentStartSnappedJunction->connectedRoadSegments.push_back(rs1);
				rs1->ConnectedObjectAtStart.junction = roadSegmentStartSnappedJunction;
			}
			else
			{
				rs1->ConnectedObjectAtStart.end = m_StartSnappedRoadSegment->ConnectedObjectAtStart.end;
				m_StartSnappedRoadSegment->ConnectedObjectAtStart.end->connectedRoadSegment = rs1;
			}
			m_RoadSegments.push_back(rs1);

			curve = {
				m_StartSnappedRoadSegment->GetCurvePoint(3),
				m_StartSnappedRoadSegment->GetCurvePoint(2),
				glm::vec3(0.0f),
				m_ConstructionPositions[0],
			};
			curve[1] = (curve[0] + curve[1]) / 2.0f;
			length = glm::length(curve[1] - curve[0]);
			prevPointOnCurve = Math::CubicCurve(m_StartSnappedRoadSegment->GetCurvePoints(), m_StartSnappedRoadSegmentT + m_StartSnappedRoadSegmentTDelta);
			vec = glm::normalize(prevPointOnCurve - curve[3]) * length;
			curve[2] = curve[3] + vec;

			RoadSegment* rs2 = new RoadSegment(
				m_StartSnappedRoadSegment->Type,
				curve
			);

			Junction* roadSegmentEndSnappedJunction = m_StartSnappedRoadSegment->ConnectedObjectAtEnd.junction;
			if (roadSegmentEndSnappedJunction != nullptr)
			{
				auto it = std::find(
					roadSegmentEndSnappedJunction->connectedRoadSegments.begin(),
					roadSegmentEndSnappedJunction->connectedRoadSegments.end(),
					m_StartSnappedRoadSegment
				);
				roadSegmentEndSnappedJunction->connectedRoadSegments.erase(it);
				roadSegmentEndSnappedJunction->connectedRoadSegments.push_back(rs2);
				rs2->ConnectedObjectAtStart.junction = roadSegmentEndSnappedJunction;
			}
			else
			{
				rs2->ConnectedObjectAtStart.end = m_StartSnappedRoadSegment->ConnectedObjectAtEnd.end;
				m_StartSnappedRoadSegment->ConnectedObjectAtEnd.end->connectedRoadSegment = rs2;
			}
			m_RoadSegments.push_back(rs2);

			float roadWidth = rs1->object->prefab->boundingBoxM.z - rs1->object->prefab->boundingBoxL.z;
			rs1->object->enabled = true; // Needed??

			for (Building* building : m_StartSnappedRoadSegment->Buildings)
			{

				/* Needs to be Redone
				glm::vec3 B = building->position - r1->GetStartPosition();
				float bLength = glm::length(B);

				float angle = glm::acos(glm::dot(r1->direction, B) / bLength);

				float c = bLength * glm::cos(angle);
				if (c <= 0.0f || c >= r1->length)
				{
					r2->connectedBuildings.push_back(building);
					building->connectedRoad = r2;
				}
				else
				{
					r1->connectedBuildings.push_back(building);
					building->connectedRoad = r1;
				}*/
			}

			auto it = std::find(m_RoadSegments.begin(), m_RoadSegments.end(), m_StartSnappedRoadSegment);
			m_RoadSegments.erase(it);
			delete m_StartSnappedRoadSegment;

			Junction* newJunction = new Junction(std::vector<RoadSegment*>{ roadSegment, rs1, rs2 }, m_ConstructionPositions[0]);
			m_Junctions.push_back(newJunction);
			roadSegment->ConnectedObjectAtStart.junction = newJunction;
			rs1->ConnectedObjectAtEnd.junction = newJunction;
			rs2->ConnectedObjectAtEnd.junction = newJunction;
			newJunction->ConstructObject();
		}
		else
		{
			End* newEnd = new End(
				roadSegment,
				m_Scene->MainApplication->roads[m_Type][2],
				m_ConstructionPositions[0],
				glm::vec3{ 1.0f, 1.0f, 1.0f },
				glm::vec3{
					0.0f,
					roadSegment->GetStartRotation().y + glm::radians(180.0f),
					roadSegment->GetStartRotation().x
				}
			);
			roadSegment->ConnectedObjectAtStart.end = newEnd;
			m_Ends.push_back(newEnd);
		}

		if (m_EndSnappedJunction != nullptr)
		{
			roadSegment->ConnectedObjectAtEnd.junction = m_EndSnappedJunction;
			m_EndSnappedJunction->connectedRoadSegments.push_back(roadSegment);
			m_EndSnappedJunction->ReconstructObject();
		}
		else if (m_EndSnappedEnd != nullptr)
		{
			// Check angle if bigger than x
			// Create Junction
			// Else
			// Connected 2 road Segment

			RoadSegment* connectedRoadSegment = m_EndSnappedEnd->connectedRoadSegment;
			if (connectedRoadSegment->ConnectedObjectAtStart.end == m_EndSnappedEnd)
				connectedRoadSegment->ConnectedObjectAtStart.end = nullptr;
			else
				connectedRoadSegment->ConnectedObjectAtEnd.end = nullptr;


			Junction* newJunction = new Junction(std::vector<RoadSegment*>{ connectedRoadSegment, roadSegment}, m_EndSnappedEnd->object->position);
			roadSegment->ConnectedObjectAtEnd.junction = newJunction;

			if (connectedRoadSegment->GetStartPosition() == m_EndSnappedEnd->object->position)
				connectedRoadSegment->ConnectedObjectAtStart.junction = newJunction;
			else
				connectedRoadSegment->ConnectedObjectAtEnd.junction = newJunction;


			auto position = std::find(m_Ends.begin(), m_Ends.end(), m_EndSnappedEnd);
			m_Ends.erase(position);
			delete m_EndSnappedEnd;

			m_Junctions.push_back(newJunction);
			newJunction->ConstructObject();
		}
		else if (m_EndSnappedRoadSegment != nullptr)
		{
			std::array<glm::vec3, 4> curve{
				m_EndSnappedRoadSegment->GetCurvePoint(0),
				m_EndSnappedRoadSegment->GetCurvePoint(1),
				glm::vec3(0.0f),
				m_ConstructionPositions[3],
			};
			curve[1] = (curve[0] + curve[1]) / 2.0f;
			float length = glm::length(curve[1] - curve[0]);
			glm::vec3 prevPointOnCurve = Math::CubicCurve(m_EndSnappedRoadSegment->GetCurvePoints(), m_EndSnappedRoadSegmentT - m_EndSnappedRoadSegmentTDelta);
			glm::vec3 vec = glm::normalize(prevPointOnCurve - curve[3]) * length;
			curve[2] = curve[3] + vec;

			RoadSegment* rs1 = new RoadSegment(
				m_EndSnappedRoadSegment->Type,
				curve
			);

			Junction* roadSegmentStartSnappedJunction = m_EndSnappedRoadSegment->ConnectedObjectAtStart.junction;
			if (roadSegmentStartSnappedJunction != nullptr)
			{
				auto it = std::find(
					roadSegmentStartSnappedJunction->connectedRoadSegments.begin(),
					roadSegmentStartSnappedJunction->connectedRoadSegments.end(),
					m_EndSnappedRoadSegment
				);
				roadSegmentStartSnappedJunction->connectedRoadSegments.erase(it);
				roadSegmentStartSnappedJunction->connectedRoadSegments.push_back(rs1);
				rs1->ConnectedObjectAtStart.junction = roadSegmentStartSnappedJunction;
			}
			else
			{
				rs1->ConnectedObjectAtStart.end = m_EndSnappedRoadSegment->ConnectedObjectAtStart.end;
				m_EndSnappedRoadSegment->ConnectedObjectAtStart.end->connectedRoadSegment = rs1;
			}
			m_RoadSegments.push_back(rs1);

			curve = {
				m_EndSnappedRoadSegment->GetCurvePoint(3),
				m_EndSnappedRoadSegment->GetCurvePoint(2),
				glm::vec3(0.0f),
				m_ConstructionPositions[3],
			};
			curve[1] = (curve[0] + curve[1]) / 2.0f;
			length = glm::length(curve[1] - curve[0]);
			prevPointOnCurve = Math::CubicCurve(m_EndSnappedRoadSegment->GetCurvePoints(), m_EndSnappedRoadSegmentT + m_EndSnappedRoadSegmentTDelta);
			vec = glm::normalize(prevPointOnCurve - curve[3]) * length;
			curve[2] = curve[3] + vec;

			RoadSegment* rs2 = new RoadSegment(
				m_EndSnappedRoadSegment->Type,
				curve
			);

			Junction* roadSegmentEndSnappedJunction = m_EndSnappedRoadSegment->ConnectedObjectAtStart.junction;
			if (roadSegmentEndSnappedJunction != nullptr)
			{
				auto it = std::find(
					roadSegmentEndSnappedJunction->connectedRoadSegments.begin(),
					roadSegmentEndSnappedJunction->connectedRoadSegments.end(),
					m_EndSnappedRoadSegment
				);
				roadSegmentEndSnappedJunction->connectedRoadSegments.erase(it);
				roadSegmentEndSnappedJunction->connectedRoadSegments.push_back(rs2);
				rs2->ConnectedObjectAtStart.junction = roadSegmentEndSnappedJunction;
			}
			else
			{
				rs2->ConnectedObjectAtStart.end = m_EndSnappedRoadSegment->ConnectedObjectAtStart.end;
				m_EndSnappedRoadSegment->ConnectedObjectAtStart.end->connectedRoadSegment = rs2;
			}
			m_RoadSegments.push_back(rs2);

			for (Building* building : m_EndSnappedRoadSegment->Buildings)
			{
				/* Tekrar kodla
				glm::vec3 B = building->position - r1->GetStartPosition();
				float bLength = glm::length(B);

				float angle = glm::acos(glm::dot(r1->direction, B) / bLength);

				float c = bLength * glm::cos(angle);
				if (c <= 0.0f || c >= r1->length)
				{
					r2->connectedBuildings.push_back(building);
					building->connectedRoad = r2;
				}
				else
				{
					r1->connectedBuildings.push_back(building);
					building->connectedRoad = r1;
				}*/
			}

			auto it = std::find(m_RoadSegments.begin(), m_RoadSegments.end(), m_EndSnappedRoadSegment);
			m_RoadSegments.erase(it);
			delete m_EndSnappedRoadSegment;

			Junction* newJunction = new Junction(std::vector<RoadSegment*>{ roadSegment, rs1, rs2 }, m_ConstructionPositions[3]);
			m_Junctions.push_back(newJunction);
			roadSegment->ConnectedObjectAtEnd.junction = newJunction;
			rs1->ConnectedObjectAtEnd.junction = newJunction;
			rs2->ConnectedObjectAtEnd.junction = newJunction;
			newJunction->ConstructObject();
		}
		else
		{
			End* newEnd = new End(
				roadSegment,
				m_Scene->MainApplication->roads[m_Type][2],
				m_ConstructionPositions[3],
				glm::vec3{ 1.0f, 1.0f, 1.0f },
				glm::vec3{
					0.0f,
					roadSegment->GetEndRotation().y + glm::radians(180.0f),
					roadSegment->GetEndRotation().x
				}
			);
			roadSegment->ConnectedObjectAtEnd.end = newEnd;
			m_Ends.push_back(newEnd);
		}

		//Helper::LevelTheTerrain(startIndex, endIndex, newRoad->startPos, newRoad->endPos,  m_Scene->MainApplication->m_Terrain, roadPrefabWidth);

	}

	void RoadManager::RemoveRoadSegment(RoadSegment* roadSegment)
	{
		if (roadSegment->ConnectedObjectAtStart.end != nullptr)
		{
			auto endPosition = std::find(m_Ends.begin(), m_Ends.end(), roadSegment->ConnectedObjectAtStart.end);
			m_Ends.erase(endPosition);
			delete roadSegment->ConnectedObjectAtStart.end;
		}
		else if (roadSegment->ConnectedObjectAtStart.roadSegment != nullptr)
		{
			// Fill
		}
		else
		{
			Junction* junction = roadSegment->ConnectedObjectAtStart.junction;
			std::vector<RoadSegment*>& connectedRoadSegments = junction->connectedRoadSegments;
			auto roadSegmentPosition = std::find(connectedRoadSegments.begin(), connectedRoadSegments.end(), roadSegment);
			connectedRoadSegments.erase(roadSegmentPosition);
			if (connectedRoadSegments.size() == 1)
			{
				RoadSegment* otherRoadSegment = connectedRoadSegments[0];
				if (otherRoadSegment->ConnectedObjectAtStart.junction == junction)
				{
					End* newEnd = new End(
						otherRoadSegment,
						otherRoadSegment->Type[2],
						junction->position,
						glm::vec3{ 1.0f, 1.0f, 1.0f },
						glm::vec3{ 0.0f, otherRoadSegment->GetStartRotation().y, otherRoadSegment->GetStartRotation().x }
					);
					otherRoadSegment->ConnectedObjectAtStart.end = newEnd;
					otherRoadSegment->ConnectedObjectAtStart.junction = nullptr;
					otherRoadSegment->SetStartPosition(junction->position);
					m_Ends.push_back(newEnd);
				}
				else
				{
					End* newEnd = new End(
						otherRoadSegment,
						otherRoadSegment->Type[2],
						junction->position,
						glm::vec3{ 1.0f, 1.0f, 1.0f },
						glm::vec3{ 0.0f, otherRoadSegment->GetEndRotation().y, otherRoadSegment->GetEndRotation().x }
					);
					otherRoadSegment->ConnectedObjectAtEnd.end = newEnd;
					otherRoadSegment->ConnectedObjectAtEnd.junction = nullptr;
					otherRoadSegment->SetEndPosition(junction->position);
					m_Ends.push_back(newEnd);
				}

				auto juncPosition = std::find(m_Junctions.begin(), m_Junctions.end(), junction);
				m_Junctions.erase(juncPosition);
				delete junction;
			}
			else
				junction->ReconstructObject();
		}

		if (roadSegment->ConnectedObjectAtEnd.end != nullptr)
		{
			auto endPosition = std::find(m_Ends.begin(), m_Ends.end(), roadSegment->ConnectedObjectAtEnd.end);
			m_Ends.erase(endPosition);
			delete roadSegment->ConnectedObjectAtEnd.end;
		}
		else if (roadSegment->ConnectedObjectAtEnd.roadSegment != nullptr)
		{
			// Fill
		}
		else
		{
			Junction* junction = roadSegment->ConnectedObjectAtEnd.junction;
			std::vector<RoadSegment*>& connectedRoadSegments = junction->connectedRoadSegments;
			auto roadSegmentPosition = std::find(connectedRoadSegments.begin(), connectedRoadSegments.end(), roadSegment);
			connectedRoadSegments.erase(roadSegmentPosition);
			if (connectedRoadSegments.size() == 1)
			{
				RoadSegment* otherRoadSegment = connectedRoadSegments[0];
				if (otherRoadSegment->ConnectedObjectAtStart.junction == junction)
				{
					End* newEnd = new End(
						otherRoadSegment,
						otherRoadSegment->Type[2],
						junction->position,
						glm::vec3{ 1.0f, 1.0f, 1.0f },
						glm::vec3{ 0.0f, otherRoadSegment->GetStartRotation().y, otherRoadSegment->GetStartRotation().x }
					);
					otherRoadSegment->ConnectedObjectAtStart.end = newEnd;
					otherRoadSegment->ConnectedObjectAtStart.junction = nullptr;
					otherRoadSegment->SetStartPosition(junction->position);
					m_Ends.push_back(newEnd);
				}
				else
				{
					End* newEnd = new End(
						otherRoadSegment,
						otherRoadSegment->Type[2],
						junction->position,
						glm::vec3{ 1.0f, 1.0f, 1.0f },
						glm::vec3{ 0.0f, otherRoadSegment->GetEndRotation().y, otherRoadSegment->GetEndRotation().x }
					);
					otherRoadSegment->ConnectedObjectAtEnd.end = newEnd;
					otherRoadSegment->ConnectedObjectAtEnd.junction = nullptr;
					otherRoadSegment->SetEndPosition(junction->position);
					m_Ends.push_back(newEnd);
				}

				auto juncPosition = std::find(m_Junctions.begin(), m_Junctions.end(), junction);
				m_Junctions.erase(juncPosition);
				delete junction;
			}
			else
				junction->ReconstructObject();
		}

		auto position = std::find(m_RoadSegments.begin(), m_RoadSegments.end(), roadSegment);
		m_RoadSegments.erase(position);

		for (Building* building : roadSegment->Buildings)
		{
			m_Scene->m_BuildingManager.GetBuildings().erase(std::find(m_Scene->m_BuildingManager.GetBuildings().begin(), m_Scene->m_BuildingManager.GetBuildings().end(), building));
			delete building;
		}

		delete roadSegment;
	}

	SnapInformation RoadManager::CheckSnapping(const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
	{
		Prefab* selectedRoad = m_Scene->MainApplication->roads[m_Type][0];
		float roadPrefabWidth = selectedRoad->boundingBoxM.z - selectedRoad->boundingBoxL.z;
		float snapDistance = roadPrefabWidth;
		SnapInformation results{ false, { 0.0f, 0.0f, 0.0f } };

		/* Update later
		for (Junction* junction : m_Junctions)
		{
			glm::vec3 Intersection = Helper::RayPlaneIntersection(cameraPosition, cameraDirection, junction->position, { 0.0f, 1.0f, 0.0f, });

			float distance = glm::length(junction->position - Intersection);
			if (distance < snapDistance)
			{
				results.snapLocation = junction->position;
				results.snapped = true;
				results.snappedJunction = junction;
				return results;
			}
		}
		for (End* end : m_Ends)
		{
			float endRadius = end->object->prefab->boundingBoxM.x - end->object->prefab->boundingBoxL.x;
			snapDistance = roadPrefabWidth / 2.0f + endRadius;

			glm::vec3 Intersection = Helper::RayPlaneIntersection(cameraPosition, cameraDirection, end->object->position, { 0.0f, 1.0f, 0.0f, });

			float distance = glm::length(end->object->position - Intersection);
			if (distance < snapDistance)
			{
				results.snapLocation = end->object->position;
				results.snapped = true;
				results.snappedEnd = end;
				return results;
			}
		}
		float roadPrefabLength = selectedRoad->boundingBoxM.x - selectedRoad->boundingBoxL.x;
		for (RoadSegment* roadSegment : m_RoadSegments)
		{
			float roadWidth = road->object->prefab->boundingBoxM.z - road->object->prefab->boundingBoxL.z;
			snapDistance = (roadPrefabWidth + roadWidth) / 2.0f;

			road->object->enabled = true;
			glm::vec3 Intersection = Helper::RayPlaneIntersection(cameraPosition, cameraDirection, road->GetStartPosition(), { 0.0f, 1.0f, 0.0f, });


			glm::vec3 B = Intersection - road->GetStartPosition();
			float bLength = glm::length(B);

			float angle = glm::acos(glm::dot(road->direction, B) / bLength);
			float distance = bLength * glm::sin(angle);

			if (distance < snapDistance)
			{
				float c = bLength * glm::cos(angle);
				if (c <= roadPrefabLength || c >= road->length - roadPrefabLength)
					continue;

				results.snapLocation = road->GetStartPosition() + glm::normalize(road->direction) * c;
				results.snapped = true;
				results.snappedRoad = road;
				return results;
			}
		}
		*/
		return results;
	}

	void RoadManager::ResetStates()
	{
		m_ConstructionPhase = 0;

		b_ConstructionStartSnapped = false;
		b_ConstructionEndSnapped = false;

		m_ConstructionPositions = {
			glm::vec3(0.0f),
			glm::vec3(0.0f),
			glm::vec3(0.0f)
		};

		m_StartSnappedJunction = nullptr;
		m_StartSnappedEnd = nullptr;
		m_StartSnappedRoadSegment = nullptr;

		m_EndSnappedJunction = nullptr;
		m_EndSnappedEnd = nullptr;
		m_EndSnappedRoadSegment = nullptr;

		m_DestructionSnappedJunction = nullptr;
		m_DestructionSnappedEnd = nullptr;
		m_DestructionSnappedRoadSegment = nullptr;




		for (RoadSegment* roadSegment : m_RoadSegments)
		{
			roadSegment->object->enabled = true;
			roadSegment->object->SetTransform(roadSegment->GetStartPosition());
		}

		for (Junction* junction : m_Junctions)
		{
			junction->object->enabled = true;
			junction->object->SetTransform(junction->position);
		}

		for (End* end : m_Ends)
		{
			end->object->enabled = true;
			end->object->SetTransform(end->position);
		}


		for (std::vector<Object*>& os : m_Guidelines)
			for (Object* rg : os)
				rg->enabled = false;
		for (size_t& inUse : m_GuidelinesInUse)
			inUse = 0;

		m_GuidelinesStart->enabled = false;
		m_GuidelinesEnd->enabled = false;

		m_GuidelinesStart->tintColor = glm::vec4(1.0f);
		m_GuidelinesEnd->tintColor = glm::vec4(1.0f);
	}
}