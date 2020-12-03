#include "canpch.h"
#include "RoadManager.h"

#include "Road.h"
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

		if (b_ConstructionStarted == false)
		{
			if (snapOptions[4])
			{
				prevLocation.x = prevLocation.x - std::fmod(prevLocation.x + 0.25f, 0.5f) + 0.25f;
				prevLocation.z = prevLocation.z - std::fmod(prevLocation.z + 0.25f, 0.5f) - 0.25f;
			}
			if (snapOptions[0])
			{
				SnapInformation snapInformation = CheckSnapping(cameraPosition, cameraDirection);
				prevLocation = snapInformation.snapped ? snapInformation.snapLocation : prevLocation;
				b_ConstructionStartSnapped = snapInformation.snapped;
				m_StartSnappedJunction = snapInformation.snappedJunction;
				m_StartSnappedEnd = snapInformation.snappedEnd;
				m_StartSnappedRoad = snapInformation.snappedRoad;
			}
			m_ConstructionPositions[0] = prevLocation;

			m_GuidelinesStart->SetTransform(prevLocation + glm::vec3{ 0.0f, 0.15f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, glm::radians(180.0f), 0.0f });
			m_GuidelinesEnd->SetTransform(prevLocation + glm::vec3{ 0.0f, 0.15f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f });
		}
		else
		{
			b_ConstructionRestricted = false;
			if (snapOptions[4])
			{
				prevLocation.x = prevLocation.x - std::fmod(prevLocation.x + 0.25f, 0.5f) + 0.25f;
				prevLocation.z = prevLocation.z - std::fmod(prevLocation.z + 0.25f, 0.5f) + 0.25f;
			}
			if (snapOptions[0])
			{
				SnapInformation snapInformation = CheckSnapping(cameraPosition, cameraDirection);
				prevLocation = snapInformation.snapped ? snapInformation.snapLocation : prevLocation;
				b_ConstructionEndSnapped = snapInformation.snapped;
				m_EndSnappedJunction = snapInformation.snappedJunction;
				m_EndSnappedEnd = snapInformation.snappedEnd;
				m_EndSnappedRoad = snapInformation.snappedRoad;
			}
			m_ConstructionPositions[2] = prevLocation;

			bool angleIsRestricted = false;
			if (restrictions[0])
			{
				if (m_StartSnappedJunction)
				{
					for (Road* road : m_StartSnappedJunction->connectedRoads)
					{
						glm::vec3 directionOldRoad = road->startJunction == m_StartSnappedJunction ? road->direction : -road->direction;
						directionOldRoad.y = 0;
						directionOldRoad = glm::normalize(directionOldRoad);

						glm::vec3 directionNewRoad = prevLocation - m_ConstructionPositions[0];
						directionNewRoad.y = 0;
						directionNewRoad = glm::normalize(directionNewRoad);

						float angle = glm::acos(glm::dot(directionOldRoad, directionNewRoad));

						if (angle < 0.5f)
						{
							angleIsRestricted = true;
							break;
						}
					}
				}
				else if (m_StartSnappedEnd)
				{
					Road* road = m_StartSnappedEnd->connectedRoad;

					glm::vec3 directionOldRoad = road->startEnd == m_StartSnappedEnd ? road->direction : -road->direction;
					directionOldRoad.y = 0;
					directionOldRoad = glm::normalize(directionOldRoad);

					glm::vec3 directionNewRoad = prevLocation - m_ConstructionPositions[0];
					directionNewRoad.y = 0;
					directionNewRoad = glm::normalize(directionNewRoad);

					float angle = glm::acos(glm::dot(directionOldRoad, directionNewRoad));

					angleIsRestricted = angle < 0.5f;
				}
				else if (m_StartSnappedRoad)
				{
					glm::vec3 directionOldRoad = m_StartSnappedRoad->direction;

					directionOldRoad.y = 0;
					directionOldRoad = glm::normalize(directionOldRoad);

					glm::vec3 directionNewRoad = prevLocation - m_ConstructionPositions[0];
					directionNewRoad.y = 0;
					directionNewRoad = glm::normalize(directionNewRoad);

					float angle = glm::acos(glm::dot(directionOldRoad, directionNewRoad));

					angleIsRestricted = angle < 0.5f || angle > 2.63f;
				}

				if (m_EndSnappedJunction)
				{
					for (Road* road : m_EndSnappedJunction->connectedRoads)
					{
						glm::vec3 directionOldRoad = road->startJunction == m_EndSnappedJunction ? road->direction : -road->direction;
						directionOldRoad.y = 0;
						directionOldRoad = glm::normalize(directionOldRoad);

						glm::vec3 directionNewRoad = m_ConstructionPositions[0] - prevLocation;
						directionNewRoad.y = 0;
						directionNewRoad = glm::normalize(directionNewRoad);

						float angle = glm::acos(glm::dot(directionOldRoad, directionNewRoad));

						if (angle < 0.5f)
						{
							angleIsRestricted = true;
							break;
						}
					}
				}
				else if (m_EndSnappedEnd)
				{
					Road* road = m_EndSnappedEnd->connectedRoad;

					glm::vec3 directionOldRoad = road->startEnd == m_EndSnappedEnd ? road->direction : -road->direction;
					directionOldRoad.y = 0;
					directionOldRoad = glm::normalize(directionOldRoad);

					glm::vec3 directionNewRoad = m_ConstructionPositions[0] - prevLocation;
					directionNewRoad.y = 0;
					directionNewRoad = glm::normalize(directionNewRoad);

					float angle = glm::acos(glm::dot(directionOldRoad, directionNewRoad));

					angleIsRestricted = angle < 0.5f;
				}
				else if (m_EndSnappedRoad)
				{
					glm::vec3 directionOldRoad = m_EndSnappedRoad->direction;

					directionOldRoad.y = 0;
					directionOldRoad = glm::normalize(directionOldRoad);

					glm::vec3 directionNewRoad = m_ConstructionPositions[0] - prevLocation;
					directionNewRoad.y = 0;
					directionNewRoad = glm::normalize(directionNewRoad);

					float angle = glm::acos(glm::dot(directionOldRoad, directionNewRoad));

					angleIsRestricted = angle < 0.5f || angle > 2.63f;
				}

			}

			bool collisionIsRestricted = false;
			if (restrictions[2])
			{
				glm::vec2 p0 = { m_ConstructionPositions[0].x, m_ConstructionPositions[0].z };
				glm::vec2 p1 = { m_ConstructionPositions[2].x, m_ConstructionPositions[2].z };
				for (Road* road : m_Roads)
				{
					if (road == m_EndSnappedRoad || road == m_StartSnappedRoad)
						continue;
					if (m_StartSnappedEnd && road == m_StartSnappedEnd->connectedRoad)
						continue;
					if (m_EndSnappedEnd && road == m_EndSnappedEnd->connectedRoad)
						continue;
					if (m_StartSnappedJunction)
					{
						auto it = std::find(m_StartSnappedJunction->connectedRoads.begin(), m_StartSnappedJunction->connectedRoads.end(), road);
						if (it != m_StartSnappedJunction->connectedRoads.end())
							continue;
					}
					if (m_EndSnappedJunction)
					{
						auto it = std::find(m_EndSnappedJunction->connectedRoads.begin(), m_EndSnappedJunction->connectedRoads.end(), road);
						if (it != m_EndSnappedJunction->connectedRoads.end())
							continue;
					}
					glm::vec3 roadStart = road->startEnd ? road->GetStartPosition() : road->startJunction->position;
					glm::vec3 roadEnd = road->endEnd ? road->GetEndPosition() : road->endJunction->position;

					glm::vec2 p2 = { roadStart.x, roadStart.z };
					glm::vec2 p3 = { roadEnd.x, roadEnd.z };

					if (Helper::LineSLineSIntersection(p0, p1, p2, p3, nullptr))
					{
						collisionIsRestricted = true;
						break;
					}
					float width = road->object->prefab->boundingBoxM.z - road->object->prefab->boundingBoxL.z;
					if (Helper::DistanceBetweenLineSLineS(p0, p1, p2, p3) < width)
					{
						collisionIsRestricted = true;
						break;
					}
				}
			}

			glm::vec3 AB = m_ConstructionPositions[2] - m_ConstructionPositions[0];

			float rotationOffset = AB.x < 0.0f ? 180.0f : 0.0f;
			float rotationStart = glm::atan(-AB.z / AB.x) + glm::radians(180.0f + rotationOffset);
			float rotationEnd = glm::atan(-AB.z / AB.x) + glm::radians(rotationOffset);

			if (snapOptions[1] && glm::length(AB) > 0.5f)
			{
				float length = glm::length(AB);
				length = length - std::fmod(length, roadPrefabLength);
				AB = length * glm::normalize(AB);
				m_ConstructionPositions[2] = m_ConstructionPositions[0] + AB;
			}

			if (snapOptions[2] && glm::length(AB) > 0.5f)
			{
				if (
					!m_EndSnappedJunction &&
					!m_EndSnappedRoad &&
					!m_EndSnappedEnd
					)
					m_ConstructionPositions[2].y = m_ConstructionPositions[0].y;
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
					m_ConstructionPositions[2] = m_ConstructionPositions[0] + AB;
				}
				else if (m_StartSnappedRoad)
				{
					float snappedRoadRotationY = glm::degrees(m_StartSnappedRoad->rotation.y);
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
					m_ConstructionPositions[2] = m_ConstructionPositions[0] + AB;
				}
				else if (m_StartSnappedJunction)
				{
					float newRoadRotationY = glm::degrees(rotationEnd);
					float smallestAngle = 180.0f;
					for (Road* road : m_StartSnappedJunction->connectedRoads)
					{
						float snappedRoadRotationY = m_StartSnappedJunction == road->startJunction ? glm::degrees(road->rotation.y) : glm::degrees(road->rotation.y) + 180.0f;
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
					m_ConstructionPositions[2] = m_ConstructionPositions[0] + AB;
				}
				else if (Input::IsKeyPressed(KeyCode::LeftControl))
				{
					float angle = std::fmod(glm::degrees(rotationEnd) + 720.0f, 360.0f);
					float newAngle = angle + 2.5f - std::fmod(angle + 2.5f, 5.0f);

					AB = glm::rotate(AB, -glm::radians(angle - newAngle), { 0.0f, 1.0f, 0.0f });
					m_ConstructionPositions[2] = m_ConstructionPositions[0] + AB;
				}
			}

			if (glm::length(AB) > 0.5f)
			{
				if (m_EndSnappedRoad)
				{
					glm::vec3 n = { -m_EndSnappedRoad->direction.z,0,m_EndSnappedRoad->direction.x };
					m_ConstructionPositions[2] = Helper::RayPlaneIntersection(
						m_ConstructionPositions[0],
						AB,
						m_EndSnappedRoad->GetStartPosition(),
						n
					);
					AB = m_ConstructionPositions[2] - m_ConstructionPositions[0];
				}
				else if (m_EndSnappedEnd)
				{
					m_ConstructionPositions[2] = m_EndSnappedEnd->position;
					AB = m_ConstructionPositions[2] - m_ConstructionPositions[0];
				}
				else if (m_EndSnappedJunction)
				{
					m_ConstructionPositions[2] = m_EndSnappedJunction->position;
					AB = m_ConstructionPositions[2] - m_ConstructionPositions[0];
				}
			}

			glm::vec3 normalizedAB = glm::normalize(AB);

			rotationOffset = (AB.x < 0.0f) * 180.0f;
			rotationStart = glm::atan(-AB.z / AB.x) + glm::radians(180.0f + rotationOffset);
			rotationEnd = glm::atan(-AB.z / AB.x) + glm::radians(rotationOffset);

			glm::vec2 least = { -roadPrefabWidth / 2.0f, -roadPrefabWidth / 2.0f };
			glm::vec2 most = { glm::length(AB) + roadPrefabWidth / 2.0f, roadPrefabWidth / 2.0f };
			if (m_StartSnappedEnd || m_StartSnappedJunction || m_StartSnappedRoad)
				least.x = 0.0f;
			if (m_EndSnappedEnd || m_EndSnappedJunction || m_EndSnappedRoad)
				most.x = glm::length(AB);

			if (m_Scene->m_BuildingManager.restrictions[0] && restrictions[2])
			{
				for (Building* building : m_Scene->m_BuildingManager.GetBuildings())
				{
					glm::vec2 mtv = Helper::CheckRotatedRectangleCollision(
						least,
						most,
						rotationEnd,
						glm::vec2{ m_ConstructionPositions[0].x, m_ConstructionPositions[0].z },
						glm::vec2{ building->object->prefab->boundingBoxL.x ,building->object->prefab->boundingBoxL.z },
						glm::vec2{ building->object->prefab->boundingBoxM.x ,building->object->prefab->boundingBoxM.z },
						building->object->rotation.y,
						glm::vec2{ building->position.x,building->position.z }
					);
					building->object->tintColor = glm::vec4(1.0f);
					if (mtv.x != 0.0f || mtv.y != 0.0f)
						building->object->tintColor = glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f };
				}
			}

			if (m_Scene->m_TreeManager.restrictions[0] && restrictions[2])
			{
				for (Object* tree : m_Scene->m_TreeManager.GetTrees())
				{
					glm::vec2 mtv = Helper::CheckRotatedRectangleCollision(
						least,
						most,
						rotationEnd,
						glm::vec2{ m_ConstructionPositions[0].x, m_ConstructionPositions[0].z },
						glm::vec2{ tree->prefab->boundingBoxL.x * tree->scale.x ,tree->prefab->boundingBoxL.z * tree->scale.z },
						glm::vec2{ tree->prefab->boundingBoxM.x * tree->scale.x ,tree->prefab->boundingBoxM.z * tree->scale.z },
						tree->rotation.y,
						glm::vec2{ tree->position.x, tree->position.z }
					);
					tree->tintColor = glm::vec4(1.0f);
					if (mtv.x != 0.0f || mtv.y != 0.0f)
						tree->tintColor = glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f };
				}
			}

			m_GuidelinesStart->enabled = !b_ConstructionStartSnapped;
			m_GuidelinesEnd->enabled = !b_ConstructionEndSnapped;

			m_GuidelinesStart->SetTransform(m_ConstructionPositions[0] + glm::vec3{ 0.0f, 0.15f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, rotationStart, 0.0f });
			m_GuidelinesEnd->SetTransform(m_ConstructionPositions[2] + glm::vec3{ 0.0f, 0.15f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, rotationEnd, 0.0f });

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


			for (std::vector<Object*>& os : m_Guidelines)
				for (Object* rg : os)
					rg->enabled = false;

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

			if (m_StartSnappedRoad != nullptr)
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

			if (m_EndSnappedRoad != nullptr)
			{
				float snappedRoadPrefabLength = m_EndSnappedRoad->type[0]->boundingBoxM.x - m_EndSnappedRoad->type[0]->boundingBoxL.x;
				size_t snappedRoadTypeIndex = m_EndSnappedRoad->typeIndex;
				m_EndSnappedRoad->object->enabled = false;
				glm::vec3 R0I = m_ConstructionPositions[2] - m_EndSnappedRoad->GetStartPosition();
				glm::vec3 R1I = m_ConstructionPositions[2] - m_EndSnappedRoad->GetEndPosition();

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

			b_ConstructionRestricted |= angleIsRestricted;
			b_ConstructionRestricted |= lengthIsRestricted;
			b_ConstructionRestricted |= collisionIsRestricted;

			m_GuidelinesStart->tintColor = b_ConstructionRestricted ? glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f } : glm::vec4(1.0f);
			m_GuidelinesEnd->tintColor = b_ConstructionRestricted ? glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f } : glm::vec4(1.0f);

			for (std::vector<Object*>& os : m_Guidelines)
				for (Object* rg : os)
					rg->tintColor = b_ConstructionRestricted ? glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f } : glm::vec4(1.0f);

		}
	}
	void RoadManager::OnUpdate_QuadraticCurve(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
	{
		Prefab* selectedRoad = m_Scene->MainApplication->roads[m_Type][0];
		float roadPrefabWidth = selectedRoad->boundingBoxM.z - selectedRoad->boundingBoxL.z;
		float roadPrefabLength = selectedRoad->boundingBoxM.x - selectedRoad->boundingBoxL.x;

		if (b_ConstructionStarted == false)
		{
			if (snapOptions[4])
			{
				prevLocation.x = prevLocation.x - std::fmod(prevLocation.x + 0.25f, 0.5f) + 0.25f;
				prevLocation.z = prevLocation.z - std::fmod(prevLocation.z + 0.25f, 0.5f) - 0.25f;
			}
			if (snapOptions[0])
			{
				SnapInformation snapInformation = CheckSnapping(cameraPosition, cameraDirection);
				prevLocation = snapInformation.snapped ? snapInformation.snapLocation : prevLocation;
				b_ConstructionStartSnapped = snapInformation.snapped;
				m_StartSnappedJunction = snapInformation.snappedJunction;
				m_StartSnappedEnd = snapInformation.snappedEnd;
				m_StartSnappedRoad = snapInformation.snappedRoad;
			}
			//m_ConstructionPositions[0] = prevLocation;

			m_GuidelinesStart->SetTransform(prevLocation + glm::vec3{ 0.0f, 0.15f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, glm::radians(180.0f), 0.0f });
			m_GuidelinesEnd->SetTransform(prevLocation + glm::vec3{ 0.0f, 0.15f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f });
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
				prevLocation = snapInformation.snapped ? snapInformation.snapLocation : prevLocation;
				b_ConstructionStartSnapped = snapInformation.snapped;
				m_StartSnappedJunction = snapInformation.snappedJunction;
				m_StartSnappedEnd = snapInformation.snappedEnd;
				m_StartSnappedRoad = snapInformation.snappedRoad;
			}
			//m_ConstructionPositions[0] = prevLocation;

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
			m_ConstructionPositions[cubicCurveOrder[2]] = prevLocation;
			m_ConstructionPositions[cubicCurveOrder[3]] = prevLocation;

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
				m_ConstructionPositions[2] = m_ConstructionPositions[0] + AB;
			}

			if (snapOptions[2] && glm::length(AB) > 0.5f)
			{
				if (
					!m_EndSnappedJunction &&
					!m_EndSnappedRoad &&
					!m_EndSnappedEnd
					)
					m_ConstructionPositions[2].y = m_ConstructionPositions[0].y;
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
					m_ConstructionPositions[2] = m_ConstructionPositions[0] + AB;
				}
				else if (m_StartSnappedRoad)
				{
					float snappedRoadRotationY = glm::degrees(m_StartSnappedRoad->rotation.y);
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
					m_ConstructionPositions[2] = m_ConstructionPositions[0] + AB;
				}
				else if (m_StartSnappedJunction)
				{
					float newRoadRotationY = glm::degrees(rotationEnd);
					float smallestAngle = 180.0f;
					for (Road* road : m_StartSnappedJunction->connectedRoads)
					{
						float snappedRoadRotationY = m_StartSnappedJunction == road->startJunction ? glm::degrees(road->rotation.y) : glm::degrees(road->rotation.y) + 180.0f;
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
					m_ConstructionPositions[2] = m_ConstructionPositions[0] + AB;
				}
				else if (Input::IsKeyPressed(KeyCode::LeftControl))
				{
					float angle = std::fmod(glm::degrees(rotationEnd) + 720.0f, 360.0f);
					float newAngle = angle + 2.5f - std::fmod(angle + 2.5f, 5.0f);

					AB = glm::rotate(AB, -glm::radians(angle - newAngle), { 0.0f, 1.0f, 0.0f });
					m_ConstructionPositions[2] = m_ConstructionPositions[0] + AB;
				}
			}

			if (glm::length(AB) > 0.5f)
			{
				if (m_EndSnappedRoad)
				{
					glm::vec3 n = { -m_EndSnappedRoad->direction.z,0,m_EndSnappedRoad->direction.x };
					m_ConstructionPositions[2] = Helper::RayPlaneIntersection(
						m_ConstructionPositions[0],
						AB,
						m_EndSnappedRoad->GetStartPosition(),
						n
					);
					AB = m_ConstructionPositions[2] - m_ConstructionPositions[0];
				}
				else if (m_EndSnappedEnd)
				{
					m_ConstructionPositions[2] = m_EndSnappedEnd->position;
					AB = m_ConstructionPositions[2] - m_ConstructionPositions[0];
				}
				else if (m_EndSnappedJunction)
				{
					m_ConstructionPositions[2] = m_EndSnappedJunction->position;
					AB = m_ConstructionPositions[2] - m_ConstructionPositions[0];
				}
			}

			glm::vec3 normalizedAB = glm::normalize(AB);

			rotationOffset = (AB.x < 0.0f) * 180.0f;
			rotationStart = glm::atan(-AB.z / AB.x) + glm::radians(180.0f + rotationOffset);
			rotationEnd = glm::atan(-AB.z / AB.x) + glm::radians(rotationOffset);

			glm::vec2 least = { -roadPrefabWidth / 2.0f, -roadPrefabWidth / 2.0f };
			glm::vec2 most = { glm::length(AB) + roadPrefabWidth / 2.0f, roadPrefabWidth / 2.0f };
			if (m_StartSnappedEnd || m_StartSnappedJunction || m_StartSnappedRoad)
				least.x = 0.0f;
			if (m_EndSnappedEnd || m_EndSnappedJunction || m_EndSnappedRoad)
				most.x = glm::length(AB);

			m_GuidelinesStart->enabled = !b_ConstructionStartSnapped;
			m_GuidelinesEnd->enabled = !b_ConstructionEndSnapped;

			m_GuidelinesStart->SetTransform(m_ConstructionPositions[0] + glm::vec3{ 0.0f, 0.15f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, rotationStart, 0.0f });
			m_GuidelinesEnd->SetTransform(m_ConstructionPositions[2] + glm::vec3{ 0.0f, 0.15f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, rotationEnd, 0.0f });

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

			if (m_StartSnappedRoad != nullptr)
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
			m_ConstructionPositions[cubicCurveOrder[3]] = prevLocation;

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

			glm::vec3 AB1 = m_ConstructionPositions[1] - m_ConstructionPositions[0];
			glm::vec3 AB2 = m_ConstructionPositions[2] - m_ConstructionPositions[3];

			float rotationOffset1 = AB1.x < 0.0f ? 180.0f : 0.0f;
			float rotationOffset2 = AB2.x < 0.0f ? 180.0f : 0.0f;

			float rotationStart = glm::atan(-AB1.z / AB1.x) + glm::radians(rotationOffset1 + 180.0f);
			float rotationEnd = glm::atan(-AB2.z / AB2.x) + glm::radians(rotationOffset2 + 180.0f);

			m_GuidelinesStart->enabled = !b_ConstructionStartSnapped;
			m_GuidelinesEnd->enabled = !b_ConstructionEndSnapped;

			m_GuidelinesStart->SetTransform(m_ConstructionPositions[0] + glm::vec3{ 0.0f, 0.15f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, rotationStart, 0.0f });
			m_GuidelinesEnd->SetTransform(m_ConstructionPositions[2] + glm::vec3{ 0.0f, 0.15f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, rotationEnd, 0.0f });

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
		}
		else if (m_ConstructionPhase == 3)
		{

			b_ConstructionRestricted = false;
			m_ConstructionPositions[cubicCurveOrder[3]] = prevLocation;

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

			glm::vec3 AB1 = m_ConstructionPositions[1] - m_ConstructionPositions[0];
			glm::vec3 AB2 = m_ConstructionPositions[2] - m_ConstructionPositions[3];

			float rotationOffset1 = AB1.x < 0.0f ? 180.0f : 0.0f;
			float rotationOffset2 = AB2.x < 0.0f ? 180.0f : 0.0f;

			float rotationStart = glm::atan(-AB1.z / AB1.x) + glm::radians(rotationOffset1 + 180.0f);
			float rotationEnd = glm::atan(-AB2.z / AB2.x) + glm::radians(rotationOffset2 + 180.0f);

			m_GuidelinesStart->enabled = !b_ConstructionStartSnapped;
			m_GuidelinesEnd->enabled = !b_ConstructionEndSnapped;

			m_GuidelinesStart->SetTransform(m_ConstructionPositions[0] + glm::vec3{ 0.0f, 0.15f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, rotationStart, 0.0f });
			m_GuidelinesEnd->SetTransform(m_ConstructionPositions[2] + glm::vec3{ 0.0f, 0.15f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, rotationEnd, 0.0f });

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
		}
	}
	void RoadManager::OnUpdate_Destruction(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
	{
		SnapInformation snapInformation = CheckSnapping(cameraPosition, cameraDirection);
		m_DestructionSnappedJunction = snapInformation.snappedJunction;
		m_DestructionSnappedEnd = snapInformation.snappedEnd;
		m_DestructionSnappedRoad = snapInformation.snappedRoad;

		for (Junction* junction : m_Junctions)
			junction->object->SetTransform(junction->position);

		for (End* end : m_Ends)
			end->object->SetTransform(end->position);

		for (Road* road : m_Roads)
			road->object->SetTransform(road->GetStartPosition());

		if (snapInformation.snapped)
		{
			if (m_DestructionSnappedJunction != nullptr)
			{
				m_DestructionSnappedJunction->object->SetTransform(m_DestructionSnappedJunction->position + glm::vec3{ 0.0f, 0.1f, 0.0f });

				for (Road* road : m_DestructionSnappedJunction->connectedRoads)
				{
					if (road->startEnd != nullptr)
						road->startEnd->object->SetTransform(road->startEnd->position + glm::vec3{ 0.0f, 0.1f, 0.0f });
					if (road->endEnd != nullptr)
						road->endEnd->object->SetTransform(road->endEnd->position + glm::vec3{ 0.0f, 0.1f, 0.0f });
					road->object->SetTransform(road->GetStartPosition() + glm::vec3{ 0.0f, 0.1f, 0.0f });
				}
			}
			else if (m_DestructionSnappedEnd != nullptr)
			{
				Road* road = m_DestructionSnappedEnd->connectedRoad;

				if (road->startEnd != nullptr)
					road->startEnd->object->SetTransform(road->startEnd->position + glm::vec3{ 0.0f, 0.1f, 0.0f });
				if (road->endEnd != nullptr)
					road->endEnd->object->SetTransform(road->endEnd->position + glm::vec3{ 0.0f, 0.1f, 0.0f });
				road->object->SetTransform(road->GetStartPosition() + glm::vec3{ 0.0f, 0.1f, 0.0f });
			}
			else
			{
				if (m_DestructionSnappedRoad->startEnd != nullptr)
					m_DestructionSnappedRoad->startEnd->object->SetTransform(m_DestructionSnappedRoad->startEnd->position + glm::vec3{ 0.0f, 0.1f, 0.0f });
				if (m_DestructionSnappedRoad->endEnd != nullptr)
					m_DestructionSnappedRoad->endEnd->object->SetTransform(m_DestructionSnappedRoad->endEnd->position + glm::vec3{ 0.0f, 0.1f, 0.0f });
				m_DestructionSnappedRoad->object->SetTransform(m_DestructionSnappedRoad->GetStartPosition() + glm::vec3{ 0.0f, 0.1f, 0.0f });
			}
		}
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
			if (!b_ConstructionStarted)
			{
				b_ConstructionStarted = true;
				if (!b_ConstructionStartSnapped)
					m_ConstructionPositions[0] = intersection;
			}
			else if (!b_ConstructionRestricted)
			{
				b_ConstructionEnded = true;
				/*if (!b_RoadConstructionEndSnapped)
					m_EndPosition = intersection;*/
				for (std::vector<Object*>& os : m_Guidelines)
					for (Object* rg : os)
						rg->enabled = false;
				for (size_t& inUse : m_GuidelinesInUse)
					inUse = 0;
			}
		}

		if (b_ConstructionEnded) // [[unlikely]] 
		{
			Road* newRoad = new Road(
				m_Scene->MainApplication->roads[m_Type][0],
				m_ConstructionPositions[0],
				m_ConstructionPositions[2],
				{
					 m_Scene->MainApplication->roads[m_Type][0],
					 m_Scene->MainApplication->roads[m_Type][1],
					 m_Scene->MainApplication->roads[m_Type][2]
				},
				m_Type
			);
			m_Roads.push_back(newRoad);

			float roadPrefabWidth = m_Scene->MainApplication->roads[m_Type][0]->boundingBoxM.z - m_Scene->MainApplication->roads[m_Type][0]->boundingBoxL.z;
			glm::vec3 AB = m_ConstructionPositions[2] - m_ConstructionPositions[0];
			float rotation = glm::atan(-AB.z / AB.x) + glm::radians((AB.x < 0.0f) * 180.0f);

			glm::vec2 least = { -roadPrefabWidth / 2.0f, -roadPrefabWidth / 2.0f };
			glm::vec2 most = { glm::length(AB) + roadPrefabWidth / 2.0f, roadPrefabWidth / 2.0f };
			if (m_StartSnappedEnd || m_StartSnappedJunction || m_StartSnappedRoad)
				least.x = 0.0f;
			if (m_EndSnappedEnd || m_EndSnappedJunction || m_EndSnappedRoad)
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
						if (building->connectedRoad)
						{
							auto it = std::find(
								building->connectedRoad->connectedBuildings.begin(),
								building->connectedRoad->connectedBuildings.end(),
								building
							);
							building->connectedRoad->connectedBuildings.erase(it);
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


			if (m_StartSnappedJunction != nullptr)
			{
				newRoad->startJunction = m_StartSnappedJunction;
				m_StartSnappedJunction->connectedRoads.push_back(newRoad);
				m_StartSnappedJunction->ReconstructObject();
			}
			else if (m_StartSnappedEnd != nullptr)
			{
				Road* connectedRoad = m_StartSnappedEnd->connectedRoad;
				if (connectedRoad->startEnd == m_StartSnappedEnd)
					connectedRoad->startEnd = nullptr;
				else
					connectedRoad->endEnd = nullptr;

				Junction* newJunction = new Junction(std::vector<Road*>{ connectedRoad, newRoad }, m_StartSnappedEnd->object->position);
				newRoad->startJunction = newJunction;

				if (connectedRoad->GetStartPosition() == m_StartSnappedEnd->object->position)
					connectedRoad->startJunction = newJunction;
				else
					connectedRoad->endJunction = newJunction;


				auto position = std::find(m_Ends.begin(), m_Ends.end(), m_StartSnappedEnd);
				m_Ends.erase(position);
				delete m_StartSnappedEnd;

				m_Junctions.push_back(newJunction);
				newJunction->ConstructObject();
			}
			else if (m_StartSnappedRoad != nullptr)
			{
				Road* r1 = new Road(
					m_StartSnappedRoad,
					m_StartSnappedRoad->GetStartPosition(),
					m_ConstructionPositions[0]
				);
				if (m_StartSnappedRoad->startJunction != nullptr)
				{
					auto it = std::find(
						m_StartSnappedRoad->startJunction->connectedRoads.begin(),
						m_StartSnappedRoad->startJunction->connectedRoads.end(),
						m_StartSnappedRoad
					);
					m_StartSnappedRoad->startJunction->connectedRoads.erase(it);
					m_StartSnappedRoad->startJunction->connectedRoads.push_back(r1);
					r1->startJunction = m_StartSnappedRoad->startJunction;
				}
				else
				{
					r1->startEnd = m_StartSnappedRoad->startEnd;
					m_StartSnappedRoad->startEnd->connectedRoad = r1;
				}
				m_Roads.push_back(r1);


				Road* r2 = new Road(
					m_StartSnappedRoad,
					m_StartSnappedRoad->GetEndPosition(),
					m_ConstructionPositions[0]
				);
				if (m_StartSnappedRoad->endJunction != nullptr)
				{
					auto it = std::find(
						m_StartSnappedRoad->endJunction->connectedRoads.begin(),
						m_StartSnappedRoad->endJunction->connectedRoads.end(),
						m_StartSnappedRoad
					);
					m_StartSnappedRoad->endJunction->connectedRoads.erase(it);
					m_StartSnappedRoad->endJunction->connectedRoads.push_back(r2);
					r2->startJunction = m_StartSnappedRoad->endJunction;
				}
				else
				{
					r2->startEnd = m_StartSnappedRoad->endEnd;
					m_StartSnappedRoad->endEnd->connectedRoad = r2;
				}
				m_Roads.push_back(r2);

				float roadWidth = r1->object->prefab->boundingBoxM.z - r1->object->prefab->boundingBoxL.z;
				r1->object->enabled = true;

				for (Building* building : m_StartSnappedRoad->connectedBuildings)
				{
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
					}
				}

				auto it = std::find(m_Roads.begin(), m_Roads.end(), m_StartSnappedRoad);
				m_Roads.erase(it);
				delete m_StartSnappedRoad;

				Junction* newJunction = new Junction(std::vector<Road*>{ newRoad, r1, r2 }, m_ConstructionPositions[0]);
				m_Junctions.push_back(newJunction);
				newRoad->startJunction = newJunction;
				r1->endJunction = newJunction;
				r2->endJunction = newJunction;
				newJunction->ConstructObject();
			}
			else
			{
				End* newEnd = new End(
					newRoad,
					m_Scene->MainApplication->roads[m_Type][2],
					m_ConstructionPositions[0],
					glm::vec3{ 1.0f, 1.0f, 1.0f },
					glm::vec3{ 0.0f, newRoad->rotation.y + glm::radians(180.0f), -newRoad->rotation.z }
				);
				newRoad->startEnd = newEnd;
				m_Ends.push_back(newEnd);
			}

			if (m_EndSnappedJunction != nullptr)
			{
				newRoad->endJunction = m_EndSnappedJunction;
				m_EndSnappedJunction->connectedRoads.push_back(newRoad);
				m_EndSnappedJunction->ReconstructObject();
			}
			else if (m_EndSnappedEnd != nullptr)
			{
				Road* connectedRoad = m_EndSnappedEnd->connectedRoad;
				if (connectedRoad->startEnd == m_EndSnappedEnd)
					connectedRoad->startEnd = nullptr;
				else
					connectedRoad->endEnd = nullptr;


				Junction* newJunction = new Junction(std::vector<Road*>{ connectedRoad, newRoad }, m_EndSnappedEnd->object->position);
				newRoad->endJunction = newJunction;

				if (connectedRoad->GetStartPosition() == m_EndSnappedEnd->object->position)
					connectedRoad->startJunction = newJunction;
				else
					connectedRoad->endJunction = newJunction;


				auto position = std::find(m_Ends.begin(), m_Ends.end(), m_EndSnappedEnd);
				m_Ends.erase(position);
				delete m_EndSnappedEnd;

				m_Junctions.push_back(newJunction);
				newJunction->ConstructObject();
			}
			else if (m_EndSnappedRoad != nullptr)
			{
				Road* r1 = new Road(
					m_EndSnappedRoad,
					m_EndSnappedRoad->GetStartPosition(),
					m_ConstructionPositions[2]
				);
				if (m_EndSnappedRoad->startJunction != nullptr)
				{
					auto it = std::find(
						m_EndSnappedRoad->startJunction->connectedRoads.begin(),
						m_EndSnappedRoad->startJunction->connectedRoads.end(),
						m_EndSnappedRoad
					);
					m_EndSnappedRoad->startJunction->connectedRoads.erase(it);
					m_EndSnappedRoad->startJunction->connectedRoads.push_back(r1);
					r1->startJunction = m_EndSnappedRoad->startJunction;
				}
				else
				{
					r1->startEnd = m_EndSnappedRoad->startEnd;
					m_EndSnappedRoad->startEnd->connectedRoad = r1;
				}
				m_Roads.push_back(r1);


				Road* r2 = new Road(
					m_EndSnappedRoad,
					m_EndSnappedRoad->GetEndPosition(),
					m_ConstructionPositions[2]
				);
				if (m_EndSnappedRoad->endJunction != nullptr)
				{
					auto it = std::find(
						m_EndSnappedRoad->endJunction->connectedRoads.begin(),
						m_EndSnappedRoad->endJunction->connectedRoads.end(),
						m_EndSnappedRoad
					);
					m_EndSnappedRoad->endJunction->connectedRoads.erase(it);
					m_EndSnappedRoad->endJunction->connectedRoads.push_back(r2);
					r2->startJunction = m_EndSnappedRoad->endJunction;
				}
				else
				{
					r2->startEnd = m_EndSnappedRoad->endEnd;
					m_EndSnappedRoad->endEnd->connectedRoad = r2;
				}
				m_Roads.push_back(r2);

				for (Building* building : m_EndSnappedRoad->connectedBuildings)
				{
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
					}
				}

				auto it = std::find(m_Roads.begin(), m_Roads.end(), m_EndSnappedRoad);
				m_Roads.erase(it);
				delete m_EndSnappedRoad;

				Junction* newJunction = new Junction(std::vector<Road*>{ newRoad, r1, r2 }, m_ConstructionPositions[2]);
				m_Junctions.push_back(newJunction);
				newRoad->endJunction = newJunction;
				r1->endJunction = newJunction;
				r2->endJunction = newJunction;
				newJunction->ConstructObject();
			}
			else
			{
				End* newEnd = new End(
					newRoad,
					m_Scene->MainApplication->roads[m_Type][2],
					m_ConstructionPositions[2],
					glm::vec3{ 1.0f, 1.0f, 1.0f },
					glm::vec3{ 0.0f, newRoad->rotation.y, newRoad->rotation.z }
				);
				newRoad->endEnd = newEnd;
				m_Ends.push_back(newEnd);
			}

			//Helper::LevelTheTerrain(startIndex, endIndex, newRoad->startPos, newRoad->endPos,  m_Scene->MainApplication->m_Terrain, roadPrefabWidth);
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
			RoadSegment* newRoadSegment = new RoadSegment(
				m_Scene->MainApplication->roads[m_Type],
				{
					m_ConstructionPositions[0],
					(m_ConstructionPositions[0] + m_ConstructionPositions[1]) / 2.0f,
					(m_ConstructionPositions[1] + m_ConstructionPositions[2]) / 2.0f,
					m_ConstructionPositions[2]
				}
			);
			m_RoadSegments.push_back(newRoadSegment);
			m_ConstructionPhase = 0;
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
			RoadSegment* newRoadSegment = new RoadSegment(
				m_Scene->MainApplication->roads[m_Type],
				{
					m_ConstructionPositions[0],
					m_ConstructionPositions[1],
					m_ConstructionPositions[2],
					m_ConstructionPositions[3]
				}
			);
			m_RoadSegments.push_back(newRoadSegment);
			m_ConstructionPhase = 0;
		}
		return false;
	}
	bool RoadManager::OnMousePressed_Destruction()
	{
		if (m_DestructionSnappedJunction != nullptr)
		{
			std::vector<Road*> roads_copy = m_DestructionSnappedJunction->connectedRoads;
			for (Road* road : roads_copy)
				Remove(road);
		}
		else if (m_DestructionSnappedEnd != nullptr)
		{
			Remove(m_DestructionSnappedEnd->connectedRoad);
		}
		else if (m_DestructionSnappedRoad != nullptr)
		{
			Remove(m_DestructionSnappedRoad);
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

	void RoadManager::Remove(Road* road)
	{
		if (road->startEnd != nullptr)
		{
			auto endPosition = std::find(m_Ends.begin(), m_Ends.end(), road->startEnd);
			m_Ends.erase(endPosition);
			delete road->startEnd;
		}
		else
		{
			Junction* junction = road->startJunction;
			std::vector<Road*>& connectedRoads = junction->connectedRoads;
			auto roadPosition = std::find(connectedRoads.begin(), connectedRoads.end(), road);
			connectedRoads.erase(roadPosition);
			if (connectedRoads.size() == 1)
			{
				Road* otherRoad = connectedRoads[0];
				if (otherRoad->startJunction == junction)
				{
					End* newEnd = new End(
						otherRoad,
						m_Scene->MainApplication->roads[otherRoad->typeIndex][2],
						junction->position,
						glm::vec3{ 1.0f, 1.0f, 1.0f },
						glm::vec3{ 0.0f, otherRoad->rotation.y + glm::radians(180.0f), otherRoad->rotation.z }
					);
					otherRoad->startEnd = newEnd;
					otherRoad->startJunction = nullptr;
					otherRoad->SetStartPosition(junction->position);
					m_Ends.push_back(newEnd);
				}
				else
				{
					End* newEnd = new End(
						otherRoad,
						m_Scene->MainApplication->roads[otherRoad->typeIndex][2],
						junction->position,
						glm::vec3{ 1.0f, 1.0f, 1.0f },
						glm::vec3{ 0.0f, otherRoad->rotation.y, otherRoad->rotation.z }
					);
					otherRoad->endEnd = newEnd;
					otherRoad->endJunction = nullptr;
					otherRoad->SetEndPosition(junction->position);
					m_Ends.push_back(newEnd);
				}

				auto juncPosition = std::find(m_Junctions.begin(), m_Junctions.end(), junction);
				m_Junctions.erase(juncPosition);
				delete junction;
			}
			else
				junction->ReconstructObject();
		}

		if (road->endEnd != nullptr)
		{
			auto endPosition = std::find(m_Ends.begin(), m_Ends.end(), road->endEnd);
			m_Ends.erase(endPosition);
			delete road->endEnd;
		}
		else
		{
			Junction* junction = road->endJunction;
			std::vector<Road*>& connectedRoads = junction->connectedRoads;
			auto roadPosition = std::find(connectedRoads.begin(), connectedRoads.end(), road);
			connectedRoads.erase(roadPosition);
			if (connectedRoads.size() == 1)
			{
				Road* otherRoad = connectedRoads[0];
				if (otherRoad->startJunction == junction)
				{
					End* newEnd = new End(
						otherRoad,
						m_Scene->MainApplication->roads[otherRoad->typeIndex][2],
						junction->position,
						glm::vec3{ 1.0f, 1.0f, 1.0f },
						glm::vec3{ 0.0f, otherRoad->rotation.y + glm::radians(180.0f), otherRoad->rotation.z }
					);
					otherRoad->startEnd = newEnd;
					otherRoad->startJunction = nullptr;
					otherRoad->SetStartPosition(junction->position);
					m_Ends.push_back(newEnd);
				}
				else
				{
					End* newEnd = new End(
						otherRoad,
						m_Scene->MainApplication->roads[otherRoad->typeIndex][2],
						junction->position,
						glm::vec3{ 1.0f, 1.0f, 1.0f },
						glm::vec3{ 0.0f, otherRoad->rotation.y, otherRoad->rotation.z }
					);
					otherRoad->endEnd = newEnd;
					otherRoad->endJunction = nullptr;
					otherRoad->SetEndPosition(junction->position);
					m_Ends.push_back(newEnd);
				}

				auto juncPosition = std::find(m_Junctions.begin(), m_Junctions.end(), junction);
				m_Junctions.erase(juncPosition);
				delete junction;
			}
			else
				junction->ReconstructObject();
		}
		auto position = std::find(m_Roads.begin(), m_Roads.end(), road);
		m_Roads.erase(position);

		for (Building* building : road->connectedBuildings)
		{
			m_Scene->m_BuildingManager.GetBuildings().erase(std::find(m_Scene->m_BuildingManager.GetBuildings().begin(), m_Scene->m_BuildingManager.GetBuildings().end(), building));
			delete building;
		}

		delete road;
	}

	SnapInformation RoadManager::CheckSnapping(const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
	{
		Prefab* selectedRoad = m_Scene->MainApplication->roads[m_Type][0];
		float roadPrefabWidth = selectedRoad->boundingBoxM.z - selectedRoad->boundingBoxL.z;
		float snapDistance = roadPrefabWidth;
		SnapInformation results{ false, { 0.0f, 0.0f, 0.0f } };

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
		for (Road* road : m_Roads)
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
		return results;
	}

	void RoadManager::ResetStates()
	{
		m_ConstructionPhase = 0;

		b_ConstructionStarted = false;
		b_ConstructionEnded = false;
		b_ConstructionStartSnapped = false;
		b_ConstructionEndSnapped = false;

		m_ConstructionPositions = {
			glm::vec3(0.0f),
			glm::vec3(0.0f),
			glm::vec3(0.0f)
		};

		m_StartSnappedJunction = nullptr;
		m_StartSnappedEnd = nullptr;
		m_StartSnappedRoad = nullptr;

		m_EndSnappedJunction = nullptr;
		m_EndSnappedEnd = nullptr;
		m_EndSnappedRoad = nullptr;

		m_DestructionSnappedJunction = nullptr;
		m_DestructionSnappedEnd = nullptr;
		m_DestructionSnappedRoad = nullptr;




		for (Road* road : m_Roads)
		{
			road->object->enabled = true;
			road->object->SetTransform(road->GetStartPosition());
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