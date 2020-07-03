#include "canpch.h"
#include "TestScene.h"
#include "GameApp.h"

namespace Can
{
	TestScene::TestScene(GameApp* parent)
		: m_Parent(parent)
		, m_MainCameraController(
			45.0f,
			1280.0f / 720.0f,
			0.0001f,
			100.0f,
			{ 1.0f, 0.5f, -1.0f },
			{ -45, 0, 0 }
		)
		, roadPrefab(m_Parent->UploadObject("assets/objects/road.obj", "assets/shaders/Object.glsl", "assets/objects/road.png"))
		, endPrefab(m_Parent->UploadObject("assets/objects/road_end.obj", "assets/shaders/Object.glsl", "assets/objects/road_end.png"))
		, JunctionPrefab(m_Parent->UploadObject("assets/objects/road_junction.obj", "assets/shaders/Object.glsl", "assets/objects/road_junction.png"))
	{
		m_RoadGuidelinesStart = m_Parent->UploadObject("assets/objects/road_end.obj", "assets/shaders/Object.glsl", "assets/objects/road_end.png");
		m_RoadGuidelinesEnd = m_Parent->UploadObject("assets/objects/road_end.obj", "assets/shaders/Object.glsl", "assets/objects/road_end.png");

		roadPrefab->isEnabled = false;
		endPrefab->isEnabled = false;
		JunctionPrefab->isEnabled = false;

		auto road = m_Parent->UploadObject("assets/objects/road.obj", "assets/shaders/Object.glsl", "assets/objects/road.png");
		road->isEnabled = false;
		m_RoadGuidelines.push_back(road);

		float maxX = 0.0f;
		float minX = 0.0f;
		float maxZ = 0.0f;
		float minZ = 0.0f;
		for (size_t j = 0; j < roadPrefab->indexCount; j++)
		{
			float x = roadPrefab->Vertices[j * 8];
			float z = roadPrefab->Vertices[j * 8 + 2];
			maxX = std::max(maxX, x);
			minX = std::min(minX, x);
			maxZ = std::max(maxZ, z);
			minZ = std::min(minZ, z);
		}
		roadPrefabWidth = (maxX - minX);
		roadPrefabLength = (maxZ - minZ);
	}

	void TestScene::OnUpdate(Can::TimeStep ts)
	{
		m_MainCameraController.OnUpdate(ts);

		Can::RenderCommand::SetClearColor({ 0.9f, 0.9f, 0.9f, 1.0f });
		Can::RenderCommand::Clear();

		glm::vec3 camPos = m_MainCameraController.GetCamera().GetPosition();
		glm::vec3 forward = GetRayCastedFromScreen();

		//delete?
		glm::vec3 I = m_Parent->RayPlaneIntersection(camPos, forward, { 0.0f, 0.0f, 0.0f, }, { 0.0f, 1.0f, 0.0f, });

		if (isnan(I.x) == false)
		{
			if (b_RoadConstructionStarted == false)
			{
				m_RoadConstructionStartSnappedType = -1;
				bool snapped = false;
				glm::vec3 prevLocation = I;/* GetTerrainHeigth */
				float snapDist = roadPrefabWidth / 100.0f;
				float roadPrefabLengthInTheScene = roadPrefabLength / 100.0f;

				for (Junction* junction : m_Junctions)
				{
					glm::vec3 Intersection = m_Parent->RayPlaneIntersection(camPos, forward, junction->position, { 0.0f, 1.0f, 0.0f, });

					float distance = glm::length(junction->position - Intersection);
					if (distance < snapDist)
					{
						snapped = true;
						prevLocation = junction->position;
						m_RoadConstructionStartSnappedType = 0;
						m_RoadConstructionStartSnappedJunction = junction;
						break;
					}
				}

				if (!snapped)
				{
					for (End* end : m_Ends)
					{
						glm::vec3 Intersection = m_Parent->RayPlaneIntersection(camPos, forward, end->position, { 0.0f, 1.0f, 0.0f, });

						float distance = glm::length(end->position - Intersection);
						if (distance < snapDist)
						{
							snapped = true;
							prevLocation = end->position;
							m_RoadConstructionStartSnappedType = 1;
							m_RoadConstructionStartSnappedEnd = end;
							break;
						}
					}
				}

				if (!snapped)
				{
					for (Road* road : m_Roads)
					{
						// glm::vec3 roadDir = glm::normalize(roadVec);
						// B.y += roadDir.y*c;

						road->object->isEnabled = true;
						glm::vec3 Intersection = m_Parent->RayPlaneIntersection(camPos, forward, road->startPos, { 0.0f, 1.0f, 0.0f, });

						glm::vec3 roadVec = road->endPos - road->startPos;
						float roadLength = glm::length(roadVec);

						glm::vec3 B = Intersection - road->startPos;
						float bLength = glm::length(B);

						float angle = glm::acos(glm::dot(roadVec, B) / (roadLength * bLength));
						float distance = bLength * glm::sin(angle);

						if (distance < snapDist)
						{
							float c = bLength * glm::cos(angle);
							if (c <= roadPrefabLengthInTheScene || c >= roadLength - roadPrefabLengthInTheScene)
								continue;

							snapped = true;
							prevLocation = road->startPos + glm::normalize(roadVec) * c;
							m_RoadConstructionStartSnappedType = 2;
							m_RoadConstructionStartSnappedRoad = road;
							break;
						}
					}
				}
				b_RoadConstructionStartSnapped = snapped;
				if (snapped)
					m_RoadConstructionStartCoordinate = prevLocation;

				m_Parent->SetTransform(m_RoadGuidelinesStart, prevLocation + glm::vec3{ 0.0f, 0.015f, 0.0f }, { 0.01f, 0.01f, 0.01f });
				m_Parent->SetTransform(m_RoadGuidelinesEnd, prevLocation + glm::vec3{ 0.0f, 0.015f, 0.0f }, { 0.01f, 0.01f, 0.01f }, { 0.0f, glm::radians(180.0f), 0.0f });
			}
			else
			{
				m_RoadConstructionEndCoordinate = { I.x, m_RoadConstructionStartCoordinate.y, I.z };
				m_RoadConstructionEndSnappedType = -1;
				bool snapped = false;
				float snapDist = roadPrefabWidth / 100.0f;
				float roadPrefabLengthInTheScene = roadPrefabLength / 100.0f;

				for (Junction* junction : m_Junctions)
				{
					glm::vec3 Intersection = m_Parent->RayPlaneIntersection(camPos, forward, junction->position, { 0.0f, 1.0f, 0.0f, });

					float distance = glm::length(junction->position - Intersection);
					if (distance < snapDist)
					{
						snapped = true;
						m_RoadConstructionEndCoordinate.x = junction->position.x;
						m_RoadConstructionEndCoordinate.z = junction->position.z;
						m_RoadConstructionEndSnappedType = 0;
						m_RoadConstructionEndSnappedJunction = junction;
						break;
					}
				}

				if (!snapped)
				{
					for (End* end : m_Ends)
					{
						glm::vec3 Intersection = m_Parent->RayPlaneIntersection(camPos, forward, end->position, { 0.0f, 1.0f, 0.0f, });

						float distance = glm::length(end->position - Intersection);
						if (distance < snapDist)
						{
							snapped = true;
							m_RoadConstructionEndCoordinate.x = end->position.x;
							m_RoadConstructionEndCoordinate.z = end->position.z;
							m_RoadConstructionEndSnappedType = 1;
							m_RoadConstructionEndSnappedEnd = end;
							break;
						}
					}
				}

				if (!snapped)
				{
					for (Road* road : m_Roads)
					{
						// glm::vec3 roadDir = glm::normalize(roadVec);
						// B.y += roadDir.y*c;
						road->object->isEnabled = true;
						glm::vec3 Intersection = m_Parent->RayPlaneIntersection(camPos, forward, road->startPos, { 0.0f, 1.0f, 0.0f, });

						glm::vec3 roadVec = road->endPos - road->startPos;
						float roadLength = glm::length(roadVec);

						glm::vec3 B = Intersection - road->startPos;
						float bLength = glm::length(B);

						float angle = glm::acos(glm::dot(roadVec, B) / (roadLength * bLength));
						float distance = bLength * glm::sin(angle);

						if (distance < snapDist)
						{
							float c = bLength * glm::cos(angle);
							if (c <= roadPrefabLengthInTheScene || c >= roadLength - roadPrefabLengthInTheScene)
								continue;

							snapped = true;
							glm::vec3 temp = road->startPos + glm::normalize(roadVec) * c;
							m_RoadConstructionEndCoordinate.x = temp.x;
							m_RoadConstructionEndCoordinate.z = temp.z;
							m_RoadConstructionEndSnappedType = 2;
							m_RoadConstructionEndSnappedRoad = road;
							break;
						}
					}
				}
				b_RoadConstructionEndSnapped = snapped;

				glm::vec3 AB = m_RoadConstructionEndCoordinate - m_RoadConstructionStartCoordinate;
				glm::vec3 normalizedAB = glm::normalize(AB);

				float rotationOffset = AB.x < 0 ? 180.0f : 0.0f;
				float rotationStart = glm::radians(glm::degrees(glm::atan(-AB.z / AB.x)) + 90 + rotationOffset);
				float rotationEnd = glm::radians(glm::degrees(glm::atan(-AB.z / AB.x)) - 90 + rotationOffset);

				m_RoadGuidelinesStart->isEnabled = !b_RoadConstructionStartSnapped;
				m_RoadGuidelinesEnd->isEnabled = !b_RoadConstructionEndSnapped;
				m_Parent->SetTransform(m_RoadGuidelinesStart, m_RoadConstructionStartCoordinate + glm::vec3{ 0.0f, 0.015f, 0.0f }, { 0.01f, 0.01f, 0.01f }, { 0.0f, rotationStart, 0.0f });
				m_Parent->SetTransform(m_RoadGuidelinesEnd, m_RoadConstructionEndCoordinate + glm::vec3{ 0.0f, 0.015f, 0.0f }, { 0.01f, 0.01f, 0.01f }, { 0.0f, rotationEnd, 0.0f });

				float availableABLength = (
					glm::length(AB)
					- (b_RoadConstructionStartSnapped ? (roadPrefabLength / 100.0f) : 0.0f)
					- (b_RoadConstructionEndSnapped ? (roadPrefabLength / 100.0f) : 0.0f)
					);
				availableABLength = std::max(availableABLength, 0.0f);

				int countAB = availableABLength / (roadPrefabLength / 100.0f);
				float scaleAB = (availableABLength / (roadPrefabLength / 100.0f)) / countAB;
				float scaledRoadLength = availableABLength / countAB;

				int discountStart = (b_RoadConstructionStartSnapped ? 1 : 0);

				int sum = countAB;
				if (sum > m_RoadGuidelines.size())
				{
					for (size_t j = m_RoadGuidelines.size(); j < sum; j++)
					{
						auto road = m_Parent->UploadObject("assets/objects/road.obj", "assets/shaders/Object.glsl", "assets/objects/road.png");
						m_RoadGuidelines.push_back(road);
					}
				}

				for (size_t j = 0; j < countAB; j++)
				{
					auto& roadG = m_RoadGuidelines[j];
					roadG->isEnabled = true;
					m_Parent->SetTransform(
						roadG,
						glm::vec3{
							m_RoadConstructionStartCoordinate.x + normalizedAB.x * (j + 0.5f + discountStart) * scaledRoadLength,
							m_RoadConstructionStartCoordinate.y,
							m_RoadConstructionStartCoordinate.z + normalizedAB.z * (j + 0.5f + discountStart) * scaledRoadLength
						} + glm::vec3{ 0.0f, 0.015f, 0.0f },
						{ 0.01f, 0.01f, 0.01f * scaleAB },
						{ 0.0f, rotationStart, 0.0f }
					);
				}

				if (m_RoadConstructionStartSnappedType == 2)
				{
					m_RoadConstructionStartSnappedRoad->object->isEnabled = false;
					glm::vec3 R0I = m_RoadConstructionStartCoordinate - m_RoadConstructionStartSnappedRoad->startPos;
					glm::vec3 R1I = m_RoadConstructionStartCoordinate - m_RoadConstructionStartSnappedRoad->endPos;

					glm::vec3 normalizedR0I = glm::normalize(R0I);
					glm::vec3 normalizedR1I = glm::normalize(R1I);

					float rotationOffset = R0I.z < 0 ? 180.0f : 0.0f;
					float roatationR0I = glm::radians(glm::degrees(glm::atan(-R0I.z / R0I.x)) + 90.0f + rotationOffset);
					float roatationR1I = glm::radians(glm::degrees(glm::atan(-R0I.z / R0I.x)) - 90.0f + rotationOffset);

					float availableR0ILength = glm::length(R0I) - (roadPrefabLength / 100.0f);
					float availableR1ILength = glm::length(R1I) - (roadPrefabLength / 100.0f);
					availableR0ILength = std::max(availableR0ILength, 0.0f);
					availableR1ILength = std::max(availableR1ILength, 0.0f);

					int countR0I = availableR0ILength / (roadPrefabLength / 100.0f);
					int countR1I = availableR1ILength / (roadPrefabLength / 100.0f);

					float scaleR0I = (availableR0ILength / (roadPrefabLength / 100.0f)) / countR0I;
					float scaleR1I = (availableR1ILength / (roadPrefabLength / 100.0f)) / countR1I;

					float scaledR0IRoadLength = availableR0ILength / countR0I;
					float scaledR1IRoadLength = availableR1ILength / countR1I;


					sum += countR0I;
					sum += countR1I;
					if (sum > m_RoadGuidelines.size())
					{
						for (size_t j = m_RoadGuidelines.size(); j < sum; j++)
						{
							auto road = m_Parent->UploadObject("assets/objects/road.obj", "assets/shaders/Object.glsl", "assets/objects/road.png");
							m_RoadGuidelines.push_back(road);
						}
					}

					for (size_t j = 0; j < countR0I; j++)
					{
						auto& roadG = m_RoadGuidelines[j + countAB];
						roadG->isEnabled = true;
						m_Parent->SetTransform(
							roadG,
							glm::vec3{
								m_RoadConstructionStartSnappedRoad->startPos.x + normalizedR0I.x * (j + 0.5f) * scaledR0IRoadLength,
								m_RoadConstructionStartSnappedRoad->startPos.y,
								m_RoadConstructionStartSnappedRoad->startPos.z + normalizedR0I.z * (j + 0.5f) * scaledR0IRoadLength
							} + glm::vec3{ 0.0f, 0.015f, 0.0f },
							{ 0.01f, 0.01f, 0.01f * scaleR0I },
							{ 0.0f, roatationR0I, 0.0f }
						);
					}

					for (size_t j = 0; j < countR1I; j++)
					{
						auto& roadG = m_RoadGuidelines[j + countAB + countR0I];
						roadG->isEnabled = true;
						m_Parent->SetTransform(
							roadG,
							glm::vec3{
								m_RoadConstructionStartSnappedRoad->endPos.x + normalizedR1I.x * (j + 0.5f) * scaledR1IRoadLength,
								m_RoadConstructionStartSnappedRoad->endPos.y,
								m_RoadConstructionStartSnappedRoad->endPos.z + normalizedR1I.z * (j + 0.5f) * scaledR1IRoadLength
							} + glm::vec3{ 0.0f, 0.015f, 0.0f },
							{ 0.01f, 0.01f, 0.01f * scaleR1I },
							{ 0.0f, roatationR1I, 0.0f }
						);
					}
				}

				if (m_RoadConstructionEndSnappedType == 2)
				{
					m_RoadConstructionEndSnappedRoad->object->isEnabled = false;
					glm::vec3 R0I = m_RoadConstructionEndCoordinate - m_RoadConstructionEndSnappedRoad->startPos;
					glm::vec3 R1I = m_RoadConstructionEndCoordinate - m_RoadConstructionEndSnappedRoad->endPos;

					glm::vec3 normalizedR0I = glm::normalize(R0I);
					glm::vec3 normalizedR1I = glm::normalize(R1I);

					float rotationOffset = R0I.z < 0 ? 180.0f : 0.0f;
					float roatationR0I = glm::radians(glm::degrees(glm::atan(-R0I.z / R0I.x)) + 90.0f + rotationOffset);
					float roatationR1I = glm::radians(glm::degrees(glm::atan(-R0I.z / R0I.x)) - 90.0f + rotationOffset);

					float availableR0ILength = glm::length(R0I) - (roadPrefabLength / 100.0f);
					float availableR1ILength = glm::length(R1I) - (roadPrefabLength / 100.0f);
					availableR0ILength = std::max(availableR0ILength, 0.0f);
					availableR1ILength = std::max(availableR1ILength, 0.0f);

					int countR0I = availableR0ILength / (roadPrefabLength / 100.0f);
					int countR1I = availableR1ILength / (roadPrefabLength / 100.0f);

					float scaleR0I = (availableR0ILength / (roadPrefabLength / 100.0f)) / countR0I;
					float scaleR1I = (availableR1ILength / (roadPrefabLength / 100.0f)) / countR1I;

					float scaledR0IRoadLength = availableR0ILength / countR0I;
					float scaledR1IRoadLength = availableR1ILength / countR1I;


					sum += countR0I;
					sum += countR1I;
					if (sum > m_RoadGuidelines.size())
					{
						for (size_t j = m_RoadGuidelines.size(); j < sum; j++)
						{
							auto road = m_Parent->UploadObject("assets/objects/road.obj", "assets/shaders/Object.glsl", "assets/objects/road.png");
							m_RoadGuidelines.push_back(road);
						}
					}

					for (size_t j = 0; j < countR0I; j++)
					{
						auto& roadG = m_RoadGuidelines[j + countAB];
						roadG->isEnabled = true;
						m_Parent->SetTransform(
							roadG,
							glm::vec3{
								m_RoadConstructionEndSnappedRoad->startPos.x + normalizedR0I.x * (j + 0.5f) * scaledR0IRoadLength,
								m_RoadConstructionEndSnappedRoad->startPos.y,
								m_RoadConstructionEndSnappedRoad->startPos.z + normalizedR0I.z * (j + 0.5f) * scaledR0IRoadLength
							} + glm::vec3{ 0.0f, 0.015f, 0.0f },
							{ 0.01f, 0.01f, 0.01f * scaleR0I },
							{ 0.0f, roatationR0I, 0.0f }
						);
					}

					for (size_t j = 0; j < countR1I; j++)
					{
						auto& roadG = m_RoadGuidelines[j + countAB + countR0I];
						roadG->isEnabled = true;
						m_Parent->SetTransform(
							roadG,
							glm::vec3{
								m_RoadConstructionEndSnappedRoad->endPos.x + normalizedR1I.x * (j + 0.5f) * scaledR1IRoadLength,
								m_RoadConstructionEndSnappedRoad->endPos.y,
								m_RoadConstructionEndSnappedRoad->endPos.z + normalizedR1I.z * (j + 0.5f) * scaledR1IRoadLength
							} + glm::vec3{ 0.0f, 0.015f, 0.0f },
							{ 0.01f, 0.01f, 0.01f * scaleR1I },
							{ 0.0f, roatationR1I, 0.0f }
						);
					}
				}

				for (size_t j = sum; j < m_RoadGuidelines.size(); j++)
				{
					auto& road = m_RoadGuidelines[j];
					road->isEnabled = false;
				}

			}
		}

		Can::Renderer3D::BeginScene(m_MainCameraController.GetCamera());

		Can::Renderer3D::DrawObjects();

		Can::Renderer3D::EndScene();
	}

	void TestScene::OnEvent(Can::Event::Event& event)
	{
		m_MainCameraController.OnEvent(event);
		Can::Event::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Can::Event::MouseButtonPressedEvent>(CAN_BIND_EVENT_FN(TestScene::OnMousePressed));
	}

	bool TestScene::OnMousePressed(Can::Event::MouseButtonPressedEvent& event)
	{
		Can::Object* terrain = m_Parent->m_Terrain;

		glm::vec3 camPos = m_MainCameraController.GetCamera().GetPosition();
		glm::vec3 forward = GetRayCastedFromScreen();

		float* data = terrain->Vertices;
		bool willBreak = false;

		// Find better way
		for (size_t y = 0; y < terrain->h - 1; y++)
		{
			for (size_t x = 0; x < terrain->w - 1; x++)
			{
				for (size_t z = 0; z < 2; z++)
				{
					size_t index = (x + (terrain->w - 1) * y) * 60 + z * 30;
					float* A = &data[index + 0];
					float* B = &data[index + 10];
					float* C = &data[index + 20];
					bool result = m_Parent->RayTriangleIntersection(
						camPos,
						forward,
						{ A[0], A[1], A[2] },
						{ B[0], B[1], B[2] },
						{ C[0], C[1], C[2] },
						{ A[7], A[8], A[9] }
					);
					if (result)
					{
						// Will be deleted //
						A[3] = 1.0f;	   //
						A[4] = 1.0f;	   //
						A[5] = 1.0f;	   //
						B[3] = 1.0f;	   //
						B[4] = 1.0f;	   //
						B[5] = 1.0f;	   //
						C[3] = 1.0f;	   //
						C[4] = 1.0f;	   //
						C[5] = 1.0f;	   //
						/////////////////////

						if (!b_RoadConstructionStarted)
						{
							b_RoadConstructionStarted = true;
							if (!b_RoadConstructionStartSnapped)
							{
								m_RoadConstructionStartCoordinate = glm::vec3{
								(A[0] + B[0] + C[0]) / 3.0f,
								A[1],
								(A[2] + B[2] + C[2]) / 3.0f
								};
							}
						}
						else
						{
							b_RoadConstructionEnded = true;
							if (!b_RoadConstructionEndSnapped)
							{
								m_RoadConstructionEndCoordinate = glm::vec3{
								(A[0] + B[0] + C[0]) / 3.0f,
								A[1],
								(A[2] + B[2] + C[2]) / 3.0f
								};
							}
							for (Can::Object* roadG : m_RoadGuidelines)
							{
								roadG->isEnabled = false;
							}
							for (Can::Object* junctionG : m_JunctionGuidelines)
							{
								junctionG->isEnabled = false;
							}
						}

						int vertexCount = terrain->indexCount * (3 + 4 + 3);
						terrain->VB->Bind();
						terrain->VB->ReDo(terrain->Vertices, sizeof(float) * vertexCount);
						terrain->VB->Unbind();
						willBreak = true;
						break;
					}
				}
				if (willBreak)
					break;
			}
			if (willBreak)
				break;
		}

		if (b_RoadConstructionEnded) //[[unlikely]]
		{
			Road* newRoad = new Road();
			newRoad->startPos = m_RoadConstructionStartCoordinate;
			newRoad->endPos = m_RoadConstructionEndCoordinate;
			m_Parent->ReconstructRoad(newRoad, roadPrefab, "assets/shaders/Object.glsl", "assets/objects/road.png");
			m_Roads.push_back(newRoad);

			if (m_RoadConstructionStartSnappedType == 0)
			{
				newRoad->startJunction = m_RoadConstructionStartSnappedJunction;
				m_RoadConstructionStartSnappedJunction->connectedRoads.push_back(newRoad);
				m_Parent->UpdateTheJunction(m_RoadConstructionStartSnappedJunction, JunctionPrefab, "assets/shaders/Object.glsl", "assets/objects/road_junction.png");
			}
			else if (m_RoadConstructionStartSnappedType == 1)
			{
				if (m_RoadConstructionStartSnappedEnd->connectedRoad->startEnd == m_RoadConstructionStartSnappedEnd)
					m_RoadConstructionStartSnappedEnd->connectedRoad->startEnd = nullptr;
				else
					m_RoadConstructionStartSnappedEnd->connectedRoad->endEnd = nullptr;

				Junction* newJunction = new Junction();
				newJunction->position = m_RoadConstructionStartSnappedEnd->position;
				newJunction->connectedRoads.push_back(m_RoadConstructionStartSnappedEnd->connectedRoad);
				newJunction->connectedRoads.push_back(newRoad);
				newRoad->startJunction = newJunction;

				if (m_RoadConstructionStartSnappedEnd->connectedRoad->startPos == m_RoadConstructionStartSnappedEnd->position)
					m_RoadConstructionStartSnappedEnd->connectedRoad->startJunction = newJunction;
				else
					m_RoadConstructionStartSnappedEnd->connectedRoad->endJunction = newJunction;


				auto position = std::find(m_Ends.begin(), m_Ends.end(), m_RoadConstructionStartSnappedEnd);
				m_Ends.erase(position);
				Can::Renderer3D::DeleteObject(m_RoadConstructionStartSnappedEnd->object);
				delete m_RoadConstructionStartSnappedEnd->object;
				delete m_RoadConstructionStartSnappedEnd;

				m_Parent->UpdateTheJunction(newJunction, JunctionPrefab, "assets/shaders/Object.glsl", "assets/objects/road_junction.png");
				m_Junctions.push_back(newJunction);
			}
			else if (m_RoadConstructionStartSnappedType == 2)
			{
				Junction* newJunction = new Junction();
				newJunction->position = m_RoadConstructionStartCoordinate;
				m_Junctions.push_back(newJunction);
				newJunction->connectedRoads.push_back(newRoad);
				newRoad->startJunction = newJunction;

				Road* r1 = new Road();
				r1->startPos = m_RoadConstructionStartSnappedRoad->startPos;
				r1->endPos = m_RoadConstructionStartCoordinate;
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
				r1->endJunction = newJunction;
				m_Roads.push_back(r1);
				newJunction->connectedRoads.push_back(r1);


				Road* r2 = new Road();
				r2->startPos = m_RoadConstructionStartSnappedRoad->endPos;
				r2->endPos = m_RoadConstructionStartCoordinate;
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
				r2->endJunction = newJunction;
				m_Roads.push_back(r2);
				newJunction->connectedRoads.push_back(r2);

				auto it = std::find(m_Roads.begin(), m_Roads.end(), m_RoadConstructionStartSnappedRoad);
				m_Roads.erase(it);
				Can::Renderer3D::DeleteObject(m_RoadConstructionStartSnappedRoad->object);
				delete m_RoadConstructionStartSnappedRoad->object;
				delete m_RoadConstructionStartSnappedRoad;

				m_Parent->UpdateTheJunction(newJunction, JunctionPrefab, "assets/shaders/Object.glsl", "assets/objects/road_junction.png");
			}
			else
			{
				End* newEnd = new End();
				newEnd->position = m_RoadConstructionStartCoordinate;
				newEnd->object = m_Parent->UploadObject("assets/objects/road_end.obj", "assets/shaders/Object.glsl", "assets/objects/road_end.png");
				newEnd->connectedRoad = newRoad;
				newRoad->startEnd = newEnd;

				glm::vec2 AB = {
					m_RoadConstructionEndCoordinate.x - m_RoadConstructionStartCoordinate.x,
					m_RoadConstructionEndCoordinate.z - m_RoadConstructionStartCoordinate.z
				};
				int edA = AB.x <= 0 ? 180.0f : 0.0f;
				float angle = glm::degrees(glm::atan(-AB.y / AB.x)) + 90.0f + edA;

				m_Parent->SetTransform(newEnd->object, newEnd->position + glm::vec3{ 0.0f, 0.01f, 0.0f }, { 0.01f, 0.01f, 0.01f }, { 0.0f, glm::radians(angle), 0.0f });

				m_Ends.push_back(newEnd);
			}

			if (m_RoadConstructionEndSnappedType == 0)
			{
				newRoad->endJunction = m_RoadConstructionEndSnappedJunction;
				m_RoadConstructionEndSnappedJunction->connectedRoads.push_back(newRoad);
				m_Parent->UpdateTheJunction(m_RoadConstructionEndSnappedJunction, JunctionPrefab, "assets/shaders/Object.glsl", "assets/objects/road_junction.png");
			}
			else if (m_RoadConstructionEndSnappedType == 1)
			{
				if (m_RoadConstructionEndSnappedEnd->connectedRoad->startEnd == m_RoadConstructionEndSnappedEnd)
					m_RoadConstructionEndSnappedEnd->connectedRoad->startEnd = nullptr;
				else
					m_RoadConstructionEndSnappedEnd->connectedRoad->endEnd = nullptr;


				Junction* newJunction = new Junction();
				newJunction->position = m_RoadConstructionEndSnappedEnd->position;
				newJunction->connectedRoads.push_back(m_RoadConstructionEndSnappedEnd->connectedRoad);
				newJunction->connectedRoads.push_back(newRoad);
				newRoad->endJunction = newJunction;

				if (m_RoadConstructionEndSnappedEnd->connectedRoad->startPos == m_RoadConstructionEndSnappedEnd->position)
					m_RoadConstructionEndSnappedEnd->connectedRoad->startJunction = newJunction;
				else
					m_RoadConstructionEndSnappedEnd->connectedRoad->endJunction = newJunction;


				auto position = std::find(m_Ends.begin(), m_Ends.end(), m_RoadConstructionEndSnappedEnd);
				m_Ends.erase(position);
				Can::Renderer3D::DeleteObject(m_RoadConstructionEndSnappedEnd->object);
				delete m_RoadConstructionEndSnappedEnd->object;
				delete m_RoadConstructionEndSnappedEnd;

				m_Parent->UpdateTheJunction(newJunction, JunctionPrefab, "assets/shaders/Object.glsl", "assets/objects/road_junction.png");
				m_Junctions.push_back(newJunction);
			}
			else if (m_RoadConstructionEndSnappedType == 2)
			{
				Junction* newJunction = new Junction();
				newJunction->position = m_RoadConstructionEndCoordinate;
				m_Junctions.push_back(newJunction);
				newJunction->connectedRoads.push_back(newRoad);
				newRoad->endJunction = newJunction;

				Road* r1 = new Road();
				r1->startPos = m_RoadConstructionEndSnappedRoad->startPos;
				r1->endPos = m_RoadConstructionEndCoordinate;
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
				r1->endJunction = newJunction;
				m_Roads.push_back(r1);
				newJunction->connectedRoads.push_back(r1);


				Road* r2 = new Road();
				r2->startPos = m_RoadConstructionEndSnappedRoad->endPos;
				r2->endPos = m_RoadConstructionEndCoordinate;
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
				r2->endJunction = newJunction;
				m_Roads.push_back(r2);
				newJunction->connectedRoads.push_back(r2);

				auto it = std::find(m_Roads.begin(), m_Roads.end(), m_RoadConstructionEndSnappedRoad);
				m_Roads.erase(it);
				Can::Renderer3D::DeleteObject(m_RoadConstructionEndSnappedRoad->object);
				delete m_RoadConstructionEndSnappedRoad->object;
				delete m_RoadConstructionEndSnappedRoad;

				m_Parent->UpdateTheJunction(newJunction, JunctionPrefab, "assets/shaders/Object.glsl", "assets/objects/road_junction.png");
			}
			else
			{
				End* newEnd = new End();
				newEnd->position = m_RoadConstructionEndCoordinate;
				newEnd->object = m_Parent->UploadObject("assets/objects/road_end.obj", "assets/shaders/Object.glsl", "assets/objects/road_end.png");
				newEnd->connectedRoad = newRoad;
				newRoad->endEnd = newEnd;

				glm::vec2 AB = {
					m_RoadConstructionStartCoordinate.x - m_RoadConstructionEndCoordinate.x,
					m_RoadConstructionStartCoordinate.z - m_RoadConstructionEndCoordinate.z
				};
				int edA = AB.x <= 0 ? 180.0f : 0.0f;
				float angle = glm::degrees(glm::atan(-AB.y / AB.x)) + 90.0f + edA;

				m_Parent->SetTransform(newEnd->object, newEnd->position + glm::vec3{ 0.0f, 0.01f, 0.0f }, { 0.01f, 0.01f, 0.01f }, { 0.0f, glm::radians(angle), 0.0f });

				m_Ends.push_back(newEnd);
			}
			glm::vec2 startIndex = { (int)std::abs(newRoad->startPos.x * 50) ,(int)std::abs(newRoad->startPos.z * 50) };
			glm::vec2 endIndex = { (int)std::abs(newRoad->endPos.x * 50) ,(int)std::abs(newRoad->endPos.z * 50) };
			m_Parent->LevelTheTerrain(startIndex, endIndex, newRoad->startPos, newRoad->endPos, m_Parent->m_Terrain, roadPrefabWidth);

			//reset EVERYTHING
			b_RoadConstructionStarted = false;
			b_RoadConstructionEnded = false;
			b_RoadConstructionStartSnapped = false;
			b_RoadConstructionEndSnapped = false;

			m_RoadConstructionStartSnappedJunction = nullptr;
			m_RoadConstructionStartSnappedEnd = nullptr;
			m_RoadConstructionStartSnappedRoad = nullptr;

			m_RoadConstructionEndSnappedJunction = nullptr;
			m_RoadConstructionEndSnappedEnd = nullptr;
			m_RoadConstructionEndSnappedRoad = nullptr;

			m_RoadConstructionStartSnappedType = -1;
			m_RoadConstructionEndSnappedType = -1;

			for (Can::Object* roadG : m_RoadGuidelines)
				roadG->isEnabled = false;

			for (Can::Object* junctionG : m_JunctionGuidelines)
				junctionG->isEnabled = false;

			m_RoadGuidelinesStart->isEnabled = true;
			m_RoadGuidelinesEnd->isEnabled = true;
		}

		return false;
	}

	glm::vec3 TestScene::GetRayCastedFromScreen()
	{
		auto [mouseX, mouseY] = Can::Input::GetMousePos();
		Application& app = Application::Get();
		float w = app.GetWindow().GetWidth();
		float h = app.GetWindow().GetHeight();

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
}
