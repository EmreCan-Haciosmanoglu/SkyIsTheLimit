#include "canpch.h"
#include "TestScene.h"
#include "GameApp.h"

#include "Helper.h"

namespace Can
{
	TestScene::TestScene(GameApp* parent)
		: m_Parent(parent)
		, m_Terrain(new Object(m_Parent->terrainPrefab, m_Parent->terrainPrefab, { 0.0f, 0.0f, 0.0f, }, { 1.0f, 1.0f, 1.0f, }, { 0.0f, 0.0f, 0.0f, }))
		, m_MainCameraController(
			45.0f,
			1280.0f / 720.0f,
			0.1f,
			1000.0f,
			glm::vec3{ 1.0f, 5.5f, 0.0f },
			glm::vec3{ -45.0f, 0.0f, 0.0f }
		)
	{
		m_RoadGuidelinesStart = new Object(m_Parent->roads[m_RoadConstructionType][2], m_Parent->roads[m_RoadConstructionType][2], { 0.0f, 0.0f, 0.0f, }, { 1.0f, 1.0f, 1.0f, }, { 0.0f, 0.0f, 0.0f, });
		m_RoadGuidelinesEnd = new Object(m_Parent->roads[m_RoadConstructionType][2], m_Parent->roads[m_RoadConstructionType][2], { 0.0f, 0.0f, 0.0f, }, { 1.0f, 1.0f, 1.0f, }, { 0.0f, 0.0f, 0.0f, });

		size_t roadTypeCount = m_Parent->roads.size();
		for (size_t i = 0; i < roadTypeCount; i++)
		{
			m_RoadGuidelinesInUse.push_back(0);
			m_RoadGuidelines.push_back({});
			m_RoadGuidelines[i].push_back(new Object(m_Parent->roads[i][0], m_Parent->roads[i][0], { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, false));
		}
	}

	void TestScene::OnUpdate(Can::TimeStep ts)
	{
		m_MainCameraController.OnUpdate(ts);

		Can::RenderCommand::SetClearColor({ 0.9f, 0.9f, 0.9f, 1.0f });
		Can::RenderCommand::Clear();

		glm::vec3 camPos = m_MainCameraController.GetCamera().GetPosition();
		glm::vec3 forward = GetRayCastedFromScreen();

		glm::vec3 I = Helper::RayPlaneIntersection(camPos, forward, { 0.0f, 0.0f, 0.0f, }, { 0.0f, 1.0f, 0.0f, });

		if (isnan(I.x) == false)
		{
			switch (m_ConstructionMode)
			{
			case Can::ConstructionMode::Road:
				switch (m_RoadConstructionMode)
				{
				case Can::RoadConstructionMode::None:
					break;
				case Can::RoadConstructionMode::Construct:
					OnUpdate_RoadConstruction(I, camPos, forward);
					break;
				case Can::RoadConstructionMode::Upgrade:
					break;
				case Can::RoadConstructionMode::Destruct:
					OnUpdate_RoadDestruction(I, camPos, forward);
					break;
				}
				break;
			case Can::ConstructionMode::Building:
				break;
			}
		}

		Can::Renderer3D::BeginScene(m_MainCameraController.GetCamera());

		Can::Renderer3D::DrawObjects();

		Can::Renderer3D::EndScene();
	}
	void TestScene::OnUpdate_RoadConstruction(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
	{
		Prefab* selectedRoad = m_Parent->roads[m_RoadConstructionType][0];
		float roadPrefabWidth = selectedRoad->boundingBoxM.z - selectedRoad->boundingBoxL.z;
		float roadPrefabLength = selectedRoad->boundingBoxM.x - selectedRoad->boundingBoxL.x;
		if (b_RoadConstructionStarted == false)
		{
			if (roadSnapOptions[0])
			{
				RoadSnapInformation snapInformation = DidRoadSnapped(cameraPosition, cameraDirection);
				prevLocation = snapInformation.snapped ? snapInformation.snapLocation : prevLocation;
				b_RoadConstructionStartSnapped = snapInformation.snapped;
				m_RoadConstructionStartSnappedJunction = snapInformation.snappedJunction;
				m_RoadConstructionStartSnappedEnd = snapInformation.snappedEnd;
				m_RoadConstructionStartSnappedRoad = snapInformation.snappedRoad;
			}
			m_RoadConstructionStartCoordinate = prevLocation;

			m_RoadGuidelinesStart->SetTransform(prevLocation + glm::vec3{ 0.0f, 0.15f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, glm::radians(180.0f), 0.0f });
			m_RoadGuidelinesEnd->SetTransform(prevLocation + glm::vec3{ 0.0f, 0.15f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f });
		}
		else
		{
			b_ConstructionRestricted = false;
			if (roadSnapOptions[0])
			{
				RoadSnapInformation snapInformation = DidRoadSnapped(cameraPosition, cameraDirection);
				prevLocation = snapInformation.snapped ? snapInformation.snapLocation : prevLocation;
				b_RoadConstructionEndSnapped = snapInformation.snapped;
				m_RoadConstructionEndSnappedJunction = snapInformation.snappedJunction;
				m_RoadConstructionEndSnappedEnd = snapInformation.snappedEnd;
				m_RoadConstructionEndSnappedRoad = snapInformation.snappedRoad;
			}
			m_RoadConstructionEndCoordinate = prevLocation;

			bool angleIsRestricted = false;
			if (roadRestrictionOptions[0])
			{
				if (m_RoadConstructionStartSnappedJunction)
				{
					for (Road* road : m_RoadConstructionStartSnappedJunction->connectedRoads)
					{
						glm::vec3 directionOldRoad = road->startJunction == m_RoadConstructionStartSnappedJunction ? road->direction : -road->direction;
						directionOldRoad.y = 0;
						directionOldRoad = glm::normalize(directionOldRoad);

						glm::vec3 directionNewRoad = prevLocation - m_RoadConstructionStartCoordinate;
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
				else if (m_RoadConstructionStartSnappedEnd)
				{
					Road* road = m_RoadConstructionStartSnappedEnd->connectedRoad;

					glm::vec3 directionOldRoad = road->startEnd == m_RoadConstructionStartSnappedEnd ? road->direction : -road->direction;
					directionOldRoad.y = 0;
					directionOldRoad = glm::normalize(directionOldRoad);

					glm::vec3 directionNewRoad = prevLocation - m_RoadConstructionStartCoordinate;
					directionNewRoad.y = 0;
					directionNewRoad = glm::normalize(directionNewRoad);

					float angle = glm::acos(glm::dot(directionOldRoad, directionNewRoad));

					angleIsRestricted = angle < 0.5f;
				}
				else if (m_RoadConstructionStartSnappedRoad)
				{
					glm::vec3 directionOldRoad = m_RoadConstructionStartSnappedRoad->direction;

					directionOldRoad.y = 0;
					directionOldRoad = glm::normalize(directionOldRoad);

					glm::vec3 directionNewRoad = prevLocation - m_RoadConstructionStartCoordinate;
					directionNewRoad.y = 0;
					directionNewRoad = glm::normalize(directionNewRoad);

					float angle = glm::acos(glm::dot(directionOldRoad, directionNewRoad));

					angleIsRestricted = angle < 0.5f || angle > 2.63f;
				}

				if (m_RoadConstructionEndSnappedJunction)
				{
					for (Road* road : m_RoadConstructionEndSnappedJunction->connectedRoads)
					{
						glm::vec3 directionOldRoad = road->startJunction == m_RoadConstructionEndSnappedJunction ? road->direction : -road->direction;
						directionOldRoad.y = 0;
						directionOldRoad = glm::normalize(directionOldRoad);

						glm::vec3 directionNewRoad = m_RoadConstructionStartCoordinate - prevLocation;
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
				else if (m_RoadConstructionEndSnappedEnd)
				{
					Road* road = m_RoadConstructionEndSnappedEnd->connectedRoad;

					glm::vec3 directionOldRoad = road->startEnd == m_RoadConstructionEndSnappedEnd ? road->direction : -road->direction;
					directionOldRoad.y = 0;
					directionOldRoad = glm::normalize(directionOldRoad);

					glm::vec3 directionNewRoad = m_RoadConstructionStartCoordinate - prevLocation;
					directionNewRoad.y = 0;
					directionNewRoad = glm::normalize(directionNewRoad);

					float angle = glm::acos(glm::dot(directionOldRoad, directionNewRoad));

					angleIsRestricted = angle < 0.5f;
				}
				else if (m_RoadConstructionEndSnappedRoad)
				{
					glm::vec3 directionOldRoad = m_RoadConstructionEndSnappedRoad->direction;

					directionOldRoad.y = 0;
					directionOldRoad = glm::normalize(directionOldRoad);

					glm::vec3 directionNewRoad = m_RoadConstructionStartCoordinate - prevLocation;
					directionNewRoad.y = 0;
					directionNewRoad = glm::normalize(directionNewRoad);

					float angle = glm::acos(glm::dot(directionOldRoad, directionNewRoad));

					angleIsRestricted = angle < 0.5f || angle > 2.63f;
				}

			}

			bool collisionIsRestricted = false;
			if (roadRestrictionOptions[2])
			{
				glm::vec2 p0 = { m_RoadConstructionStartCoordinate.x, m_RoadConstructionStartCoordinate.z };
				glm::vec2 p1 = { m_RoadConstructionEndCoordinate.x, m_RoadConstructionEndCoordinate.z };
				for (Road* road : m_Roads)
				{
					if (road == m_RoadConstructionEndSnappedRoad || road == m_RoadConstructionStartSnappedRoad)
						continue;
					if (m_RoadConstructionStartSnappedEnd && road == m_RoadConstructionStartSnappedEnd->connectedRoad)
						continue;
					if (m_RoadConstructionEndSnappedEnd && road == m_RoadConstructionEndSnappedEnd->connectedRoad)
						continue;
					if (m_RoadConstructionStartSnappedJunction)
					{
						auto it = std::find(m_RoadConstructionStartSnappedJunction->connectedRoads.begin(), m_RoadConstructionStartSnappedJunction->connectedRoads.end(), road);
						if (it != m_RoadConstructionStartSnappedJunction->connectedRoads.end())
							continue;
					}
					if (m_RoadConstructionEndSnappedJunction)
					{
						auto it = std::find(m_RoadConstructionEndSnappedJunction->connectedRoads.begin(), m_RoadConstructionEndSnappedJunction->connectedRoads.end(), road);
						if (it != m_RoadConstructionEndSnappedJunction->connectedRoads.end())
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
					if(Helper::DistanceBetweenLineSLineS(p0, p1, p2, p3) < width)
					{
						collisionIsRestricted = true;
						break;
					}
				}
			}

			bool collisionWitBuildingIsRestricted = false;
			if (roadRestrictionOptions[3])
			{
				/*
				glm::vec3 SE = m_RoadConstructionEndCoordinate - m_RoadConstructionStartCoordinate;
				glm::vec3 ray = glm::normalize(SE);
				float length = glm::length(SE);
				for (Object* building : m_Buildings)
				{
					glm::vec3 least = building->prefab->boundingBoxL;
					glm::vec3 most = building->prefab->boundingBoxM;
					if (Helper::CheckBoundingRectangleHit(m_RoadConstructionStartCoordinate, ray, length, least, most))
					{
						m_CollidedBuilding = building;
						collisionWitBuildingIsRestricted = true;
						continue;
					}
				}
				*/
			}

			bool collisionWithOtherObjectsIsRestricted = false;
			if (roadRestrictionOptions[4])
			{
				// Future checks
			}

			glm::vec3 AB = m_RoadConstructionEndCoordinate - m_RoadConstructionStartCoordinate;

			float rotationOffset = AB.x < 0.0f ? 180.0f : 0.0f;
			float rotationStart = glm::atan(-AB.z / AB.x) + glm::radians(180.0f + rotationOffset);
			float rotationEnd = glm::atan(-AB.z / AB.x) + glm::radians(rotationOffset);

			if (roadSnapOptions[1] && glm::length(AB) > 0.5f)
			{
				float length = glm::length(AB);
				length = length - std::fmod(length, roadPrefabLength);
				AB = length * glm::normalize(AB);
				m_RoadConstructionEndCoordinate = m_RoadConstructionStartCoordinate + AB;
			}

			if (roadSnapOptions[2] && glm::length(AB) > 0.5f)
			{
				if (
					!m_RoadDestructionSnappedJunction &&
					!m_RoadDestructionSnappedRoad &&
					!m_RoadDestructionSnappedEnd
					)
					m_RoadConstructionEndCoordinate.y = m_RoadConstructionStartCoordinate.y;
			}

			if (roadSnapOptions[3] && glm::length(AB) > 0.5f)
			{
				if (m_RoadConstructionStartSnappedEnd)
				{
					float snappedRoadRotationY = glm::degrees(m_RoadConstructionStartSnappedEnd->object->rotation.y) + 180.0f;
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
					else
						newAngle = angle + 1.0f - std::fmod(angle + 1.0f, 2.0f);

					AB = glm::rotate(AB, glm::radians(angle - newAngle), { 0.0f, 1.0f, 0.0f });
					m_RoadConstructionEndCoordinate = m_RoadConstructionStartCoordinate + AB;
				}
				else if (m_RoadConstructionStartSnappedRoad)
				{
					float snappedRoadRotationY = glm::degrees(m_RoadConstructionStartSnappedRoad->rotation.y);
					float newRoadRotationY = glm::degrees(rotationEnd);
					float angle = std::fmod(snappedRoadRotationY - newRoadRotationY + 720.0f, 180.0f);

					float newAngle = 0.0f;
					if (angle < 32.0f)
						newAngle = 30.0f;
					else if (angle > 80.0f && angle < 100.0f)
						newAngle = 90.0f;
					else if (angle > 148.0f && angle < 160.0f)
						newAngle = 150.0f;
					else
						newAngle = angle + 1.0f - std::fmod(angle + 1.0f, 2.0f);

					AB = glm::rotate(AB, glm::radians(angle - newAngle), { 0.0f, 1.0f, 0.0f });
					m_RoadConstructionEndCoordinate = m_RoadConstructionStartCoordinate + AB;
				}
				else if (m_RoadConstructionStartSnappedJunction)
				{
					float newRoadRotationY = glm::degrees(rotationEnd);
					float smallestAngle = 180.0f;
					for (Road* road : m_RoadConstructionStartSnappedJunction->connectedRoads)
					{
						float snappedRoadRotationY = m_RoadConstructionStartSnappedJunction == road->startJunction ? glm::degrees(road->rotation.y) : glm::degrees(road->rotation.y) + 180.0f;
						float angle = std::fmod(snappedRoadRotationY - newRoadRotationY + 720.0f, 180.0f);
						smallestAngle = std::min(smallestAngle, angle);
					}
					float newAngle = 0.0f;
					if (smallestAngle < 32.0f)
						newAngle = 30.0f;
					else if (smallestAngle > 80.0f && smallestAngle < 100.0f)
						newAngle = 90.0f;
					else
						newAngle = smallestAngle + 1.0f - std::fmod(smallestAngle + 1.0f, 2.0f);

					AB = glm::rotate(AB, glm::radians(smallestAngle - newAngle), { 0.0f, 1.0f, 0.0f });
					m_RoadConstructionEndCoordinate = m_RoadConstructionStartCoordinate + AB;
				}
				else
				{
					float angle = glm::degrees(rotationEnd);
					float newAngle = angle + 1.0f - std::fmod(angle + 1.0f, 2.0f);

					AB = glm::rotate(AB, glm::radians(angle - newAngle), { 0.0f, 1.0f, 0.0f });
					m_RoadConstructionEndCoordinate = m_RoadConstructionStartCoordinate + AB;
				}
			}

			if (glm::length(AB) > 0.5f)
			{
				if (m_RoadConstructionEndSnappedRoad)
				{
					glm::vec3 n = { -m_RoadConstructionEndSnappedRoad->direction.z,0,m_RoadConstructionEndSnappedRoad->direction.x };
					m_RoadConstructionEndCoordinate = Helper::RayPlaneIntersection(
						m_RoadConstructionStartCoordinate,
						AB,
						m_RoadConstructionEndSnappedRoad->GetStartPosition(),
						n
					);
					AB = m_RoadConstructionEndCoordinate - m_RoadConstructionStartCoordinate;
				}
				else if (m_RoadConstructionEndSnappedEnd)
				{
					m_RoadConstructionEndCoordinate = m_RoadConstructionEndSnappedEnd->position;
					AB = m_RoadConstructionEndCoordinate - m_RoadConstructionStartCoordinate;
				}
				else if (m_RoadConstructionEndSnappedJunction)
				{
					m_RoadConstructionEndCoordinate = m_RoadConstructionEndSnappedJunction->position;
					AB = m_RoadConstructionEndCoordinate - m_RoadConstructionStartCoordinate;
				}
			}
			glm::vec3 normalizedAB = glm::normalize(AB);

			rotationOffset = AB.x < 0.0f ? 180.0f : 0.0f;
			rotationStart = glm::atan(-AB.z / AB.x) + glm::radians(180.0f + rotationOffset);
			rotationEnd = glm::atan(-AB.z / AB.x) + glm::radians(rotationOffset);

			m_RoadGuidelinesStart->enabled = !b_RoadConstructionStartSnapped;
			m_RoadGuidelinesEnd->enabled = !b_RoadConstructionEndSnapped;

			m_RoadGuidelinesStart->SetTransform(m_RoadConstructionStartCoordinate + glm::vec3{ 0.0f, 0.15f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, rotationStart, 0.0f });
			m_RoadGuidelinesEnd->SetTransform(m_RoadConstructionEndCoordinate + glm::vec3{ 0.0f, 0.15f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, rotationEnd, 0.0f });

			float availableABLength = (
				glm::length(AB)
				- (b_RoadConstructionStartSnapped ? roadPrefabLength : 0.0f)
				- (b_RoadConstructionEndSnapped ? roadPrefabLength : 0.0f)
				);
			availableABLength = std::max(availableABLength, 0.0f);

			int countAB = (int)(availableABLength / roadPrefabLength);
			float scaleAB = (availableABLength / roadPrefabLength) / countAB;
			float scaledRoadLength = availableABLength / countAB;

			bool lengthIsRestricted = roadRestrictionOptions[1] && countAB < 1;


			for (std::vector<Object*>& os : m_RoadGuidelines)
				for (Object* rg : os)
					rg->enabled = false;

			for (size_t& inUse : m_RoadGuidelinesInUse)
				inUse = 0;

			int discountStart = (b_RoadConstructionStartSnapped ? 1 : 0);

			m_RoadGuidelinesInUse[m_RoadConstructionType] += countAB;
			if (m_RoadGuidelinesInUse[m_RoadConstructionType] > m_RoadGuidelines[m_RoadConstructionType].size())
				for (size_t j = m_RoadGuidelines[m_RoadConstructionType].size(); j < m_RoadGuidelinesInUse[m_RoadConstructionType]; j++)
					m_RoadGuidelines[m_RoadConstructionType].push_back(new Object(m_Parent->roads[m_RoadConstructionType][0], m_Parent->roads[m_RoadConstructionType][0]));

			for (size_t j = 0; j < countAB; j++)
			{
				Object* roadG = m_RoadGuidelines[m_RoadConstructionType][j];
				roadG->enabled = true;
				roadG->SetTransform(
					m_RoadConstructionStartCoordinate + (normalizedAB * ((j + discountStart) * scaledRoadLength)) + glm::vec3{ 0.0f, 0.15f, 0.0f },
					glm::vec3{ 1.0f * scaleAB, 1.0f, 1.0f },
					glm::vec3{ 0.0f, rotationEnd, 0.0f }
				);
			}

			if (m_RoadConstructionStartSnappedRoad != nullptr)
			{
				float snappedRoadPrefabLength = m_RoadConstructionStartSnappedRoad->type[0]->boundingBoxM.x - m_RoadConstructionStartSnappedRoad->type[0]->boundingBoxL.x;
				size_t snappedRoadTypeIndex = m_RoadConstructionStartSnappedRoad->typeIndex;
				m_RoadConstructionStartSnappedRoad->object->enabled = false;
				glm::vec3 R0I = m_RoadConstructionStartCoordinate - m_RoadConstructionStartSnappedRoad->GetStartPosition();
				glm::vec3 R1I = m_RoadConstructionStartCoordinate - m_RoadConstructionStartSnappedRoad->GetEndPosition();

				glm::vec3 normalizedR0I = glm::normalize(R0I);
				glm::vec3 normalizedR1I = glm::normalize(R1I);

				glm::vec3 rotationR0I = m_RoadConstructionStartSnappedRoad->rotation;
				glm::vec3 rotationR1I = {
					0.0f,
					m_RoadConstructionStartSnappedRoad->rotation.y + glm::radians(180.0f),
					-m_RoadConstructionStartSnappedRoad->rotation.z
				};

				float availableR0ILength = std::max(glm::length(R0I) - snappedRoadPrefabLength, 0.0f);
				float availableR1ILength = std::max(glm::length(R1I) - snappedRoadPrefabLength, 0.0f);

				int countR0I = (int)(availableR0ILength / snappedRoadPrefabLength);
				int countR1I = (int)(availableR1ILength / snappedRoadPrefabLength);

				lengthIsRestricted |= roadRestrictionOptions[1] && countR0I < 2;
				lengthIsRestricted |= roadRestrictionOptions[1] && countR1I < 2;

				float scaleR0I = (availableR0ILength / snappedRoadPrefabLength) / countR0I;
				float scaleR1I = (availableR1ILength / snappedRoadPrefabLength) / countR1I;

				float scaledR0IRoadLength = availableR0ILength / countR0I;
				float scaledR1IRoadLength = availableR1ILength / countR1I;

				size_t prevIndex = m_RoadGuidelinesInUse[snappedRoadTypeIndex];
				m_RoadGuidelinesInUse[snappedRoadTypeIndex] += countR0I;
				m_RoadGuidelinesInUse[snappedRoadTypeIndex] += countR1I;
				if (m_RoadGuidelinesInUse[snappedRoadTypeIndex] > m_RoadGuidelines[snappedRoadTypeIndex].size())
					for (size_t j = m_RoadGuidelines[snappedRoadTypeIndex].size(); j < m_RoadGuidelinesInUse[snappedRoadTypeIndex]; j++)
						m_RoadGuidelines[snappedRoadTypeIndex].push_back(new Object(m_Parent->roads[snappedRoadTypeIndex][0], m_Parent->roads[snappedRoadTypeIndex][0]));

				for (size_t j = 0; j < countR0I; j++)
				{
					Object* roadG = m_RoadGuidelines[snappedRoadTypeIndex][j + prevIndex];
					roadG->enabled = true;
					roadG->SetTransform(
						m_RoadConstructionStartSnappedRoad->GetStartPosition() + normalizedR0I * (j * scaledR0IRoadLength) + glm::vec3{ 0.0f, 0.15f, 0.0f },
						glm::vec3{ 1.0f * scaleR0I, 1.0f, 1.0f },
						rotationR0I
					);
				}

				for (size_t j = 0; j < countR1I; j++)
				{
					Object* roadG = m_RoadGuidelines[snappedRoadTypeIndex][j + prevIndex + countR0I];
					roadG->enabled = true;
					roadG->SetTransform(
						m_RoadConstructionStartSnappedRoad->GetEndPosition() + normalizedR1I * (j * scaledR1IRoadLength) + glm::vec3{ 0.0f, 0.15f, 0.0f },
						glm::vec3{ 1.0f * scaleR1I, 1.0f, 1.0f },
						rotationR1I
					);
				}
			}

			if (m_RoadConstructionEndSnappedRoad != nullptr)
			{
				float snappedRoadPrefabLength = m_RoadConstructionEndSnappedRoad->type[0]->boundingBoxM.x - m_RoadConstructionEndSnappedRoad->type[0]->boundingBoxL.x;
				size_t snappedRoadTypeIndex = m_RoadConstructionEndSnappedRoad->typeIndex;
				m_RoadConstructionEndSnappedRoad->object->enabled = false;
				glm::vec3 R0I = m_RoadConstructionEndCoordinate - m_RoadConstructionEndSnappedRoad->GetStartPosition();
				glm::vec3 R1I = m_RoadConstructionEndCoordinate - m_RoadConstructionEndSnappedRoad->GetEndPosition();

				glm::vec3 normalizedR0I = glm::normalize(R0I);
				glm::vec3 normalizedR1I = glm::normalize(R1I);

				glm::vec3 rotationR0I = m_RoadConstructionEndSnappedRoad->rotation;
				glm::vec3 rotationR1I = {
					0.0f,
					m_RoadConstructionEndSnappedRoad->rotation.y + glm::radians(180.0f),
					-m_RoadConstructionEndSnappedRoad->rotation.z
				};

				float availableR0ILength = std::max(glm::length(R0I) - snappedRoadPrefabLength, 0.0f);
				float availableR1ILength = std::max(glm::length(R1I) - snappedRoadPrefabLength, 0.0f);

				int countR0I = (int)(availableR0ILength / snappedRoadPrefabLength);
				int countR1I = (int)(availableR1ILength / snappedRoadPrefabLength);

				lengthIsRestricted |= roadRestrictionOptions[1] && countR0I < 2;
				lengthIsRestricted |= roadRestrictionOptions[1] && countR1I < 2;

				float scaleR0I = (availableR0ILength / snappedRoadPrefabLength) / countR0I;
				float scaleR1I = (availableR1ILength / snappedRoadPrefabLength) / countR1I;

				float scaledR0IRoadLength = availableR0ILength / countR0I;
				float scaledR1IRoadLength = availableR1ILength / countR1I;


				size_t prevIndex = m_RoadGuidelinesInUse[snappedRoadTypeIndex];
				m_RoadGuidelinesInUse[snappedRoadTypeIndex] += countR0I;
				m_RoadGuidelinesInUse[snappedRoadTypeIndex] += countR1I;
				if (m_RoadGuidelinesInUse[snappedRoadTypeIndex] > m_RoadGuidelines[snappedRoadTypeIndex].size())
					for (size_t j = m_RoadGuidelines[snappedRoadTypeIndex].size(); j < m_RoadGuidelinesInUse[snappedRoadTypeIndex]; j++)
						m_RoadGuidelines[snappedRoadTypeIndex].push_back(new Object(m_Parent->roads[snappedRoadTypeIndex][0], m_Parent->roads[snappedRoadTypeIndex][0]));

				for (size_t j = 0; j < countR0I; j++)
				{
					Object* roadG = m_RoadGuidelines[snappedRoadTypeIndex][j + prevIndex];
					roadG->enabled = true;
					roadG->SetTransform(
						m_RoadConstructionEndSnappedRoad->GetStartPosition() + normalizedR0I * (j * scaledR0IRoadLength) + glm::vec3{ 0.0f, 0.15f, 0.0f },
						glm::vec3{ 1.0f * scaleR0I, 1.0f, 1.0f },
						rotationR0I
					);
				}

				for (size_t j = 0; j < countR1I; j++)
				{
					Object* roadG = m_RoadGuidelines[snappedRoadTypeIndex][j + prevIndex + countR0I];
					roadG->enabled = true;
					roadG->SetTransform(
						m_RoadConstructionEndSnappedRoad->GetEndPosition() + normalizedR1I * (j * scaledR1IRoadLength) + glm::vec3{ 0.0f, 0.15f, 0.0f },
						glm::vec3{ 1.0f * scaleR1I, 1.0f, 1.0f },
						rotationR1I
					);
				}
			}

			b_ConstructionRestricted |= angleIsRestricted;
			b_ConstructionRestricted |= lengthIsRestricted;
			b_ConstructionRestricted |= collisionIsRestricted;
			b_ConstructionRestricted |= collisionWitBuildingIsRestricted;
			b_ConstructionRestricted |= collisionWithOtherObjectsIsRestricted;

			m_RoadGuidelinesStart->tintColor = b_ConstructionRestricted ? glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f } : glm::vec4(1.0f);
			m_RoadGuidelinesEnd->tintColor = b_ConstructionRestricted ? glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f } : glm::vec4(1.0f);

			for (std::vector<Object*>& os : m_RoadGuidelines)
				for (Object* rg : os)
					rg->tintColor = b_ConstructionRestricted ? glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f } : glm::vec4(1.0f);

		}
	}
	void TestScene::OnUpdate_RoadDestruction(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
	{
		RoadSnapInformation snapInformation = DidRoadSnapped(cameraPosition, cameraDirection);
		m_RoadDestructionSnappedJunction = snapInformation.snappedJunction;
		m_RoadDestructionSnappedEnd = snapInformation.snappedEnd;
		m_RoadDestructionSnappedRoad = snapInformation.snappedRoad;

		for (Junction* junction : m_Junctions)
			junction->object->SetTransform(junction->position);

		for (End* end : m_Ends)
			end->object->SetTransform(end->position);

		for (Road* road : m_Roads)
			road->object->SetTransform(road->GetStartPosition());

		if (snapInformation.snapped)
		{
			if (m_RoadDestructionSnappedJunction != nullptr)
			{
				m_RoadDestructionSnappedJunction->object->SetTransform(m_RoadDestructionSnappedJunction->position + glm::vec3{ 0.0f, 0.1f, 0.0f });

				for (Road* road : m_RoadDestructionSnappedJunction->connectedRoads)
				{
					if (road->startEnd != nullptr)
						road->startEnd->object->SetTransform(road->startEnd->position + glm::vec3{ 0.0f, 0.1f, 0.0f });
					if (road->endEnd != nullptr)
						road->endEnd->object->SetTransform(road->endEnd->position + glm::vec3{ 0.0f, 0.1f, 0.0f });
					road->object->SetTransform(road->GetStartPosition() + glm::vec3{ 0.0f, 0.1f, 0.0f });
				}
			}
			else if (m_RoadDestructionSnappedEnd != nullptr)
			{
				Road* road = m_RoadDestructionSnappedEnd->connectedRoad;

				if (road->startEnd != nullptr)
					road->startEnd->object->SetTransform(road->startEnd->position + glm::vec3{ 0.0f, 0.1f, 0.0f });
				if (road->endEnd != nullptr)
					road->endEnd->object->SetTransform(road->endEnd->position + glm::vec3{ 0.0f, 0.1f, 0.0f });
				road->object->SetTransform(road->GetStartPosition() + glm::vec3{ 0.0f, 0.1f, 0.0f });
			}
			else
			{
				if (m_RoadDestructionSnappedRoad->startEnd != nullptr)
					m_RoadDestructionSnappedRoad->startEnd->object->SetTransform(m_RoadDestructionSnappedRoad->startEnd->position + glm::vec3{ 0.0f, 0.1f, 0.0f });
				if (m_RoadDestructionSnappedRoad->endEnd != nullptr)
					m_RoadDestructionSnappedRoad->endEnd->object->SetTransform(m_RoadDestructionSnappedRoad->endEnd->position + glm::vec3{ 0.0f, 0.1f, 0.0f });
				m_RoadDestructionSnappedRoad->object->SetTransform(m_RoadDestructionSnappedRoad->GetStartPosition() + glm::vec3{ 0.0f, 0.1f, 0.0f });
			}
		}
	}

	void TestScene::OnEvent(Can::Event::Event& event)
	{
		m_MainCameraController.OnEvent(event);
		Can::Event::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Can::Event::MouseButtonPressedEvent>(CAN_BIND_EVENT_FN(TestScene::OnMousePressed));
	}

	bool TestScene::OnMousePressed(Can::Event::MouseButtonPressedEvent& event)
	{
		MouseCode button = event.GetMouseButton();
		glm::vec3 camPos = m_MainCameraController.GetCamera().GetPosition();
		glm::vec3 forward = GetRayCastedFromScreen();


		glm::vec3 bottomPlaneCollisionPoint = Helper::RayPlaneIntersection(camPos, forward, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });
		glm::vec3 topPlaneCollisionPoint = Helper::RayPlaneIntersection(camPos, forward, { 0.0f, 1.0f * COLOR_COUNT, 0.0f }, { 0.0f, 1.0f, 0.0f });

		bottomPlaneCollisionPoint.z *= -1;
		topPlaneCollisionPoint.z *= -1;

		if (!Helper::CheckBoundingBoxHit(camPos, forward, m_Terrain->prefab->boundingBoxL, m_Terrain->prefab->boundingBoxM))
			return false;

		switch (m_ConstructionMode)
		{
		case Can::ConstructionMode::Road:
			switch (m_RoadConstructionMode)
			{
			case Can::RoadConstructionMode::None:
				break;
			case Can::RoadConstructionMode::Construct:
				if (button == MouseCode::Button1)
				{
					ResetStates();
					m_RoadGuidelinesStart->enabled = true;
					m_RoadGuidelinesEnd->enabled = true;
					return false;
				}
				else if (button != MouseCode::Button0)
					return false;
				OnMousePressed_RoadConstruction(camPos, forward);
				break;
			case Can::RoadConstructionMode::Upgrade:
				break;
			case Can::RoadConstructionMode::Destruct:
				OnMousePressed_RoadDestruction();
				break;
			}
			break;
		case Can::ConstructionMode::Building:
			break;
		}
		return false;
	}
	bool TestScene::OnMousePressed_RoadConstruction(const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
	{
		float* data = m_Terrain->prefab->vertices;
		glm::vec3 bottomPlaneCollisionPoint = Helper::RayPlaneIntersection(cameraPosition, cameraDirection, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });
		glm::vec3 topPlaneCollisionPoint = Helper::RayPlaneIntersection(cameraPosition, cameraDirection, { 0.0f, 1.0f * COLOR_COUNT, 0.0f }, { 0.0f, 1.0f, 0.0f });

		bottomPlaneCollisionPoint.z *= -1;
		topPlaneCollisionPoint.z *= -1;

		float terrainW = m_Terrain->prefab->boundingBoxM.x * TERRAIN_SCALE_DOWN;
		float terrainH = -m_Terrain->prefab->boundingBoxL.z * TERRAIN_SCALE_DOWN;

		glm::vec2 minCoord = {
			std::max(0.0f, TERRAIN_SCALE_DOWN * (std::min(bottomPlaneCollisionPoint.x, topPlaneCollisionPoint.x) - m_Terrain->position.x) - 1),
			std::max(0.0f, TERRAIN_SCALE_DOWN * (std::min(bottomPlaneCollisionPoint.z, topPlaneCollisionPoint.z) - m_Terrain->position.z) - 1)
		};

		glm::vec2 maxCoord = {
			std::min(terrainW, TERRAIN_SCALE_DOWN * (std::max(bottomPlaneCollisionPoint.x, topPlaneCollisionPoint.x) - m_Terrain->position.x) + 1),
			std::min(terrainH, TERRAIN_SCALE_DOWN * (std::max(bottomPlaneCollisionPoint.z, topPlaneCollisionPoint.z) - m_Terrain->position.z) + 1)
		};


		for (size_t y = (size_t)(minCoord.y); y < maxCoord.y; y++)
			//for (size_t y = 0; y < terrainH; y++)
		{
			for (size_t x = (size_t)(minCoord.x); x < maxCoord.x; x++)
				//for (size_t x = 0; x < terrainW; x++)
			{
				for (size_t z = 0; z < 2; z++)
				{
					size_t index = (x + ((int)terrainW - 1) * y) * 60 + z * 30;
					float* A = &data[index + 0];
					float* B = &data[index + 10];
					float* C = &data[index + 20];
					glm::vec3 intersection;
					bool result = Helper::RayTriangleIntersection(
						cameraPosition,
						cameraDirection,
						{ A[0], A[1], A[2] },
						{ B[0], B[1], B[2] },
						{ C[0], C[1], C[2] },
						{ A[7], A[8], A[9] },
						intersection
					);
					if (result)
					{

						if (!b_RoadConstructionStarted)
						{
							b_RoadConstructionStarted = true;
							if (!b_RoadConstructionStartSnapped)
								m_RoadConstructionStartCoordinate = intersection;
						}
						else if (!b_ConstructionRestricted)
						{
							b_RoadConstructionEnded = true;
							/*if (!b_RoadConstructionEndSnapped)
								m_RoadConstructionEndCoordinate = intersection;*/
							for (std::vector<Object*>& os : m_RoadGuidelines)
								for (Object* rg : os)
									rg->enabled = false;
							for (size_t& inUse : m_RoadGuidelinesInUse)
								inUse = 0;
						}
						goto exit;
					}
				}
			}
		}
	exit:

		if (b_RoadConstructionEnded) //[[unlikely]] 
		{
			Road* newRoad = new Road(
				m_Parent->roads[m_RoadConstructionType][0],
				m_RoadConstructionStartCoordinate,
				m_RoadConstructionEndCoordinate,
				{
					m_Parent->roads[m_RoadConstructionType][0],
					m_Parent->roads[m_RoadConstructionType][1],
					m_Parent->roads[m_RoadConstructionType][2]
				},
				m_RoadConstructionType
			);
			m_Roads.push_back(newRoad);

			if (m_RoadConstructionStartSnappedJunction != nullptr)
			{
				newRoad->startJunction = m_RoadConstructionStartSnappedJunction;
				m_RoadConstructionStartSnappedJunction->connectedRoads.push_back(newRoad);
				m_RoadConstructionStartSnappedJunction->ReconstructObject();
			}
			else if (m_RoadConstructionStartSnappedEnd != nullptr)
			{
				Road* connectedRoad = m_RoadConstructionStartSnappedEnd->connectedRoad;
				if (connectedRoad->startEnd == m_RoadConstructionStartSnappedEnd)
					connectedRoad->startEnd = nullptr;
				else
					connectedRoad->endEnd = nullptr;

				Junction* newJunction = new Junction(std::vector<Road*>{ connectedRoad, newRoad }, m_RoadConstructionStartSnappedEnd->object->position);
				newRoad->startJunction = newJunction;

				if (connectedRoad->GetStartPosition() == m_RoadConstructionStartSnappedEnd->object->position)
					connectedRoad->startJunction = newJunction;
				else
					connectedRoad->endJunction = newJunction;


				auto position = std::find(m_Ends.begin(), m_Ends.end(), m_RoadConstructionStartSnappedEnd);
				m_Ends.erase(position);
				delete m_RoadConstructionStartSnappedEnd;

				m_Junctions.push_back(newJunction);
				newJunction->ConstructObject();
			}
			else if (m_RoadConstructionStartSnappedRoad != nullptr)
			{
				Road* r1 = new Road(
					m_RoadConstructionStartSnappedRoad,
					m_RoadConstructionStartSnappedRoad->GetStartPosition(),
					m_RoadConstructionStartCoordinate
				);
				if (m_RoadConstructionStartSnappedRoad->startJunction != nullptr)
				{
					auto it = std::find(
						m_RoadConstructionStartSnappedRoad->startJunction->connectedRoads.begin(),
						m_RoadConstructionStartSnappedRoad->startJunction->connectedRoads.end(),
						m_RoadConstructionStartSnappedRoad
					);
					m_RoadConstructionStartSnappedRoad->startJunction->connectedRoads.erase(it);
					m_RoadConstructionStartSnappedRoad->startJunction->connectedRoads.push_back(r1);
					r1->startJunction = m_RoadConstructionStartSnappedRoad->startJunction;
				}
				else
				{
					r1->startEnd = m_RoadConstructionStartSnappedRoad->startEnd;
					m_RoadConstructionStartSnappedRoad->startEnd->connectedRoad = r1;
				}
				m_Roads.push_back(r1);


				Road* r2 = new Road(
					m_RoadConstructionStartSnappedRoad,
					m_RoadConstructionStartSnappedRoad->GetEndPosition(),
					m_RoadConstructionStartCoordinate
				);
				if (m_RoadConstructionStartSnappedRoad->endJunction != nullptr)
				{
					auto it = std::find(
						m_RoadConstructionStartSnappedRoad->endJunction->connectedRoads.begin(),
						m_RoadConstructionStartSnappedRoad->endJunction->connectedRoads.end(),
						m_RoadConstructionStartSnappedRoad
					);
					m_RoadConstructionStartSnappedRoad->endJunction->connectedRoads.erase(it);
					m_RoadConstructionStartSnappedRoad->endJunction->connectedRoads.push_back(r2);
					r2->startJunction = m_RoadConstructionStartSnappedRoad->endJunction;
				}
				else
				{
					r2->startEnd = m_RoadConstructionStartSnappedRoad->endEnd;
					m_RoadConstructionStartSnappedRoad->endEnd->connectedRoad = r2;
				}
				m_Roads.push_back(r2);

				auto it = std::find(m_Roads.begin(), m_Roads.end(), m_RoadConstructionStartSnappedRoad);
				m_Roads.erase(it);
				delete m_RoadConstructionStartSnappedRoad;

				Junction* newJunction = new Junction(std::vector<Road*>{ newRoad, r1, r2 }, m_RoadConstructionStartCoordinate);
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
					m_Parent->roads[m_RoadConstructionType][2],
					m_RoadConstructionStartCoordinate,
					glm::vec3{ 1.0f, 1.0f, 1.0f },
					glm::vec3{ 0.0f, newRoad->rotation.y + glm::radians(180.0f), -newRoad->rotation.z }
				);
				newRoad->startEnd = newEnd;
				m_Ends.push_back(newEnd);
			}

			if (m_RoadConstructionEndSnappedJunction != nullptr)
			{
				newRoad->endJunction = m_RoadConstructionEndSnappedJunction;
				m_RoadConstructionEndSnappedJunction->connectedRoads.push_back(newRoad);
				m_RoadConstructionEndSnappedJunction->ReconstructObject();
			}
			else if (m_RoadConstructionEndSnappedEnd != nullptr)
			{
				Road* connectedRoad = m_RoadConstructionEndSnappedEnd->connectedRoad;
				if (connectedRoad->startEnd == m_RoadConstructionEndSnappedEnd)
					connectedRoad->startEnd = nullptr;
				else
					connectedRoad->endEnd = nullptr;


				Junction* newJunction = new Junction(std::vector<Road*>{ connectedRoad, newRoad }, m_RoadConstructionEndSnappedEnd->object->position);
				newRoad->endJunction = newJunction;

				if (connectedRoad->GetStartPosition() == m_RoadConstructionEndSnappedEnd->object->position)
					connectedRoad->startJunction = newJunction;
				else
					connectedRoad->endJunction = newJunction;


				auto position = std::find(m_Ends.begin(), m_Ends.end(), m_RoadConstructionEndSnappedEnd);
				m_Ends.erase(position);
				delete m_RoadConstructionEndSnappedEnd;

				m_Junctions.push_back(newJunction);
				newJunction->ConstructObject();
			}
			else if (m_RoadConstructionEndSnappedRoad != nullptr)
			{
				Road* r1 = new Road(
					m_RoadConstructionEndSnappedRoad,
					m_RoadConstructionEndSnappedRoad->GetStartPosition(),
					m_RoadConstructionEndCoordinate
				);
				if (m_RoadConstructionEndSnappedRoad->startJunction != nullptr)
				{
					auto it = std::find(
						m_RoadConstructionEndSnappedRoad->startJunction->connectedRoads.begin(),
						m_RoadConstructionEndSnappedRoad->startJunction->connectedRoads.end(),
						m_RoadConstructionEndSnappedRoad
					);
					m_RoadConstructionEndSnappedRoad->startJunction->connectedRoads.erase(it);
					m_RoadConstructionEndSnappedRoad->startJunction->connectedRoads.push_back(r1);
					r1->startJunction = m_RoadConstructionEndSnappedRoad->startJunction;
				}
				else
				{
					r1->startEnd = m_RoadConstructionEndSnappedRoad->startEnd;
					m_RoadConstructionEndSnappedRoad->startEnd->connectedRoad = r1;
				}
				m_Roads.push_back(r1);


				Road* r2 = new Road(
					m_RoadConstructionEndSnappedRoad,
					m_RoadConstructionEndSnappedRoad->GetEndPosition(),
					m_RoadConstructionEndCoordinate
				);
				if (m_RoadConstructionEndSnappedRoad->endJunction != nullptr)
				{
					auto it = std::find(
						m_RoadConstructionEndSnappedRoad->endJunction->connectedRoads.begin(),
						m_RoadConstructionEndSnappedRoad->endJunction->connectedRoads.end(),
						m_RoadConstructionEndSnappedRoad
					);
					m_RoadConstructionEndSnappedRoad->endJunction->connectedRoads.erase(it);
					m_RoadConstructionEndSnappedRoad->endJunction->connectedRoads.push_back(r2);
					r2->startJunction = m_RoadConstructionEndSnappedRoad->endJunction;
				}
				else
				{
					r2->startEnd = m_RoadConstructionEndSnappedRoad->endEnd;
					m_RoadConstructionEndSnappedRoad->endEnd->connectedRoad = r2;
				}
				m_Roads.push_back(r2);

				auto it = std::find(m_Roads.begin(), m_Roads.end(), m_RoadConstructionEndSnappedRoad);
				m_Roads.erase(it);
				delete m_RoadConstructionEndSnappedRoad;

				Junction* newJunction = new Junction(std::vector<Road*>{ newRoad, r1, r2 }, m_RoadConstructionEndCoordinate);
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
					m_Parent->roads[m_RoadConstructionType][2],
					m_RoadConstructionEndCoordinate,
					glm::vec3{ 1.0f, 1.0f, 1.0f },
					glm::vec3{ 0.0f, newRoad->rotation.y, newRoad->rotation.z }
				);
				newRoad->endEnd = newEnd;
				m_Ends.push_back(newEnd);
			}

			//Helper::LevelTheTerrain(startIndex, endIndex, newRoad->startPos, newRoad->endPos, m_Parent->m_Terrain, roadPrefabWidth);
			ResetStates();
			m_RoadGuidelinesStart->enabled = true;
			m_RoadGuidelinesEnd->enabled = true;
		}
		return false;
	}
	bool TestScene::OnMousePressed_RoadDestruction()
	{
		if (m_RoadDestructionSnappedJunction != nullptr)
		{
			std::vector<Road*> roads_copy = m_RoadDestructionSnappedJunction->connectedRoads;
			for (Road* road : roads_copy)
				DeleteSelectedRoad(road);
		}
		else if (m_RoadDestructionSnappedEnd != nullptr)
		{
			DeleteSelectedRoad(m_RoadDestructionSnappedEnd->connectedRoad);
		}
		else if (m_RoadDestructionSnappedRoad != nullptr)
		{
			DeleteSelectedRoad(m_RoadDestructionSnappedRoad);
		}
		return false;
	}

	void TestScene::SetSelectedConstructionRoad(size_t index)
	{
		m_RoadConstructionType = index;
		delete m_RoadGuidelinesEnd;
		delete m_RoadGuidelinesStart;
		m_RoadGuidelinesStart = new Object(m_Parent->roads[m_RoadConstructionType][2], m_Parent->roads[m_RoadConstructionType][2], { 0.0f, 0.0f, 0.0f, }, { 1.0f, 1.0f, 1.0f, }, { 0.0f, 0.0f, 0.0f, });
		m_RoadGuidelinesEnd = new Object(m_Parent->roads[m_RoadConstructionType][2], m_Parent->roads[m_RoadConstructionType][2], { 0.0f, 0.0f, 0.0f, }, { 1.0f, 1.0f, 1.0f, }, { 0.0f, 0.0f, 0.0f, });

	}

	void TestScene::DeleteSelectedRoad(Road* road)
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
						m_Parent->roads[otherRoad->typeIndex][2],
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
						m_Parent->roads[otherRoad->typeIndex][2],
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
						m_Parent->roads[otherRoad->typeIndex][2],
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
						m_Parent->roads[otherRoad->typeIndex][2],
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
		delete road;
	}

	void TestScene::SetConstructionMode(ConstructionMode mode)
	{
		m_ConstructionMode = mode;
		switch (m_ConstructionMode)
		{
		case Can::ConstructionMode::Road:
			SetRoadConstructionMode(m_RoadConstructionMode);
			break;
		case Can::ConstructionMode::Building:
			break;
		default:
			break;
		}
	}

	void TestScene::SetRoadConstructionMode(RoadConstructionMode mode)
	{
		ResetStates();
		m_RoadConstructionMode = mode;

		switch (m_RoadConstructionMode)
		{
		case Can::RoadConstructionMode::None:
			break;
		case Can::RoadConstructionMode::Construct:
			m_RoadGuidelinesStart->enabled = true;
			m_RoadGuidelinesEnd->enabled = true;
			break;
		case Can::RoadConstructionMode::Upgrade:
			break;
		case Can::RoadConstructionMode::Destruct:
			break;
		default:
			break;
		}
	}

	void TestScene::ResetStates()
	{
		b_RoadConstructionStarted = false;
		b_RoadConstructionEnded = false;
		b_RoadConstructionStartSnapped = false;
		b_RoadConstructionEndSnapped = false;

		m_RoadConstructionStartCoordinate = { -1.0f, -1.0f, -1.0f };
		m_RoadConstructionEndCoordinate = { -1.0f, -1.0f, -1.0f };

		m_RoadConstructionStartSnappedJunction = nullptr;
		m_RoadConstructionStartSnappedEnd = nullptr;
		m_RoadConstructionStartSnappedRoad = nullptr;

		m_RoadConstructionEndSnappedJunction = nullptr;
		m_RoadConstructionEndSnappedEnd = nullptr;
		m_RoadConstructionEndSnappedRoad = nullptr;

		m_RoadDestructionSnappedJunction = nullptr;
		m_RoadDestructionSnappedEnd = nullptr;
		m_RoadDestructionSnappedRoad = nullptr;

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

		for (std::vector<Object*>& os : m_RoadGuidelines)
			for (Object* rg : os)
				rg->enabled = false;
		for (size_t& inUse : m_RoadGuidelinesInUse)
			inUse = 0;

		m_RoadGuidelinesStart->enabled = false;
		m_RoadGuidelinesEnd->enabled = false;

		m_RoadGuidelinesStart->tintColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		m_RoadGuidelinesEnd->tintColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	}

	glm::vec3 TestScene::GetRayCastedFromScreen()
	{
		auto [mouseX, mouseY] = Can::Input::GetMousePos();
		Application& app = Application::Get();
		float w = (float)(app.GetWindow().GetWidth());
		float h = (float)(app.GetWindow().GetHeight());

		auto camera = m_MainCameraController.GetCamera();
		glm::vec3 camPos = camera.GetPosition();
		glm::vec3 camRot = camera.GetRotation();

		float fovyX = m_MainCameraController.GetFOV();
		float xoffSet = glm::degrees(glm::atan(glm::tan(glm::radians(fovyX)) * (((w / 2.0f) - mouseX) / (w / 2.0f))));
		float yoffSet = glm::degrees(glm::atan(((h - 2.0f * mouseY) * glm::sin(glm::radians(xoffSet))) / (w - 2.0f * mouseX)));

		glm::vec2 offsetDegrees = {
			xoffSet,
			yoffSet
		};

		glm::vec3 forward = {
			-glm::sin(glm::radians(camRot.y)) * glm::cos(glm::radians(camRot.x)),
			glm::sin(glm::radians(camRot.x)),
			-glm::cos(glm::radians(camRot.x)) * glm::cos(glm::radians(camRot.y))
		};
		glm::vec3 up = {
			glm::sin(glm::radians(camRot.x)) * glm::sin(glm::radians(camRot.y)),
			glm::cos(glm::radians(camRot.x)),
			glm::sin(glm::radians(camRot.x)) * glm::cos(glm::radians(camRot.y))
		};
		glm::vec3 right = {
			-glm::sin(glm::radians(camRot.y - 90.0f)),
			0,
			-glm::cos(glm::radians(camRot.y - 90.0f))
		};

		forward = glm::rotate(forward, glm::radians(offsetDegrees.x), up);
		right = glm::rotate(right, glm::radians(offsetDegrees.x), up);
		forward = glm::rotate(forward, glm::radians(offsetDegrees.y), right);
		return forward;
	}

	RoadSnapInformation TestScene::DidRoadSnapped(const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
	{
		Prefab* selectedRoad = m_Parent->roads[m_RoadConstructionType][0];
		float roadPrefabWidth = selectedRoad->boundingBoxM.z - selectedRoad->boundingBoxL.z;
		float snapDistance = roadPrefabWidth;
		RoadSnapInformation results{ false, { 0.0f, 0.0f, 0.0f } };

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
}
