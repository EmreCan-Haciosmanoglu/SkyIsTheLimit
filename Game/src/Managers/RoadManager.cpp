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

	void RoadManager::OnUpdate(glm::vec3& prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
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
	void RoadManager::OnUpdate_Straight(glm::vec3& prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
	{
		Prefab* selectedRoad = m_Scene->MainApplication->roads[m_Type][0];
		float roadPrefabWidth = selectedRoad->boundingBoxM.z - selectedRoad->boundingBoxL.z;
		float roadPrefabLength = selectedRoad->boundingBoxM.x - selectedRoad->boundingBoxL.x;

		if (m_ConstructionPhase == 0)
		{
			if (snapOptions[4])
			{
				prevLocation.x = prevLocation.x - std::fmod(prevLocation.x, 0.5f) + 0.25f;
				prevLocation.z = prevLocation.z - std::fmod(prevLocation.z, 0.5f) - 0.25f;
			}
			if (snapOptions[0])
			{
				SnapInformation snapInformation = CheckSnapping(prevLocation);
				prevLocation = snapInformation.location;
				b_ConstructionStartSnapped = snapInformation.snapped;

				m_StartSnappedJunction = snapInformation.junction;
				m_StartSnappedEnd = snapInformation.end;
				m_StartSnappedRoadSegment = snapInformation.roadSegment;

				m_StartSnappedRoadSegmentT = snapInformation.T;
			}
			m_ConstructionPositions[0] = prevLocation;

			m_GuidelinesStart->SetTransform(prevLocation + glm::vec3{ 0.0f, 0.15f, 0.0f }, glm::vec3(1.0f), glm::vec3{ 0.0f, glm::radians(180.0f), 0.0f });
			m_GuidelinesEnd->SetTransform(prevLocation + glm::vec3{ 0.0f, 0.15f, 0.0f }, glm::vec3(1.0f), glm::vec3(0.0f));
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
				prevLocation.x = prevLocation.x - std::fmod(prevLocation.x, 0.5f) + 0.25f;
				prevLocation.z = prevLocation.z - std::fmod(prevLocation.z, 0.5f) - 0.25f;
			}
			if (snapOptions[0])
			{
				SnapInformation snapInformation = CheckSnapping(prevLocation);
				prevLocation = snapInformation.location;
				b_ConstructionEndSnapped = snapInformation.snapped;

				m_EndSnappedJunction = snapInformation.junction;
				m_EndSnappedEnd = snapInformation.end;
				m_EndSnappedRoadSegment = snapInformation.roadSegment;

				m_EndSnappedRoadSegmentT = snapInformation.T;
			}

			m_ConstructionPositions[1] = prevLocation;
			m_ConstructionPositions[2] = prevLocation;
			m_ConstructionPositions[3] = prevLocation;

			bool angleIsRestricted = false; // TODO: After angle snapping
			if (restrictions[0])
			{
				glm::vec3 directionNewRoadSegment = prevLocation - m_ConstructionPositions[0];
				directionNewRoadSegment.y = 0;
				directionNewRoadSegment = glm::normalize(directionNewRoadSegment);

				if (m_StartSnappedJunction)
				{
					for (RoadSegment* roadSegment : m_StartSnappedJunction->connectedRoadSegments)
					{
						glm::vec3 directionOldRoadSegment = roadSegment->ConnectedObjectAtStart.junction == m_StartSnappedJunction ? roadSegment->GetStartDirection() : roadSegment->GetEndDirection();
						directionOldRoadSegment.y = 0;
						directionOldRoadSegment = glm::normalize(directionOldRoadSegment);

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

					float angle = glm::acos(glm::dot(directionOldRoadSegment, directionNewRoadSegment));

					angleIsRestricted = angle < 0.5f;
				}
				else if (m_StartSnappedRoadSegment)
				{
					glm::vec3 tangent = Math::CubicCurveTangent(m_StartSnappedRoadSegment->GetCurvePoints(), m_StartSnappedRoadSegmentT);
					tangent = glm::normalize(tangent);

					float angle = glm::acos(glm::dot(tangent, directionNewRoadSegment));
					angleIsRestricted = angle < 0.5f || angle > 2.63f;
				}

				directionNewRoadSegment *= -1;
				if (m_EndSnappedJunction)
				{
					for (RoadSegment* roadSegment : m_EndSnappedJunction->connectedRoadSegments)
					{
						glm::vec3 directionOldRoadSegment = roadSegment->ConnectedObjectAtStart.junction == m_EndSnappedJunction ? roadSegment->GetStartDirection() : roadSegment->GetEndDirection();
						directionOldRoadSegment.y = 0;
						directionOldRoadSegment = glm::normalize(directionOldRoadSegment);

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

					float angle = glm::acos(glm::dot(directionOldRoadSegment, directionNewRoadSegment));

					angleIsRestricted |= angle < 0.5f;
				}
				else if (m_EndSnappedRoadSegment)
				{
					glm::vec3 tangent = Math::CubicCurveTangent(m_EndSnappedRoadSegment->GetCurvePoints(), m_StartSnappedRoadSegmentT);
					tangent = glm::normalize(tangent);

					float angle = glm::acos(glm::dot(tangent, directionNewRoadSegment));

					angleIsRestricted |= angle < 0.5f || angle > 2.63f;
				}
			}

			glm::vec3 AB = m_ConstructionPositions[3] - m_ConstructionPositions[0];

			float rotationOffset = (float)(AB.x < 0.0f) * glm::radians(180.0f);
			float rotationEnd = glm::atan(-AB.z / AB.x) + rotationOffset;
			float rotationStart = rotationEnd + glm::radians(180.0f);


			if (!b_ConstructionEndSnapped)
			{
				if (snapOptions[1] && glm::length(AB) > 0.5f)
				{
					float length = glm::length(AB);
					length = length - std::fmod(length, roadPrefabLength);
					AB = length * glm::normalize(AB);
					m_ConstructionPositions[1] = m_ConstructionPositions[0] + AB;
					m_ConstructionPositions[2] = m_ConstructionPositions[1];
					m_ConstructionPositions[3] = m_ConstructionPositions[1];
				}
				if (snapOptions[2])
				{
					m_ConstructionPositions[1].y = m_ConstructionPositions[0].y;
					m_ConstructionPositions[2].y = m_ConstructionPositions[0].y;
					m_ConstructionPositions[3].y = m_ConstructionPositions[0].y;
					AB.y = 0.0f;
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

						AB = glm::rotateY(AB, glm::radians(angle - newAngle));
						m_ConstructionPositions[3] = m_ConstructionPositions[0] + AB;
					}
					else if (m_StartSnappedRoadSegment)
					{
						glm::vec3 point = Math::CubicCurveTangent(m_StartSnappedRoadSegment->GetCurvePoints(), m_StartSnappedRoadSegmentT);
						float snappedRoadRotationY = glm::degrees(glm::atan(-point.z / point.x));
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
						if (Input::IsMouseButtonPressed(MouseCode::Button2))
							std::cout << "\nOld road angle: " << snappedRoadRotationY << ",\nNew road angle: " << newRoadRotationY << ",\nOld angle: " << angle << ",\nNew angle: " << newAngle << std::endl;

						AB = glm::rotateY(AB, glm::radians(angle - newAngle));
						m_ConstructionPositions[3] = m_ConstructionPositions[0] + AB;
					}
					else if (m_StartSnappedJunction)
					{
						float newRoadRotationY = glm::degrees(rotationEnd);
						float smallestAngle = 180.0f;
						for (RoadSegment* roadSegment : m_StartSnappedJunction->connectedRoadSegments)
						{
							float snappedRoadRotationY = glm::degrees(roadSegment->ConnectedObjectAtStart.junction == m_StartSnappedJunction ? roadSegment->GetStartRotation().y : roadSegment->GetEndRotation().y);
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

						AB = glm::rotateY(AB, glm::radians(smallestAngle - newAngle));
						m_ConstructionPositions[3] = m_ConstructionPositions[0] + AB;
					}
					else if (Input::IsKeyPressed(KeyCode::LeftControl))
					{
						float angle = std::fmod(glm::degrees(rotationEnd) + 720.0f, 360.0f);
						float newAngle = angle + 2.5f - std::fmod(angle + 2.5f, 5.0f);

						AB = glm::rotateY(AB, -glm::radians(angle - newAngle));
						m_ConstructionPositions[3] = m_ConstructionPositions[0] + AB;
					}
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

			bool lengthIsRestricted = restrictions[1] && glm::length(AD) < 2.0f * roadPrefabLength;
			bool collisionIsRestricted = restrictions[2] ? CheckStraightRoadRoadCollision(newRoadPolygon) : false;

			if (m_Scene->m_BuildingManager.restrictions[0] && restrictions[2])
				CheckStraightRoadBuildingCollision(newRoadPolygon);
			if (m_Scene->m_TreeManager.restrictions[0] && restrictions[2])
				CheckStraightRoadTreeCollision(newRoadPolygon);

			b_ConstructionRestricted |= angleIsRestricted;
			b_ConstructionRestricted |= lengthIsRestricted;
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
			if (m_EndSnappedRoadSegment != nullptr)
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
	void RoadManager::OnUpdate_QuadraticCurve(glm::vec3& prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
	{
		Prefab* selectedRoad = m_Scene->MainApplication->roads[m_Type][0];
		float roadPrefabWidth = selectedRoad->boundingBoxM.z - selectedRoad->boundingBoxL.z;
		float roadPrefabLength = selectedRoad->boundingBoxM.x - selectedRoad->boundingBoxL.x;

		for (std::vector<Object*>& os : m_Guidelines)
			for (Object* rsg : os)
				rsg->enabled = false;

		for (size_t& inUse : m_GuidelinesInUse)
			inUse = 0;

		if (m_ConstructionPhase == 0)
		{
			if (snapOptions[4])
			{
				prevLocation.x = prevLocation.x - std::fmod(prevLocation.x, 0.5f) + 0.25f;
				prevLocation.z = prevLocation.z - std::fmod(prevLocation.z, 0.5f) - 0.25f;
			}
			if (snapOptions[0])
			{
				SnapInformation snapInformation = CheckSnapping(prevLocation);
				prevLocation = snapInformation.location;
				b_ConstructionStartSnapped = snapInformation.snapped;

				m_StartSnappedJunction = snapInformation.junction;
				m_StartSnappedEnd = snapInformation.end;
				m_StartSnappedRoadSegment = snapInformation.roadSegment;

				m_StartSnappedRoadSegmentT = snapInformation.T;
			}
			m_ConstructionPositions[0] = prevLocation;

			m_GuidelinesStart->SetTransform(prevLocation + glm::vec3{ 0.0f, 0.15f, 0.0f }, glm::vec3(1.0f), { 0.0f, glm::radians(180.0f), 0.0f });
			m_GuidelinesEnd->SetTransform(prevLocation + glm::vec3{ 0.0f, 0.15f, 0.0f }, glm::vec3(1.0f), glm::vec3(0.0f));
		}
		else if (m_ConstructionPhase == 1)
		{
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
				glm::vec3 directionNewRoadSegment = prevLocation - m_ConstructionPositions[0];
				directionNewRoadSegment.y = 0;
				directionNewRoadSegment = glm::normalize(directionNewRoadSegment);

				if (m_StartSnappedJunction)
				{
					for (RoadSegment* roadSegment : m_StartSnappedJunction->connectedRoadSegments)
					{
						glm::vec3 directionOldRoadSegment = roadSegment->ConnectedObjectAtStart.junction == m_StartSnappedJunction ? roadSegment->GetStartDirection() : roadSegment->GetEndDirection();
						directionOldRoadSegment.y = 0;
						directionOldRoadSegment = glm::normalize(directionOldRoadSegment);

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

					float angle = glm::acos(glm::dot(directionOldRoadSegment, directionNewRoadSegment));

					angleIsRestricted = angle < 0.5f;
				}
				else if (m_StartSnappedRoadSegment)
				{
					glm::vec3 tangent = Math::CubicCurveTangent(m_StartSnappedRoadSegment->GetCurvePoints(), m_StartSnappedRoadSegmentT);
					tangent = glm::normalize(tangent);

					float angle = glm::acos(glm::dot(tangent, directionNewRoadSegment));
					angleIsRestricted = angle < 0.5f || angle > 2.63f;
				}
			}

			glm::vec3 AB = m_ConstructionPositions[3] - m_ConstructionPositions[0];

			float rotationOffset = (float)(AB.x < 0.0f) * glm::radians(180.0f);
			float rotationEnd = glm::atan(-AB.z / AB.x) + rotationOffset;
			float rotationStart = rotationEnd + glm::radians(180.0f);

			if (snapOptions[2])
			{
				m_ConstructionPositions[1].y = m_ConstructionPositions[0].y;
				m_ConstructionPositions[2].y = m_ConstructionPositions[0].y;
				m_ConstructionPositions[3].y = m_ConstructionPositions[0].y;
				AB.y = 0.0f;
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

					AB = glm::rotateY(AB, glm::radians(angle - newAngle));
					m_ConstructionPositions[1] = m_ConstructionPositions[0] + AB;
					m_ConstructionPositions[2] = m_ConstructionPositions[1];
					m_ConstructionPositions[3] = m_ConstructionPositions[1];
				}
				else if (m_StartSnappedRoadSegment)
				{
					glm::vec3 point = Math::CubicCurveTangent(m_StartSnappedRoadSegment->GetCurvePoints(), m_StartSnappedRoadSegmentT);
					float snappedRoadRotationY = glm::degrees(glm::atan(-point.z / point.x));
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

					AB = glm::rotateY(AB, glm::radians(angle - newAngle));
					m_ConstructionPositions[1] = m_ConstructionPositions[0] + AB;
					m_ConstructionPositions[2] = m_ConstructionPositions[1];
					m_ConstructionPositions[3] = m_ConstructionPositions[1];
				}
				else if (m_StartSnappedJunction)
				{
					float newRoadRotationY = glm::degrees(rotationEnd);
					float smallestAngle = 180.0f;
					for (RoadSegment* roadSegment : m_StartSnappedJunction->connectedRoadSegments)
					{
						float snappedRoadRotationY = glm::degrees(roadSegment->ConnectedObjectAtStart.junction == m_StartSnappedJunction ? roadSegment->GetStartRotation().y : roadSegment->GetEndRotation().y);
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

					AB = glm::rotateY(AB, glm::radians(smallestAngle - newAngle));
					m_ConstructionPositions[1] = m_ConstructionPositions[0] + AB;
					m_ConstructionPositions[2] = m_ConstructionPositions[1];
					m_ConstructionPositions[3] = m_ConstructionPositions[1];
				}
				else if (Input::IsKeyPressed(KeyCode::LeftControl))
				{
					float angle = std::fmod(glm::degrees(rotationEnd) + 720.0f, 360.0f);
					float newAngle = angle + 2.5f - std::fmod(angle + 2.5f, 5.0f);

					AB = glm::rotateY(AB, -glm::radians(angle - newAngle));
					m_ConstructionPositions[1] = m_ConstructionPositions[0] + AB;
					m_ConstructionPositions[2] = m_ConstructionPositions[1];
					m_ConstructionPositions[3] = m_ConstructionPositions[1];
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

			bool collisionIsRestricted = restrictions[2] ? CheckStraightRoadRoadCollision(newRoadPolygon) : false; // Just for visual
			if (m_Scene->m_BuildingManager.restrictions[0] && restrictions[2])
				CheckStraightRoadBuildingCollision(newRoadPolygon);
			if (m_Scene->m_TreeManager.restrictions[0] && restrictions[2])
				CheckStraightRoadTreeCollision(newRoadPolygon);

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
			if (snapOptions[4])
			{
				prevLocation.x = prevLocation.x - std::fmod(prevLocation.x + 0.25f, 0.5f) + 0.25f;
				prevLocation.z = prevLocation.z - std::fmod(prevLocation.z + 0.25f, 0.5f) + 0.25f;
			}
			if (snapOptions[0])
			{
				SnapInformation snapInformation = CheckSnapping(prevLocation);
				prevLocation = snapInformation.location;
				b_ConstructionEndSnapped = snapInformation.snapped;

				m_EndSnappedJunction = snapInformation.junction;
				m_EndSnappedEnd = snapInformation.end;
				m_EndSnappedRoadSegment = snapInformation.roadSegment;

				m_EndSnappedRoadSegmentT = snapInformation.T;
			}

			b_ConstructionRestricted = false;
			m_ConstructionPositions[3] = prevLocation;

			if (!b_ConstructionEndSnapped && snapOptions[2])
				m_ConstructionPositions[3].y = m_ConstructionPositions[0].y;

			/***Magic***/ {
				glm::vec2 Cd{ m_ConstructionPositions[1].x, m_ConstructionPositions[1].z };
				glm::vec2 A{ m_ConstructionPositions[0].x, m_ConstructionPositions[0].z };
				glm::vec2 B{ m_ConstructionPositions[3].x, m_ConstructionPositions[3].z };
				glm::vec2 ray = glm::normalize(Cd - A);
				glm::vec2 AB = B - A;
				float d = glm::dot(AB, AB) / (2.0f * glm::dot(AB, ray));
				glm::vec2 C = A + d * ray;
				if (d < 200.0f && d > 0.0f)
				{
					m_ConstructionPositions[2].x = C.x;
					m_ConstructionPositions[2].z = C.y;
				}
			}
			bool angleIsRestricted = false; // also check angle of new road's curve
			if (restrictions[0])
			{
				glm::vec3 directionNewRoadSegment = prevLocation - m_ConstructionPositions[0];
				directionNewRoadSegment.y = 0;
				directionNewRoadSegment = glm::normalize(directionNewRoadSegment);

				if (m_StartSnappedJunction)
				{
					for (RoadSegment* roadSegment : m_StartSnappedJunction->connectedRoadSegments)
					{
						glm::vec3 directionOldRoadSegment = roadSegment->ConnectedObjectAtStart.junction == m_StartSnappedJunction ? roadSegment->GetStartDirection() : roadSegment->GetEndDirection();
						directionOldRoadSegment.y = 0;
						directionOldRoadSegment = glm::normalize(directionOldRoadSegment);

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

					float angle = glm::acos(glm::dot(directionOldRoadSegment, directionNewRoadSegment));

					angleIsRestricted = angle < 0.5f;
				}
				else if (m_StartSnappedRoadSegment)
				{
					glm::vec3 tangent = Math::CubicCurveTangent(m_StartSnappedRoadSegment->GetCurvePoints(), m_StartSnappedRoadSegmentT);
					tangent = glm::normalize(tangent);

					float angle = glm::acos(glm::dot(tangent, directionNewRoadSegment));
					angleIsRestricted = angle < 0.5f || angle > 2.63f;
				}

				directionNewRoadSegment = m_ConstructionPositions[2] - prevLocation;
				directionNewRoadSegment.y = 0;
				directionNewRoadSegment = glm::normalize(directionNewRoadSegment);
				if (m_EndSnappedJunction)
				{
					for (RoadSegment* roadSegment : m_EndSnappedJunction->connectedRoadSegments)
					{
						glm::vec3 directionOldRoadSegment = roadSegment->ConnectedObjectAtStart.junction == m_EndSnappedJunction ? roadSegment->GetStartDirection() : roadSegment->GetEndDirection();
						directionOldRoadSegment.y = 0;
						directionOldRoadSegment = glm::normalize(directionOldRoadSegment);

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

					float angle = glm::acos(glm::dot(directionOldRoadSegment, directionNewRoadSegment));

					angleIsRestricted |= angle < 0.5f;
				}
				else if (m_EndSnappedRoadSegment)
				{
					glm::vec3 tangent = Math::CubicCurveTangent(m_EndSnappedRoadSegment->GetCurvePoints(), m_EndSnappedRoadSegmentT);
					tangent = glm::normalize(tangent);

					float angle = glm::acos(glm::dot(tangent, directionNewRoadSegment));

					angleIsRestricted |= angle < 0.5f || angle > 2.63f;
				}
			}

			std::array<glm::vec3, 4> cps = {
					m_ConstructionPositions[0],
						(m_ConstructionPositions[2] + m_ConstructionPositions[0]) * 0.5f,
						(m_ConstructionPositions[2] + m_ConstructionPositions[3]) * 0.5f,
						m_ConstructionPositions[3],
			};
			std::array<std::array<glm::vec2, 3>, 2> newRoadBoundingBox = Math::GetBoundingBoxOfBezierCurve(cps, roadPrefabWidth * 0.5f);
			std::array<std::array<glm::vec2, 3>, (10 - 1) * 2> newRoadBoundingPolygon = Math::GetBoundingPolygonOfBezierCurve<10, 10>(cps, roadPrefabWidth * 0.5f);

			bool lengthIsRestricted = restrictions[1] && glm::length(cps[0] - cps[1]) < 2.0f * roadPrefabLength;
			bool collisionIsRestricted = restrictions[2] ? CheckRoadRoadCollision(newRoadBoundingBox, newRoadBoundingPolygon) : false;

			if (m_Scene->m_BuildingManager.restrictions[0] && restrictions[2])
				CheckRoadBuildingCollision(newRoadBoundingBox, newRoadBoundingPolygon);
			if (m_Scene->m_TreeManager.restrictions[0] && restrictions[2])
				CheckRoadTreeCollision(newRoadBoundingBox, newRoadBoundingPolygon);

			b_ConstructionRestricted |= angleIsRestricted;
			b_ConstructionRestricted |= lengthIsRestricted;
			b_ConstructionRestricted |= collisionIsRestricted;
			DrawCurvedGuidelines(cps);

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
			if (m_EndSnappedRoadSegment != nullptr)
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
	void RoadManager::OnUpdate_CubicCurve(glm::vec3& prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
	{
		Prefab* selectedRoad = m_Scene->MainApplication->roads[m_Type][0];
		float roadPrefabWidth = selectedRoad->boundingBoxM.z - selectedRoad->boundingBoxL.z;
		float roadPrefabLength = selectedRoad->boundingBoxM.x - selectedRoad->boundingBoxL.x;

		for (std::vector<Object*>& os : m_Guidelines)
			for (Object* rg : os)
				rg->enabled = false;

		for (size_t& inUse : m_GuidelinesInUse)
			inUse = 0;

		if (m_ConstructionPhase == 0)
		{
			if (snapOptions[4])
			{
				prevLocation.x = prevLocation.x - std::fmod(prevLocation.x, 0.5f) + 0.25f;
				prevLocation.z = prevLocation.z - std::fmod(prevLocation.z, 0.5f) - 0.25f;
			}
			if (snapOptions[0])
			{
				SnapInformation snapInformation = CheckSnapping(prevLocation);
				prevLocation = snapInformation.snapped ? snapInformation.location : prevLocation;
				b_ConstructionStartSnapped = snapInformation.snapped;
				m_StartSnappedJunction = snapInformation.junction;
				m_StartSnappedEnd = snapInformation.end;
				m_StartSnappedRoadSegment = snapInformation.roadSegment;

				m_StartSnappedRoadSegmentT = snapInformation.T;
			}

			m_ConstructionPositions[0] = prevLocation;
			m_ConstructionPositions[1] = prevLocation;
			m_ConstructionPositions[2] = prevLocation;
			m_ConstructionPositions[3] = prevLocation;

			m_GuidelinesStart->SetTransform(prevLocation + glm::vec3{ 0.0f, 0.15f, 0.0f }, glm::vec3(1.0f), glm::vec3{ 0.0f, glm::radians(180.0f), 0.0f });
			m_GuidelinesEnd->SetTransform(prevLocation + glm::vec3{ 0.0f, 0.15f, 0.0f }, glm::vec3(1.0f), glm::vec3(0.0f));
		}
		else if (m_ConstructionPhase == 1)
		{
			b_ConstructionRestricted = false;
			if (snapOptions[4])
			{
				prevLocation.x = prevLocation.x - std::fmod(prevLocation.x, 0.5f) + 0.25f;
				prevLocation.z = prevLocation.z - std::fmod(prevLocation.z, 0.5f) - 0.25f;
			}
			if (snapOptions[0] && cubicCurveOrder[1] == 3)
			{
				SnapInformation snapInformation = CheckSnapping(prevLocation);
				prevLocation = snapInformation.location;
				b_ConstructionEndSnapped = snapInformation.snapped;

				m_EndSnappedEnd = snapInformation.end;
				m_EndSnappedJunction = snapInformation.junction;
				m_EndSnappedRoadSegment = snapInformation.roadSegment;

				m_EndSnappedRoadSegmentT = snapInformation.T;
			}

			m_ConstructionPositions[1] = prevLocation;
			m_ConstructionPositions[2] = prevLocation;
			m_ConstructionPositions[3] = prevLocation;

			bool angleIsRestricted = false;
			if (cubicCurveOrder[1] == 1 && restrictions[0])
			{
				glm::vec3 directionNewRoadSegment = prevLocation - m_ConstructionPositions[0];
				directionNewRoadSegment.y = 0;
				directionNewRoadSegment = glm::normalize(directionNewRoadSegment);

				if (m_StartSnappedJunction)
				{
					for (RoadSegment* roadSegment : m_StartSnappedJunction->connectedRoadSegments)
					{
						glm::vec3 directionOldRoadSegment = roadSegment->ConnectedObjectAtStart.junction == m_StartSnappedJunction ? roadSegment->GetStartDirection() : roadSegment->GetEndDirection();
						directionOldRoadSegment.y = 0;
						directionOldRoadSegment = glm::normalize(directionOldRoadSegment);

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

					float angle = glm::acos(glm::dot(directionOldRoadSegment, directionNewRoadSegment));

					angleIsRestricted = angle < 0.5f;
				}
				else if (m_StartSnappedRoadSegment)
				{
					glm::vec3 tangent = Math::CubicCurveTangent(m_StartSnappedRoadSegment->GetCurvePoints(), m_StartSnappedRoadSegmentT);
					tangent = glm::normalize(tangent);

					float angle = glm::acos(glm::dot(tangent, directionNewRoadSegment));

					angleIsRestricted = angle < 0.5f || angle > 2.63f;
				}
			}

			glm::vec3 AB = prevLocation - m_ConstructionPositions[0];

			float rotationOffset = (float)(AB.x < 0.0f) * glm::radians(180.0f);
			float rotationEnd = glm::atan(-AB.z / AB.x) + rotationOffset;
			float rotationStart = rotationEnd + glm::radians(180.0f);

			if (snapOptions[1] && cubicCurveOrder[1] == 1 && glm::length(AB) > 0.5f)
			{
				float length = glm::length(AB);
				length = length - std::fmod(length, roadPrefabLength);
				AB = length * glm::normalize(AB);

				m_ConstructionPositions[1] = m_ConstructionPositions[0] + AB;
				m_ConstructionPositions[2] = m_ConstructionPositions[1];
				m_ConstructionPositions[3] = m_ConstructionPositions[1];
			}
			if (snapOptions[2] && !b_ConstructionEndSnapped)
			{
				m_ConstructionPositions[1].y = m_ConstructionPositions[0].y;
				m_ConstructionPositions[2].y = m_ConstructionPositions[0].y;
				m_ConstructionPositions[3].y = m_ConstructionPositions[0].y;
				AB.y = 0.0f;
			}
			if (snapOptions[3] && cubicCurveOrder[1] == 1 && glm::length(AB) > 0.5f)
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

					AB = glm::rotateY(AB, glm::radians(angle - newAngle));
					m_ConstructionPositions[1] = m_ConstructionPositions[0] + AB;
					m_ConstructionPositions[2] = m_ConstructionPositions[1];
					m_ConstructionPositions[3] = m_ConstructionPositions[1];
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

					AB = glm::rotateY(AB, glm::radians(angle - newAngle));
					m_ConstructionPositions[1] = m_ConstructionPositions[0] + AB;
					m_ConstructionPositions[2] = m_ConstructionPositions[1];
					m_ConstructionPositions[3] = m_ConstructionPositions[1];
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

					AB = glm::rotateY(AB, glm::radians(smallestAngle - newAngle));
					m_ConstructionPositions[1] = m_ConstructionPositions[0] + AB;
					m_ConstructionPositions[2] = m_ConstructionPositions[1];
					m_ConstructionPositions[3] = m_ConstructionPositions[1];
				}
				else if (Input::IsKeyPressed(KeyCode::LeftControl))
				{
					float angle = std::fmod(glm::degrees(rotationEnd) + 720.0f, 360.0f);
					float newAngle = angle + 2.5f - std::fmod(angle + 2.5f, 5.0f);

					AB = glm::rotateY(AB, -glm::radians(angle - newAngle));
					m_ConstructionPositions[1] = m_ConstructionPositions[0] + AB;
					m_ConstructionPositions[2] = m_ConstructionPositions[1];
					m_ConstructionPositions[3] = m_ConstructionPositions[1];
				}
			}

			glm::vec2 A = glm::vec2{ m_ConstructionPositions[cubicCurveOrder[0]].x, m_ConstructionPositions[cubicCurveOrder[0]].z };
			glm::vec2 D = glm::vec2{ m_ConstructionPositions[cubicCurveOrder[1]].x, m_ConstructionPositions[cubicCurveOrder[1]].z };
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

			bool lengthIsRestricted = restrictions[1] && glm::length(AD) < 2.0f * roadPrefabLength;
			bool collisionIsRestricted = restrictions[2] ? CheckStraightRoadRoadCollision(newRoadPolygon) : false;

			if (m_Scene->m_BuildingManager.restrictions[0] && restrictions[2])
				CheckStraightRoadBuildingCollision(newRoadPolygon);
			if (m_Scene->m_TreeManager.restrictions[0] && restrictions[2])
				CheckStraightRoadTreeCollision(newRoadPolygon);

			b_ConstructionRestricted |= angleIsRestricted;
			b_ConstructionRestricted |= lengthIsRestricted;
			b_ConstructionRestricted |= collisionIsRestricted;
			DrawStraightGuidelines(m_ConstructionPositions[0], m_ConstructionPositions[3]);

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
		}
		else if (m_ConstructionPhase == 2)
		{
		if (Input::IsKeyPressed(KeyCode::Y))
			std::cout << std::endl;
			b_ConstructionRestricted = false;
			if (snapOptions[4])
			{
				prevLocation.x = prevLocation.x - std::fmod(prevLocation.x, 0.5f) + 0.25f;
				prevLocation.z = prevLocation.z - std::fmod(prevLocation.z, 0.5f) - 0.25f;
			}
			if (cubicCurveOrder[2] == 3 && snapOptions[0])
			{
				SnapInformation snapInformation = CheckSnapping(prevLocation);
				prevLocation = snapInformation.location;
				b_ConstructionEndSnapped = snapInformation.snapped;

				m_EndSnappedEnd = snapInformation.end;
				m_EndSnappedJunction = snapInformation.junction;
				m_EndSnappedRoadSegment = snapInformation.roadSegment;

				m_EndSnappedRoadSegmentT = snapInformation.T;
			}

			m_ConstructionPositions[cubicCurveOrder[2]] = prevLocation;
			m_ConstructionPositions[cubicCurveOrder[3]] = prevLocation;

			bool angleIsRestricted = false;
			if (restrictions[0]) // also check extra things??
			{
				if (cubicCurveOrder[3] != 1)
				{
					glm::vec3 directionNewRoadSegment = m_ConstructionPositions[1] - m_ConstructionPositions[0];
					directionNewRoadSegment.y = 0;
					directionNewRoadSegment = glm::normalize(directionNewRoadSegment);

					if (m_StartSnappedJunction)
					{
						for (RoadSegment* roadSegment : m_StartSnappedJunction->connectedRoadSegments)
						{
							glm::vec3 directionOldRoadSegment = roadSegment->ConnectedObjectAtStart.junction == m_StartSnappedJunction ? roadSegment->GetStartDirection() : roadSegment->GetEndDirection();
							directionOldRoadSegment.y = 0;
							directionOldRoadSegment = glm::normalize(directionOldRoadSegment);

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

						float angle = glm::acos(glm::dot(directionOldRoadSegment, directionNewRoadSegment));

						angleIsRestricted = angle < 0.5f;
					}
					else if (m_StartSnappedRoadSegment)
					{
						glm::vec3 tangent = Math::CubicCurveTangent(m_StartSnappedRoadSegment->GetCurvePoints(), m_StartSnappedRoadSegmentT);
						tangent = glm::normalize(tangent);

						float angle = glm::acos(glm::dot(tangent, directionNewRoadSegment));

						angleIsRestricted = angle < 0.5f || angle > 2.63f;
					}
				}
				else
				{
					glm::vec3 directionNewRoadSegment = m_ConstructionPositions[2] - m_ConstructionPositions[3];
					directionNewRoadSegment.y = 0;
					directionNewRoadSegment = glm::normalize(directionNewRoadSegment);
					if (m_EndSnappedJunction)
					{
						for (RoadSegment* roadSegment : m_EndSnappedJunction->connectedRoadSegments)
						{
							glm::vec3 directionOldRoadSegment = roadSegment->ConnectedObjectAtStart.junction == m_EndSnappedJunction ? roadSegment->GetStartDirection() : roadSegment->GetEndDirection();
							directionOldRoadSegment.y = 0;
							directionOldRoadSegment = glm::normalize(directionOldRoadSegment);

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

						float angle = glm::acos(glm::dot(directionOldRoadSegment, directionNewRoadSegment));

						angleIsRestricted |= angle < 0.5f;
					}
					else if (m_EndSnappedRoadSegment)
					{
						glm::vec3 tangent = Math::CubicCurveTangent(m_StartSnappedRoadSegment->GetCurvePoints(), m_StartSnappedRoadSegmentT);
						tangent = glm::normalize(tangent);

						float angle = glm::acos(glm::dot(tangent, directionNewRoadSegment));

						angleIsRestricted |= angle < 0.5f || angle > 2.63f;
					}
				}
			}

			if (snapOptions[1])
			{
				glm::vec3 AB1 = m_ConstructionPositions[1] - m_ConstructionPositions[0];
				glm::vec3 AB2 = m_ConstructionPositions[cubicCurveOrder[2]] - m_ConstructionPositions[cubicCurveOrder[1]];

				float length1 = glm::length(AB1);
				float length2 = glm::length(AB2);

				if (cubicCurveOrder[2] == 1 && length1 > 0.1f)
				{
					length1 = length1 - std::fmod(length1, roadPrefabLength);
					AB1 = length1 * glm::normalize(AB1);

					m_ConstructionPositions[1] = m_ConstructionPositions[0] + AB1;
				}
				else if (cubicCurveOrder[3] == 1 && length2 > 0.1f)
				{
					length2 = length2 - std::fmod(length2, roadPrefabLength);
					AB2 = length2 * glm::normalize(AB2);

					m_ConstructionPositions[cubicCurveOrder[2]] = m_ConstructionPositions[cubicCurveOrder[1]] + AB2;
				}
			}
			if (snapOptions[2] && !b_ConstructionEndSnapped)
			{
				m_ConstructionPositions[cubicCurveOrder[2]].y = m_ConstructionPositions[0].y;
				m_ConstructionPositions[cubicCurveOrder[3]].y = m_ConstructionPositions[0].y;
			}
			if (snapOptions[3])
			{
				glm::vec3 AB1 = m_ConstructionPositions[1] - m_ConstructionPositions[0];
				float rotation1 = glm::atan(-AB1.z / AB1.x) + (float)(AB1.x < 0.0f) * glm::radians(180.0f);
				float length1 = glm::length(AB1);

				glm::vec3 AB2 = m_ConstructionPositions[cubicCurveOrder[2]] - m_ConstructionPositions[cubicCurveOrder[1]];
				float rotation2 = glm::atan(-AB2.z / AB2.x) + (float)(AB2.x < 0.0f) * glm::radians(180.0f);
				float length2 = glm::length(AB2);

				if (cubicCurveOrder[2] == 1 && length1 > 0.1f)
				{
					if (m_StartSnappedEnd)
					{
						float snappedRoadRotationY = glm::degrees(m_StartSnappedEnd->object->rotation.y) + 180.0f;
						float newRoadRotationY = glm::degrees(rotation1);
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

						AB1 = glm::rotateY(AB1, glm::radians(angle - newAngle));
						m_ConstructionPositions[1] = m_ConstructionPositions[0] + AB1;
					}
					else if (m_StartSnappedRoadSegment)
					{
						float snappedRoadRotationY = glm::degrees(m_StartSnappedRoadSegment->GetStartRotation().y);
						float newRoadRotationY = glm::degrees(rotation1);
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

						AB1 = glm::rotateY(AB1, glm::radians(angle - newAngle));
						m_ConstructionPositions[1] = m_ConstructionPositions[0] + AB1;
					}
					else if (m_StartSnappedJunction)
					{
						float newRoadRotationY = glm::degrees(rotation1);
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

						AB1 = glm::rotateY(AB1, glm::radians(smallestAngle - newAngle));
						m_ConstructionPositions[1] = m_ConstructionPositions[0] + AB1;
					}
					else if (Input::IsKeyPressed(KeyCode::LeftControl))
					{
						float angle = std::fmod(glm::degrees(rotation1) + 720.0f, 360.0f);
						float newAngle = angle + 2.5f - std::fmod(angle + 2.5f, 5.0f);

						AB1 = glm::rotateY(AB1, -glm::radians(angle - newAngle));
						m_ConstructionPositions[1] = m_ConstructionPositions[0] + AB1;
					}
				}
				else if (cubicCurveOrder[3] == 1 && length2 > 0.1f)
				{
					if (m_StartSnappedEnd)
					{
						float snappedRoadRotationY = glm::degrees(m_StartSnappedEnd->object->rotation.y) + 180.0f;
						float newRoadRotationY = glm::degrees(rotation2);
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

						AB2 = glm::rotateY(AB2, glm::radians(angle - newAngle));
						m_ConstructionPositions[cubicCurveOrder[2]] = m_ConstructionPositions[cubicCurveOrder[1]] + AB2;
					}
					else if (m_StartSnappedRoadSegment)
					{
						float snappedRoadRotationY = glm::degrees(m_StartSnappedRoadSegment->GetStartRotation().y);
						float newRoadRotationY = glm::degrees(rotation2);
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

						AB2 = glm::rotateY(AB2, glm::radians(angle - newAngle));
						m_ConstructionPositions[cubicCurveOrder[2]] = m_ConstructionPositions[cubicCurveOrder[1]] + AB2;
					}
					else if (m_StartSnappedJunction)
					{
						float newRoadRotationY = glm::degrees(rotation2);
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

						AB2 = glm::rotateY(AB2, glm::radians(smallestAngle - newAngle));
						m_ConstructionPositions[cubicCurveOrder[2]] = m_ConstructionPositions[cubicCurveOrder[1]] + AB2;
					}
					else if (Input::IsKeyPressed(KeyCode::LeftControl))
					{
						float angle = std::fmod(glm::degrees(rotation2) + 720.0f, 360.0f);
						float newAngle = angle + 2.5f - std::fmod(angle + 2.5f, 5.0f);

						AB2 = glm::rotateY(AB2, -glm::radians(angle - newAngle));
						m_ConstructionPositions[cubicCurveOrder[2]] = m_ConstructionPositions[cubicCurveOrder[1]] + AB2;
					}
				}
			}

			// needs some attention
			std::array<glm::vec3, 4> cps{
				m_ConstructionPositions[0],
				(m_ConstructionPositions[(cubicCurveOrder[3] == 1) ? 2 : 1] + m_ConstructionPositions[0]) * 0.5f,
				(m_ConstructionPositions[(cubicCurveOrder[3] == 1) ? 2 : 1] + m_ConstructionPositions[3]) * 0.5f,
				m_ConstructionPositions[3]
			};

			std::array<std::array<glm::vec2, 3>, 2> newRoadBoundingBox = Math::GetBoundingBoxOfBezierCurve(cps, roadPrefabWidth * 0.5f);
			std::array<std::array<glm::vec2, 3>, (10 - 1) * 2> newRoadBoundingPolygon = Math::GetBoundingPolygonOfBezierCurve<10, 10>(cps, roadPrefabWidth * 0.5f);

			bool lengthIsRestricted = restrictions[1] && (glm::length(cps[0] - cps[1]) < 2.0f * roadPrefabLength);
			bool collisionIsRestricted = restrictions[2] ? CheckRoadRoadCollision(newRoadBoundingBox, newRoadBoundingPolygon) : false;

			if (m_Scene->m_BuildingManager.restrictions[0] && restrictions[2])
				CheckRoadBuildingCollision(newRoadBoundingBox, newRoadBoundingPolygon);
			if (m_Scene->m_TreeManager.restrictions[0] && restrictions[2])
				CheckRoadTreeCollision(newRoadBoundingBox, newRoadBoundingPolygon);

			b_ConstructionRestricted |= angleIsRestricted;
			b_ConstructionRestricted |= lengthIsRestricted;
			b_ConstructionRestricted |= collisionIsRestricted;

			DrawCurvedGuidelines(cps);
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
		}
		else if (m_ConstructionPhase == 3)
		{
			b_ConstructionRestricted = false;
			if (snapOptions[4])
			{
				prevLocation.x = prevLocation.x - std::fmod(prevLocation.x, 0.5f) + 0.25f;
				prevLocation.z = prevLocation.z - std::fmod(prevLocation.z, 0.5f) - 0.25f;
			}
			if (cubicCurveOrder[3] == 3 && snapOptions[0])
			{
				SnapInformation snapInformation = CheckSnapping(prevLocation);
				prevLocation = snapInformation.location;
				b_ConstructionEndSnapped = snapInformation.snapped;

				m_EndSnappedEnd = snapInformation.end;
				m_EndSnappedJunction = snapInformation.junction;
				m_EndSnappedRoadSegment = snapInformation.roadSegment;

				m_EndSnappedRoadSegmentT = snapInformation.T;
			}

			m_ConstructionPositions[cubicCurveOrder[3]] = prevLocation;

			bool angleIsRestricted = false;
			if (restrictions[0]) // also check extra things??
			{
				glm::vec3 directionNewRoadSegment = m_ConstructionPositions[1] - m_ConstructionPositions[0];
				directionNewRoadSegment.y = 0;
				directionNewRoadSegment = glm::normalize(directionNewRoadSegment);

				if (m_StartSnappedJunction)
				{
					for (RoadSegment* roadSegment : m_StartSnappedJunction->connectedRoadSegments)
					{
						glm::vec3 directionOldRoadSegment = roadSegment->ConnectedObjectAtStart.junction == m_StartSnappedJunction ? roadSegment->GetStartDirection() : roadSegment->GetEndDirection();
						directionOldRoadSegment.y = 0;
						directionOldRoadSegment = glm::normalize(directionOldRoadSegment);

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

					float angle = glm::acos(glm::dot(directionOldRoadSegment, directionNewRoadSegment));

					angleIsRestricted = angle < 0.5f;
				}
				else if (m_StartSnappedRoadSegment)
				{
					glm::vec3 tangent = Math::CubicCurveTangent(m_StartSnappedRoadSegment->GetCurvePoints(), m_StartSnappedRoadSegmentT);
					tangent = glm::normalize(tangent);

					float angle = glm::acos(glm::dot(tangent, directionNewRoadSegment));

					angleIsRestricted = angle < 0.5f || angle > 2.63f;
				}

				directionNewRoadSegment = m_ConstructionPositions[2] - m_ConstructionPositions[3];
				directionNewRoadSegment.y = 0;
				directionNewRoadSegment = glm::normalize(directionNewRoadSegment);
				if (m_EndSnappedJunction)
				{
					for (RoadSegment* roadSegment : m_EndSnappedJunction->connectedRoadSegments)
					{
						glm::vec3 directionOldRoadSegment = roadSegment->ConnectedObjectAtStart.junction == m_EndSnappedJunction ? roadSegment->GetStartDirection() : roadSegment->GetEndDirection();
						directionOldRoadSegment.y = 0;
						directionOldRoadSegment = glm::normalize(directionOldRoadSegment);

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

					float angle = glm::acos(glm::dot(directionOldRoadSegment, directionNewRoadSegment));

					angleIsRestricted |= angle < 0.5f;
				}
				else if (m_EndSnappedRoadSegment)
				{
					glm::vec3 tangent = Math::CubicCurveTangent(m_StartSnappedRoadSegment->GetCurvePoints(), m_StartSnappedRoadSegmentT);
					tangent = glm::normalize(tangent);

					float angle = glm::acos(glm::dot(tangent, directionNewRoadSegment));

					angleIsRestricted |= angle < 0.5f || angle > 2.63f;
				}
			}

			if (snapOptions[1])
			{
				glm::vec3 AB1 = m_ConstructionPositions[1] - m_ConstructionPositions[0];
				glm::vec3 AB2 = m_ConstructionPositions[cubicCurveOrder[3]] - m_ConstructionPositions[cubicCurveOrder[2]];

				float length1 = glm::length(AB1);
				float length2 = glm::length(AB2);

				if (cubicCurveOrder[3] == 1 && length1 > 0.1f)
				{
					length1 = length1 - std::fmod(length1, roadPrefabLength);
					AB1 = length1 * glm::normalize(AB1);

					m_ConstructionPositions[1] = m_ConstructionPositions[0] + AB1;
				}
				else if (cubicCurveOrder[3] != 1 && length2 > 0.1f)
				{
					length2 = length2 - std::fmod(length2, roadPrefabLength);
					AB2 = length2 * glm::normalize(AB2);

					m_ConstructionPositions[cubicCurveOrder[3]] = m_ConstructionPositions[cubicCurveOrder[2]] + AB2;
				}
			}
			if (snapOptions[2] && !b_ConstructionEndSnapped)
			{
				m_ConstructionPositions[cubicCurveOrder[3]].y = m_ConstructionPositions[0].y;
			}
			if (snapOptions[3])
			{
				glm::vec3 AB1 = m_ConstructionPositions[1] - m_ConstructionPositions[0];
				float rotation1 = glm::atan(-AB1.z / AB1.x) + (float)(AB1.x < 0.0f) * glm::radians(180.0f);
				float length1 = glm::length(AB1);

				glm::vec3 AB2 = m_ConstructionPositions[cubicCurveOrder[3]] - m_ConstructionPositions[cubicCurveOrder[2]];
				float rotation2 = glm::atan(-AB2.z / AB2.x) + (float)(AB2.x < 0.0f) * glm::radians(180.0f);
				float length2 = glm::length(AB2);

				if (cubicCurveOrder[3] == 1 && length1 > 0.1f)
				{
					if (m_StartSnappedEnd)
					{
						float snappedRoadRotationY = glm::degrees(m_StartSnappedEnd->object->rotation.y) + 180.0f;
						float newRoadRotationY = glm::degrees(rotation1);
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

						AB1 = glm::rotateY(AB1, glm::radians(angle - newAngle));
						m_ConstructionPositions[1] = m_ConstructionPositions[0] + AB1;
					}
					else if (m_StartSnappedRoadSegment)
					{
						float snappedRoadRotationY = glm::degrees(m_StartSnappedRoadSegment->GetStartRotation().y);
						float newRoadRotationY = glm::degrees(rotation1);
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

						AB1 = glm::rotateY(AB1, glm::radians(angle - newAngle));
						m_ConstructionPositions[1] = m_ConstructionPositions[0] + AB1;
					}
					else if (m_StartSnappedJunction)
					{
						float newRoadRotationY = glm::degrees(rotation1);
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

						AB1 = glm::rotateY(AB1, glm::radians(smallestAngle - newAngle));
						m_ConstructionPositions[1] = m_ConstructionPositions[0] + AB1;
					}
					else if (Input::IsKeyPressed(KeyCode::LeftControl))
					{
						float angle = std::fmod(glm::degrees(rotation1) + 720.0f, 360.0f);
						float newAngle = angle + 2.5f - std::fmod(angle + 2.5f, 5.0f);

						AB1 = glm::rotateY(AB1, -glm::radians(angle - newAngle));
						m_ConstructionPositions[1] = m_ConstructionPositions[0] + AB1;
					}
				}
				else if (cubicCurveOrder[3] != 1 && length2 > 0.1f)
				{
					if (m_StartSnappedEnd)
					{
						float snappedRoadRotationY = glm::degrees(m_StartSnappedEnd->object->rotation.y) + 180.0f;
						float newRoadRotationY = glm::degrees(rotation2);
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

						AB2 = glm::rotateY(AB2, glm::radians(angle - newAngle));
						m_ConstructionPositions[cubicCurveOrder[3]] = m_ConstructionPositions[cubicCurveOrder[2]] + AB2;
					}
					else if (m_StartSnappedRoadSegment)
					{
						float snappedRoadRotationY = glm::degrees(m_StartSnappedRoadSegment->GetStartRotation().y);
						float newRoadRotationY = glm::degrees(rotation2);
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

						AB2 = glm::rotateY(AB2, glm::radians(angle - newAngle));
						m_ConstructionPositions[cubicCurveOrder[3]] = m_ConstructionPositions[cubicCurveOrder[2]] + AB2;
					}
					else if (m_StartSnappedJunction)
					{
						float newRoadRotationY = glm::degrees(rotation2);
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

						AB2 = glm::rotateY(AB2, glm::radians(smallestAngle - newAngle));
						m_ConstructionPositions[cubicCurveOrder[3]] = m_ConstructionPositions[cubicCurveOrder[2]] + AB2;
					}
					else if (Input::IsKeyPressed(KeyCode::LeftControl))
					{
						float angle = std::fmod(glm::degrees(rotation2) + 720.0f, 360.0f);
						float newAngle = angle + 2.5f - std::fmod(angle + 2.5f, 5.0f);

						AB2 = glm::rotateY(AB2, -glm::radians(angle - newAngle));
						m_ConstructionPositions[cubicCurveOrder[3]] = m_ConstructionPositions[cubicCurveOrder[2]] + AB2;
					}
				}
			}

			std::array<std::array<glm::vec2, 3>, 2> newRoadBoundingBox = Math::GetBoundingBoxOfBezierCurve(m_ConstructionPositions, roadPrefabWidth * 0.5f);
			std::array<std::array<glm::vec2, 3>, (10 - 1) * 2> newRoadBoundingPolygon = Math::GetBoundingPolygonOfBezierCurve<10, 10>(m_ConstructionPositions, roadPrefabWidth * 0.5f);

			bool lengthIsRestricted = restrictions[1] && (glm::length(m_ConstructionPositions[0] - m_ConstructionPositions[1]) < 2.0f * roadPrefabLength) && (glm::length(m_ConstructionPositions[3] - m_ConstructionPositions[2]) < 2.0f * roadPrefabLength);
			bool collisionIsRestricted = restrictions[2] ? CheckRoadRoadCollision(newRoadBoundingBox, newRoadBoundingPolygon) : false;

			if (m_Scene->m_BuildingManager.restrictions[0] && restrictions[2])
				CheckRoadBuildingCollision(newRoadBoundingBox, newRoadBoundingPolygon);
			if (m_Scene->m_TreeManager.restrictions[0] && restrictions[2])
				CheckRoadTreeCollision(newRoadBoundingBox, newRoadBoundingPolygon);

			b_ConstructionRestricted |= angleIsRestricted;
			b_ConstructionRestricted |= lengthIsRestricted;
			b_ConstructionRestricted |= collisionIsRestricted;

			DrawCurvedGuidelines(m_ConstructionPositions);
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
		}
	}
	void RoadManager::OnUpdate_Destruction(glm::vec3& prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
	{
		SnapInformation snapInformation = CheckSnapping(prevLocation);
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

		m_GuidelinesStart->SetTransform(pointA + glm::vec3{ 0.0f, 0.15f, 0.0f }, glm::vec3(1.0f), { 0.0f, rotationStart, 0.0f });
		m_GuidelinesEnd->SetTransform(pointB + glm::vec3{ 0.0f, 0.15f, 0.0f }, glm::vec3(1.0f), { 0.0f, rotationEnd, 0.0f });

		m_GuidelinesStart->tintColor = b_ConstructionRestricted ? glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f } : glm::vec4(1.0f);
		m_GuidelinesEnd->tintColor = b_ConstructionRestricted ? glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f } : glm::vec4(1.0f);

		for (std::vector<Object*>& os : m_Guidelines)
			for (Object* rg : os)
				rg->tintColor = b_ConstructionRestricted ? glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f } : glm::vec4(1.0f);

	}
	void RoadManager::DrawCurvedGuidelines(const std::array<glm::vec3, 4>& curvePoints)
	{
		Prefab* selectedRoad = m_Scene->MainApplication->roads[m_Type][0];
		float roadPrefabWidth = selectedRoad->boundingBoxM.z - selectedRoad->boundingBoxL.z;
		float roadPrefabLength = selectedRoad->boundingBoxM.x - selectedRoad->boundingBoxL.x;

		float l = glm::length(curvePoints[3] - curvePoints[0]);
		size_t count = 1;
		while (l > roadPrefabLength)
		{
			count *= 2;
			glm::vec3 p = Math::CubicCurve<float>(curvePoints, 1.0f / count);
			l = glm::length(p - curvePoints[0]);
		}
		if (count > 1) count /= 2;
		while (l > roadPrefabLength)
		{
			count++;
			glm::vec3 p = Math::CubicCurve<float>(curvePoints, 1.0f / count);
			l = glm::length(p - curvePoints[0]);
		}
		if (count > 1) count--;

		glm::vec3 AB1 = curvePoints[1] - curvePoints[0];
		glm::vec3 AB2 = curvePoints[2] - curvePoints[3];

		float rotationOffset1 = (float)(AB1.x >= 0.0f) * glm::radians(180.0f);
		float rotationOffset2 = (float)(AB2.x >= 0.0f) * glm::radians(180.0f);

		float rotationStart = glm::atan(-AB1.z / AB1.x) + rotationOffset1;
		float rotationEnd = glm::atan(-AB2.z / AB2.x) + rotationOffset2;

		m_GuidelinesStart->enabled = !b_ConstructionStartSnapped;
		m_GuidelinesEnd->enabled = !b_ConstructionEndSnapped;

		m_GuidelinesStart->SetTransform(curvePoints[0] + glm::vec3{ 0.0f, 0.15f, 0.0f }, glm::vec3(1.0f), { 0.0f, rotationStart, 0.0f });
		m_GuidelinesEnd->SetTransform(curvePoints[3] + glm::vec3{ 0.0f, 0.15f, 0.0f }, glm::vec3(1.0f), { 0.0f, rotationEnd, 0.0f });

		for (size_t& inUse : m_GuidelinesInUse)
			inUse = 0;
		m_GuidelinesInUse[m_Type] += count;

		if (m_GuidelinesInUse[m_Type] > m_Guidelines[m_Type].size())
			for (size_t j = m_Guidelines[m_Type].size(); j < m_GuidelinesInUse[m_Type]; j++)
				m_Guidelines[m_Type].push_back(new Object(m_Scene->MainApplication->roads[m_Type][0], m_Scene->MainApplication->roads[m_Type][0]));

		glm::vec3 p1 = curvePoints[0];
		for (int c = 0; c < count; c++)
		{
			glm::vec3 p2 = Math::CubicCurve<float>(std::array<glm::vec3, 4>{curvePoints}, (c + 1.0f) / count);
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

		m_GuidelinesStart->tintColor = b_ConstructionRestricted ? glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f } : glm::vec4(1.0f);
		m_GuidelinesEnd->tintColor = b_ConstructionRestricted ? glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f } : glm::vec4(1.0f);

		for (std::vector<Object*>& os : m_Guidelines)
			for (Object* rg : os)
				rg->tintColor = b_ConstructionRestricted ? glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f } : glm::vec4(1.0f);
	}

	bool RoadManager::CheckStraightRoadRoadCollision(const std::array<std::array<glm::vec2, 3>, 2>& polygon)
	{
		for (RoadSegment* roadSegment : m_RoadSegments)
		{
			if (b_ConstructionStartSnapped)
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
			}
			if (b_ConstructionEndSnapped)
			{
				if (roadSegment == m_EndSnappedRoadSegment)
					continue;
				if (m_EndSnappedEnd && roadSegment == m_EndSnappedEnd->connectedRoadSegment)
					continue;
				if (m_EndSnappedJunction)
				{
					auto it = std::find(m_EndSnappedJunction->connectedRoadSegments.begin(), m_EndSnappedJunction->connectedRoadSegments.end(), roadSegment);
					if (it != m_EndSnappedJunction->connectedRoadSegments.end())
						continue;
				}
			}
			float halfWidth = 0.5f * (roadSegment->Type[0]->boundingBoxM.z - roadSegment->Type[0]->boundingBoxL.z);

			std::array<std::array<glm::vec2, 3>, 2> oldRoadPolygon = Math::GetBoundingBoxOfBezierCurve(roadSegment->GetCurvePoints(), halfWidth);

			if (Math::CheckPolygonCollision(polygon, oldRoadPolygon))
			{
				std::array<std::array<glm::vec2, 3>, (10 - 1) * 2> result = Math::GetBoundingPolygonOfBezierCurve<10, 10>(roadSegment->GetCurvePoints(), halfWidth);
				if (Math::CheckPolygonCollision(result, polygon))
					return true;
			}
		}
		return false;
	}
	void RoadManager::CheckStraightRoadBuildingCollision(const std::array<std::array<glm::vec2, 3>, 2>& polygon)
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
			if (Math::CheckPolygonCollision(polygon, polygonBuilding))
				building->object->tintColor = glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f };
		}
	}
	void RoadManager::CheckStraightRoadTreeCollision(const std::array<std::array<glm::vec2, 3>, 2>& polygon)
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
			if (Math::CheckPolygonCollision(polygon, polygonTree))
				tree->tintColor = glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f };
		}

	}

	bool RoadManager::CheckRoadRoadCollision(const std::array<std::array<glm::vec2, 3>, 2>& box, const std::array<std::array<glm::vec2, 3>, (10 - 1) * 2>& polygon)
	{
		for (RoadSegment* roadSegment : m_RoadSegments)
		{
			if (b_ConstructionStartSnapped)
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
			}
			if (b_ConstructionEndSnapped)
			{
				if (roadSegment == m_EndSnappedRoadSegment)
					continue;
				if (m_EndSnappedEnd && roadSegment == m_EndSnappedEnd->connectedRoadSegment)
					continue;
				if (m_EndSnappedJunction)
				{
					auto it = std::find(m_EndSnappedJunction->connectedRoadSegments.begin(), m_EndSnappedJunction->connectedRoadSegments.end(), roadSegment);
					if (it != m_EndSnappedJunction->connectedRoadSegments.end())
						continue;
				}
			}
			float halfWidth = 0.5f * (roadSegment->Type[0]->boundingBoxM.z - roadSegment->Type[0]->boundingBoxL.z);

			std::array<std::array<glm::vec2, 3>, 2> oldRoadBoundingBox = Math::GetBoundingBoxOfBezierCurve(roadSegment->GetCurvePoints(), halfWidth);

			if (Math::CheckPolygonCollision(box, oldRoadBoundingBox))
			{
				if (Math::CheckPolygonCollision(polygon, oldRoadBoundingBox))
				{
					std::array<std::array<glm::vec2, 3>, (10 - 1) * 2> oldRoadBoundingPolygon = Math::GetBoundingPolygonOfBezierCurve<10, 10>(roadSegment->GetCurvePoints(), halfWidth);
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
	void RoadManager::CheckRoadBuildingCollision(const std::array<std::array<glm::vec2, 3>, 2>& box, const std::array<std::array<glm::vec2, 3>, (10 - 1) * 2>& polygon)
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
			if (Math::CheckPolygonCollision(box, polygonBuilding))
				if (Math::CheckPolygonCollision(polygon, polygonBuilding))
					building->object->tintColor = glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f };
		}
	}
	void RoadManager::CheckRoadTreeCollision(const std::array<std::array<glm::vec2, 3>, 2>& box, const std::array<std::array<glm::vec2, 3>, (10 - 1) * 2>& polygon)
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
			if (Math::CheckPolygonCollision(box, polygonTree))
				if (Math::CheckPolygonCollision(polygon, polygonTree))
					tree->tintColor = glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f };
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
		case RoadConstructionMode::Upgrade:
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
		m_ConstructionPhase++;

		if (m_ConstructionPhase > 1)
		{
			m_ConstructionPositions[1] = (m_ConstructionPositions[0] + m_ConstructionPositions[3]) / 2.0f;
			m_ConstructionPositions[2] = (m_ConstructionPositions[0] + m_ConstructionPositions[3]) / 2.0f;

			for (std::vector<Object*>& os : m_Guidelines)
				for (Object* rg : os)
					rg->enabled = false;
			for (size_t& inUse : m_GuidelinesInUse)
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
		m_ConstructionPhase++;

		if (m_ConstructionPhase > 2)
		{
			for (std::vector<Object*>& os : m_Guidelines)
				for (Object* rg : os)
					rg->enabled = false;
			for (size_t& inUse : m_GuidelinesInUse)
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
		m_ConstructionPhase++;

		if (m_ConstructionPhase > 3)
		{
			for (std::vector<Object*>& os : m_Guidelines)
				for (Object* rg : os)
					rg->enabled = false;
			for (size_t& inUse : m_GuidelinesInUse)
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
		if (m_Type == type) return;
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
		case RoadConstructionMode::None:
			break;
		case RoadConstructionMode::Straight:
		case RoadConstructionMode::QuadraticCurve:
		case RoadConstructionMode::CubicCurve:
			m_GuidelinesStart->enabled = true;
			m_GuidelinesEnd->enabled = true;
			break;
		case RoadConstructionMode::Upgrade:
			break;
		case RoadConstructionMode::Destruct:
			break;
		default:
			break;
		}
	}

	void RoadManager::AddRoadSegment(const std::array<glm::vec3, 4>& curvePoints)
	{
		Prefab* selectedRoad = m_Scene->MainApplication->roads[m_Type][0];
		float roadPrefabWidth = selectedRoad->boundingBoxM.z - selectedRoad->boundingBoxL.z;
		RoadSegment* roadSegment = new RoadSegment(
			m_Scene->MainApplication->roads[m_Type],
			curvePoints
		);
		m_RoadSegments.push_back(roadSegment);

		std::array<std::array<glm::vec2, 3>, 2> newRoadBoundingBox = Math::GetBoundingBoxOfBezierCurve(curvePoints, roadPrefabWidth * 0.5f);
		std::array<std::array<glm::vec2, 3>, (10 - 1) * 2> newRoadBoundingPolygon = Math::GetBoundingPolygonOfBezierCurve<10, 10>(curvePoints, roadPrefabWidth * 0.5f);

		/* check collisions for End s if not snapped
		if (m_StartSnappedEnd || m_StartSnappedJunction || m_StartSnappedRoadSegment)
			least.x = 0.0f;
		if (m_EndSnappedEnd || m_EndSnappedJunction || m_EndSnappedRoadSegment)
			most.x = glm::length(AB);*/

		auto& buildings = m_Scene->m_BuildingManager.GetBuildings();
		if (m_Scene->m_BuildingManager.restrictions[0] && restrictions[2])
			for (size_t i = 0; i < buildings.size(); i++)
			{
				Building* building = buildings[i];

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
			RoadSegment* connectedRoadSegment = m_StartSnappedEnd->connectedRoadSegment;

			Junction* newJunction = new Junction(std::vector<RoadSegment*>{ connectedRoadSegment, roadSegment }, m_StartSnappedEnd->object->position);
			roadSegment->ConnectedObjectAtStart.junction = newJunction;

			if (connectedRoadSegment->ConnectedObjectAtStart.end == m_StartSnappedEnd)
			{

				connectedRoadSegment->ConnectedObjectAtStart.end = nullptr;
				connectedRoadSegment->ConnectedObjectAtStart.junction = newJunction;
			}
			else
			{
				connectedRoadSegment->ConnectedObjectAtEnd.end = nullptr;
				connectedRoadSegment->ConnectedObjectAtEnd.junction = newJunction;
			}

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
				curvePoints[0],
			};
			curve[1] = curve[0] + (curve[1] - curve[0]) * m_StartSnappedRoadSegmentT;
			curve[2] = Math::QuadraticCurve(std::array<glm::vec3, 3>{
				m_StartSnappedRoadSegment->GetCurvePoint(0),
					m_StartSnappedRoadSegment->GetCurvePoint(1),
					m_StartSnappedRoadSegment->GetCurvePoint(2)
			}, m_StartSnappedRoadSegmentT);

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

				rs1->ConnectedObjectAtStart.junction = roadSegmentStartSnappedJunction;
				roadSegmentStartSnappedJunction->connectedRoadSegments.push_back(rs1);
				roadSegmentStartSnappedJunction->ReconstructObject();
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
				curvePoints[0],
			};

			curve[1] = curve[1] + (curve[0] - curve[1]) * m_StartSnappedRoadSegmentT;
			curve[2] = Math::QuadraticCurve(std::array<glm::vec3, 3>{
				m_StartSnappedRoadSegment->GetCurvePoint(1),
					m_StartSnappedRoadSegment->GetCurvePoint(2),
					m_StartSnappedRoadSegment->GetCurvePoint(3)
			}, m_StartSnappedRoadSegmentT);

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

				rs2->ConnectedObjectAtStart.junction = roadSegmentEndSnappedJunction;
				roadSegmentEndSnappedJunction->connectedRoadSegments.push_back(rs2);
				roadSegmentEndSnappedJunction->ReconstructObject();
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
				float t = building->snappedT;
				if (t < m_StartSnappedRoadSegmentT)
				{
					building->snappedT = t / m_StartSnappedRoadSegmentT;
					rs1->Buildings.push_back(building);
				}
				else
				{
					building->snappedT = (1.0f - t) / (1.0f - m_StartSnappedRoadSegmentT);
					rs2->Buildings.push_back(building);
				}
			}

			auto it = std::find(m_RoadSegments.begin(), m_RoadSegments.end(), m_StartSnappedRoadSegment);
			m_RoadSegments.erase(it);
			delete m_StartSnappedRoadSegment;

			Junction* newJunction = new Junction(std::vector<RoadSegment*>{ roadSegment, rs1, rs2 }, curvePoints[0]);
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
				curvePoints[0],
				glm::vec3(1.0f),
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
			RoadSegment* connectedRoadSegment = m_EndSnappedEnd->connectedRoadSegment;

			Junction* newJunction = new Junction(std::vector<RoadSegment*>{ connectedRoadSegment, roadSegment}, m_EndSnappedEnd->object->position);
			roadSegment->ConnectedObjectAtEnd.junction = newJunction;

			if (connectedRoadSegment->ConnectedObjectAtStart.end == m_EndSnappedEnd)
			{
				connectedRoadSegment->ConnectedObjectAtStart.end = nullptr;
				connectedRoadSegment->ConnectedObjectAtStart.junction = newJunction;
			}
			else
			{
				connectedRoadSegment->ConnectedObjectAtEnd.end = nullptr;
				connectedRoadSegment->ConnectedObjectAtEnd.junction = newJunction;
			}


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
				curvePoints[3],
			};
			curve[1] = curve[0] + (curve[1] - curve[0]) * m_EndSnappedRoadSegmentT;
			curve[2] = Math::QuadraticCurve(std::array<glm::vec3, 3>{
				m_EndSnappedRoadSegment->GetCurvePoint(0),
					m_EndSnappedRoadSegment->GetCurvePoint(1),
					m_EndSnappedRoadSegment->GetCurvePoint(2)
			}, m_EndSnappedRoadSegmentT);


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

				rs1->ConnectedObjectAtStart.junction = roadSegmentStartSnappedJunction;
				roadSegmentStartSnappedJunction->connectedRoadSegments.push_back(rs1);
				roadSegmentStartSnappedJunction->ReconstructObject();
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
				curvePoints[3],
			};
			curve[1] = curve[1] + (curve[0] - curve[1]) * m_EndSnappedRoadSegmentT;
			curve[2] = Math::QuadraticCurve(std::array<glm::vec3, 3>{
				m_EndSnappedRoadSegment->GetCurvePoint(1),
					m_EndSnappedRoadSegment->GetCurvePoint(2),
					m_EndSnappedRoadSegment->GetCurvePoint(3)
			}, m_EndSnappedRoadSegmentT);
			RoadSegment* rs2 = new RoadSegment(
				m_EndSnappedRoadSegment->Type,
				curve
			);

			Junction* roadSegmentEndSnappedJunction = m_EndSnappedRoadSegment->ConnectedObjectAtEnd.junction;
			if (roadSegmentEndSnappedJunction != nullptr)
			{
				auto it = std::find(
					roadSegmentEndSnappedJunction->connectedRoadSegments.begin(),
					roadSegmentEndSnappedJunction->connectedRoadSegments.end(),
					m_EndSnappedRoadSegment
				);
				roadSegmentEndSnappedJunction->connectedRoadSegments.erase(it);

				rs2->ConnectedObjectAtStart.junction = roadSegmentEndSnappedJunction;
				roadSegmentEndSnappedJunction->connectedRoadSegments.push_back(rs2);
				roadSegmentEndSnappedJunction->ReconstructObject();
			}
			else
			{
				rs2->ConnectedObjectAtStart.end = m_EndSnappedRoadSegment->ConnectedObjectAtEnd.end;
				m_EndSnappedRoadSegment->ConnectedObjectAtEnd.end->connectedRoadSegment = rs2;
			}
			m_RoadSegments.push_back(rs2);

			for (Building* building : m_EndSnappedRoadSegment->Buildings)
			{
				float t = building->snappedT;
				if (t < m_EndSnappedRoadSegmentT)
				{
					building->snappedT = t / m_EndSnappedRoadSegmentT;
					rs1->Buildings.push_back(building);
				}
				else
				{
					building->snappedT = (1.0f - t) / (1.0f - m_EndSnappedRoadSegmentT);
					rs2->Buildings.push_back(building);
				}
			}

			auto it = std::find(m_RoadSegments.begin(), m_RoadSegments.end(), m_EndSnappedRoadSegment);
			m_RoadSegments.erase(it);
			delete m_EndSnappedRoadSegment;

			Junction* newJunction = new Junction(std::vector<RoadSegment*>{ roadSegment, rs1, rs2 }, curvePoints[3]);
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
				curvePoints[3],
				glm::vec3(1.0f),
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
						glm::vec3(1.0f),
						glm::vec3{
							0.0f,
							otherRoadSegment->GetStartRotation().y + glm::radians(180.0f),
							otherRoadSegment->GetStartRotation().x
						}
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
						glm::vec3(1.0f),
						glm::vec3{
							0.0f,
							otherRoadSegment->GetEndRotation().y + glm::radians(180.0f),
							otherRoadSegment->GetEndRotation().x
						}
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
						glm::vec3(1.0f),
						glm::vec3{
							0.0f,
							otherRoadSegment->GetStartRotation().y + glm::radians(180.0f),
							otherRoadSegment->GetStartRotation().x
						}
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
						glm::vec3(1.0f),
						glm::vec3{
							0.0f,
							otherRoadSegment->GetEndRotation().y + glm::radians(180.0f),
							otherRoadSegment->GetEndRotation().x
						}
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

	SnapInformation RoadManager::CheckSnapping(const glm::vec3& prevLocation)
	{
		Prefab* selectedRoad = m_Scene->MainApplication->roads[m_Type][0];
		float roadSegmentPrefabWidth = selectedRoad->boundingBoxM.z - selectedRoad->boundingBoxL.z;
		float roadPrefabLength = selectedRoad->boundingBoxM.x - selectedRoad->boundingBoxL.x;

		SnapInformation results;
		results.location = prevLocation;
		glm::vec2 P{ prevLocation.x, prevLocation.z };

		for (Junction* junction : m_Junctions)
		{
			float distance = glm::length(junction->position - prevLocation);
			if (distance < roadSegmentPrefabWidth)
			{
				results.location = junction->position;
				results.snapped = true;
				results.junction = junction;
				return results;
			}
		}

		for (End* end : m_Ends)
		{
			float endRadius = end->object->prefab->boundingBoxM.x - end->object->prefab->boundingBoxL.x;
			float snapDistance = roadSegmentPrefabWidth * 0.5f + endRadius;

			float distance = glm::length(end->object->position - prevLocation);
			if (distance < snapDistance)
			{
				results.location = end->object->position;
				results.snapped = true;
				results.end = end;
				return results;
			}
		}

		for (RoadSegment* roadSegment : m_RoadSegments)
		{
			float roadSegmentWidth = roadSegment->Type[0]->boundingBoxM.z - roadSegment->Type[0]->boundingBoxL.z;
			float snapDistance = (roadSegmentPrefabWidth + roadSegmentWidth) * 0.5f;

			const auto& cps = roadSegment->GetCurvePoints();

			std::array<std::array<glm::vec2, 3>, 2> roadSegmentBoundingBox = Math::GetBoundingBoxOfBezierCurve(cps, snapDistance);
			if (Math::CheckPolygonPointCollision(roadSegmentBoundingBox, P))
			{
				std::array<float, 10> ts = Math::GetCubicCurveSampleTs<10, 10>(cps);

				for (size_t i = 1; i < 9; i++)
				{
					glm::vec3 pointOnCurve = Math::CubicCurve(cps, ts[i]);
					float distance = glm::length(pointOnCurve - prevLocation);
					if (distance < snapDistance)
					{
						results.location = pointOnCurve;
						results.snapped = true;
						results.roadSegment = roadSegment;

						results.T = ts[i];

						return results;
					}
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