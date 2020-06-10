#include "canpch.h"
#include "TestScene.h"
#include "GameApp.h"

namespace Can
{
	TestScene::TestScene(GameApp* parent)
		: m_Parent(parent)
		, m_MainCameraController(
			45.0f,
			1920.0f / 1080.0f,
			0.0001f,
			100.0f,
			{ 1.0f, 0.5f, -1.0f },
			{ -45, 0, 0 }
		)
	{
		m_RoadGuidelinesStart = m_Parent->UploadObject("assets/objects/road_start.obj", "assets/shaders/Object.glsl", "assets/objects/road.png");
		m_RoadGuidelinesEnd = m_Parent->UploadObject("assets/objects/road_end.obj", "assets/shaders/Object.glsl", "assets/objects/road.png");
		m_RoadGuidelineTJunction = m_Parent->UploadObject("assets/objects/road_t_junction.obj", "assets/shaders/Object.glsl", "assets/objects/road_t_junction.png");
		m_RoadGuidelineTJunctionStart = m_Parent->UploadObject("assets/objects/road_start.obj", "assets/shaders/Object.glsl", "assets/objects/road.png");
		m_RoadGuidelineTJunctionEnd = m_Parent->UploadObject("assets/objects/road_end.obj", "assets/shaders/Object.glsl", "assets/objects/road.png");
		m_RoadGuidelineTJunction->isEnabled = false;
		m_RoadGuidelineTJunctionStart->isEnabled = false;
		m_RoadGuidelineTJunctionEnd->isEnabled = false;

		auto road = m_Parent->UploadObject("assets/objects/road.obj", "assets/shaders/Object.glsl", "assets/objects/road.png");
		road->isEnabled = false;
		m_RoadGuidelines.push_back(road);
	}

	void TestScene::OnUpdate(Can::TimeStep ts)
	{
		m_MainCameraController.OnUpdate(ts);

		Can::RenderCommand::SetClearColor({ 0.9f, 0.9f, 0.9f, 1.0f });
		Can::RenderCommand::Clear();

		/*Delete After TJunc finished or moved to the end of the TJunc building code*/
		for (size_t i = 0; i < m_Roads.size(); i++)
		{
			m_Roads[i].m_RoadObject->isEnabled = true;
		}
		m_RoadGuidelinesStart->isEnabled = true;
		m_RoadGuidelinesEnd->isEnabled = true;
		b_Snap = false;
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

		glm::vec3 I = m_Parent->RayPlaneIntersection(camPos, forward, { 0,0,0 }, { 0,1,0 });
		if (!b_Start)
		{
			if (isnan(I.x) == false)
			{
				m_Parent->SetTransform(m_RoadGuidelinesStart, I + glm::vec3{0.0f, 0.02f, 0.0f}, { 0.01f, 0.01f, 0.01f });
				m_Parent->SetTransform(m_RoadGuidelinesEnd, I + glm::vec3{ 0.0f, 0.02f, 0.0f }, { 0.01f, 0.01f, 0.01f });

			}
		}
		else
		{
			auto deleteMeInTheEnd = m_Parent->UploadObject("assets/objects/road.obj", "assets/shaders/Object.glsl", "assets/objects/road.png");

			
			if (isnan(I.x) == false)
			{
				for (size_t i = 0; i < m_Roads.size(); i++)
				{
					Road r = m_Roads[i];
					r.m_RoadObject->isEnabled = true;
					glm::vec3 A = r.endPoints[0] - r.endPoints[1];
					float a = glm::length(A);
					glm::vec3 B = { I.x - r.endPoints[1].x, r.endPoints[1].y, I.z - r.endPoints[1].z };
					float b = glm::length(B);
					float d = b * glm::sin(glm::acos(glm::dot(A, B) / (a * b)));

					if (d < 0.1f)
					{
						float c = b * glm::cos(glm::acos(glm::dot(A, B) / (a * b)));
						I = r.endPoints[1] + glm::normalize(A) * c;
						b_Snap = true;
						glm::vec3 V1 = r.endPoints[0] - r.endPoints[1];
						glm::vec3 V2 = m_StartCoord - r.endPoints[1];
						V1.y = 0;
						V2.y = 0;
						glm::vec3 VResult = glm::cross(V1, V2);
						bool s1 = VResult.y < 0.0f;
						bool s2 = A.x < 0;
						m_Parent->SetTransform(m_RoadGuidelineTJunction, I, { 0.01f, 0.01f ,0.01f }, { 0, glm::radians(glm::degrees(glm::atan(-A.z / A.x)) + 90 + (s1 ? 180 : 0) + (s2 ? 180 : 0)), 0.0f });
						r.m_RoadObject->isEnabled = false;
						float iMax = 0.0f;
						float iMin = 0.0f;
						for (size_t j = 0; j < deleteMeInTheEnd->indexCount; j++)
						{
							float z = deleteMeInTheEnd->Vertices[j * 8 + 2];
							iMax = std::max(iMax, z);
							iMin = std::min(iMin, z);
						}
						float iL = (iMax - iMin);

						float tMax = 0.0f;
						float tMin = 0.0f;
						for (size_t j = 0; j < m_RoadGuidelineTJunction->indexCount; j++)
						{
							float z = m_RoadGuidelineTJunction->Vertices[j * 8 + 2];
							tMax = std::max(tMax, z);
							tMin = std::min(tMin, z);
						}
						float tL = (tMax - tMin);

						m_RoadGuidelinesStart->isEnabled = false;
						m_RoadGuidelineTJunctionStart->isEnabled = true;
						m_RoadGuidelineTJunctionEnd->isEnabled = true;
						m_Parent->SetTransform(m_RoadGuidelineTJunctionStart, r.endPoints[1], { 0.01f, 0.01f ,0.01f }, { 0, glm::radians(glm::degrees(glm::atan(-A.z / A.x)) + 90 + (s2 ? 180 : 0)), 0.0f });
						m_Parent->SetTransform(m_RoadGuidelineTJunctionEnd, r.endPoints[0], { 0.01f, 0.01f ,0.01f }, { 0, glm::radians(glm::degrees(glm::atan(-A.z / A.x)) + 90 + (s2 ? 180 : 0)), 0.0f });

						glm::vec3 R0I = I - r.endPoints[0];
						glm::vec3 R1I = I - r.endPoints[1];
						glm::vec3 DI = I - m_StartCoord;

						float tOffset = (iL / 2.0f + tL / 2.0f);

						float lR0I = (glm::length(R0I) - (tOffset / 100.0f)) / (iL / 100.f);
						float lR1I = (glm::length(R1I) - (tOffset / 100.0f)) / (iL / 100.f);
						float lDI = (glm::length(DI) - (tOffset / 100.0f)) / (iL / 100.f);

						lR0I = std::max(lR0I, -1.0f);
						lR1I = std::max(lR1I, -1.0f);
						lDI = std::max(lDI, -1.0f);

						int cR0I = lR0I  + 1;
						int cR1I = lR1I + 1;
						int cDI = lDI + 1;

						float sR0I = (lR0I + 1) / cR0I;
						float sR1I = (lR1I + 1) / cR1I;
						float sDI = (lDI + 1) / cDI;

						int sum = cR0I + cR1I + cDI;
						if (sum > m_RoadGuidelines.size())
						{
							for (size_t j = m_RoadGuidelines.size(); j < sum; j++)
							{
								auto road = m_Parent->UploadObject("assets/objects/road.obj", "assets/shaders/Object.glsl", "assets/objects/road.png");
								m_RoadGuidelines.push_back(road);
							}
						}

						for (size_t j = 0; j < cR0I; j++)
						{
							auto& roadG = m_RoadGuidelines[j];
							roadG->isEnabled = true;
							m_Parent->SetTransform(roadG, glm::vec3{ r.endPoints[0].x + glm::normalize(R0I).x * j * sR0I * (iL / 100.0f), r.endPoints[0].y, r.endPoints[0].z + glm::normalize(R0I).z * j * sR0I * (iL / 100.0f) }, { 0.01f, 0.01f, 0.01f }, { 0.0f, glm::radians(glm::degrees(glm::atan(-R0I.z / R0I.x)) + 90), 0.0f });
						}

						for (size_t j = 0; j < cR1I; j++)
						{
							auto& roadG = m_RoadGuidelines[j + cR0I];
							roadG->isEnabled = true;
							m_Parent->SetTransform(roadG, glm::vec3{ r.endPoints[1].x + glm::normalize(R1I).x * j * sR1I * (iL / 100.0f), r.endPoints[1].y, r.endPoints[1].z + glm::normalize(R1I).z * j * sR1I * (iL / 100.0f) }, { 0.01f, 0.01f, 0.01f }, { 0.0f, glm::radians(glm::degrees(glm::atan(-R1I.z / R1I.x)) + 90), 0.0f });
						}

						for (size_t j = 0; j < cDI; j++)
						{
							auto& roadG = m_RoadGuidelines[j + cR1I + cR0I];
							roadG->isEnabled = true;
							m_Parent->SetTransform(roadG, glm::vec3{ m_StartCoord.x + glm::normalize(DI).x * j * sDI * (iL / 100.0f), m_StartCoord.y, m_StartCoord.z + glm::normalize(DI).z * j * sDI * (iL / 100.0f) }, { 0.01f, 0.01f, 0.01f }, { 0.0f, glm::radians(glm::degrees(glm::atan(-DI.z / DI.x)) + 90), 0.0f });
						}

						for (size_t j = sum; j < m_RoadGuidelines.size(); j++)
						{
							auto& road = m_RoadGuidelines[j];
							road->isEnabled = false;
						}
						/*
						Guide road
						3 end point
						1 junction
						*/
						break;
					}
				}
				m_RoadGuidelineTJunction->isEnabled = b_Snap;

				if (!b_Snap)
				{
					glm::vec3 AB = {
						I.x - m_StartCoord.x,
						m_StartCoord.y,
						I.z - m_StartCoord.z
					};

					float mAB = glm::length(AB);
					float max = 0.0f;
					float min = 0.0f;
					Can::Object* first = m_RoadGuidelines.at(0);
					for (size_t i = 0; i < first->indexCount; i++)
					{
						float z = first->Vertices[i * 8 + 2];
						max = std::max(max, z);
						min = std::min(min, z);
					}
					float l = (max - min);
					float c = 100.0f * mAB / l + 1;

					if (c > m_RoadGuidelines.size())
					{
						for (size_t i = m_RoadGuidelines.size(); i < c; i++)
						{
							auto road = m_Parent->UploadObject("assets/objects/road.obj", "assets/shaders/Object.glsl", "assets/objects/road.png");
							m_RoadGuidelines.push_back(road);
						}
					}
					float Scale = (c / (int)c);
					float xzRot = glm::radians(glm::degrees(glm::atan(-AB.z / AB.x)) + 90);
					for (size_t i = 0; i < c; i++)
					{
						auto& road = m_RoadGuidelines[i];
						road->isEnabled = true;
						m_Parent->SetTransform(road, glm::vec3{ m_StartCoord.x + glm::normalize(AB).x * i * Scale * (l / 100.0f), m_StartCoord.y, m_StartCoord.z + glm::normalize(AB).z * i * Scale * (l / 100.0f) }, { 0.01f, 0.01f, 0.01f }, { 0.0f, xzRot,0.0f });
					}
					for (size_t i = c; i < m_RoadGuidelines.size(); i++)
					{
						auto& road = m_RoadGuidelines[i];
						road->isEnabled = false;
					}
					int ed = AB.x > 0 ? 180 : 0;
					m_Parent->SetTransform(m_RoadGuidelinesStart, glm::vec3{ m_StartCoord.x + glm::normalize(AB).x * (c - 0.5f) * (l / 100.0f), m_StartCoord.y, m_StartCoord.z + glm::normalize(AB).z * (c - 0.5f) * (l / 100.0f) }, { 0.01f, 0.01f, 0.01f }, { 0.0f,glm::radians(glm::degrees(glm::atan(-AB.z / AB.x)) + 90 + ed),0.0f });
					m_Parent->SetTransform(m_RoadGuidelinesEnd, glm::vec3{ m_StartCoord.x + glm::normalize(AB).x * (-0.5f) * (l / 100.0f), m_StartCoord.y, m_StartCoord.z + glm::normalize(AB).z * (-0.5f) * (l / 100.0f) }, { 0.01f, 0.01f, 0.01f }, { 0.0f,glm::radians(glm::degrees(glm::atan(-AB.z / AB.x)) + 90 + ed),0.0f });

				}
			}
			Can::Renderer3D::DeleteObject(deleteMeInTheEnd);
			delete deleteMeInTheEnd;
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

		float* data = terrain->Vertices;
		bool willBreak = false;
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
						A[3] = 1;
						A[4] = 1;
						A[5] = 1;
						B[3] = 1;
						B[4] = 1;
						B[5] = 1;
						C[3] = 1;
						C[4] = 1;
						C[5] = 1;

						if (b_Start)
						{
							b_End = true;
							m_End = { x, y };
							m_EndCoord = {
								(A[0] + B[0] + C[0]) / 3.0f,
								m_StartCoord.y + 0.01f,
								(A[2] + B[2] + C[2]) / 3.0f
							};
							AB = {
								m_EndCoord.x - m_StartCoord.x,
								m_EndCoord.z - m_StartCoord.z
							};
							mAB = glm::length(AB);
							for (size_t i = 0; i < m_RoadGuidelines.size(); i++)
							{
								auto& road = m_RoadGuidelines[i];
								road->isEnabled = false;
							}
							m_RoadGuidelineTJunction->isEnabled = false;
							m_RoadGuidelineTJunctionStart->isEnabled = false;
							m_RoadGuidelineTJunctionEnd->isEnabled = false;

							for (size_t i = 0; i < m_Roads.size(); i++)
							{
								Road r = m_Roads[i];

								glm::vec3 A = r.endPoints[0] - r.endPoints[1];
								float a = glm::length(A);
								glm::vec3 B = { m_EndCoord.x - r.endPoints[1].x, r.endPoints[1].y, m_EndCoord.z - r.endPoints[1].z };
								float b = glm::length(B);
								float d = b * glm::sin(glm::acos(glm::dot(A, B) / (a * b)));

								if (d < 0.1f)
								{
									float c = b * glm::cos(glm::acos(glm::dot(A, B) / (a * b)));
									m_EndCoord = r.endPoints[1] + glm::normalize(A) * c;
									break;
								}
							}
						}
						else if (!b_End)
						{
							b_Start = true;
							m_Start = { x, y };
							m_StartCoord = {
								(A[0] + B[0] + C[0]) / 3.0f,
								A[1],
								(A[2] + B[2] + C[2]) / 3.0f
							};
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

		if (b_End)
		{
			if (b_Snap)
			{
				// Function for 3 Roads from 1 Road and 1 Junction position and 1 outside point
			}
			bool xC = m_Start.x < m_End.x;
			int xA = xC ? 1 : -1;
			bool yC = m_Start.y < m_End.y;
			int yA = yC ? 1 : -1;
			for (size_t y = m_Start.y; (y < m_End.y) == yC; y += yA)
			{
				for (size_t x = m_Start.x; (x < m_End.x) == xC; x += xA)
				{
					int dist1 = (x + (terrain->w - 1) * y) * 60;
					int dist2 = (x + (terrain->w - 1) * (y - 1)) * 60;
					glm::vec2 AP = {
						data[dist1] - m_StartCoord.x,
						data[dist1 + 2] - m_StartCoord.z
					};
					float mAP = glm::length(AP);
					float d = mAB * glm::sin(glm::acos(glm::dot(AB, AP) / (mAP * mAB)));

					if (d < 0.2f)
					{
						if (dist2 - 60 >= 0)
						{
							data[dist2 - 39] = m_StartCoord.y;
							data[dist2 - 19] = m_StartCoord.y;
							data[dist2 + 51] = m_StartCoord.y;
						}
						data[dist1 - 49] = m_StartCoord.y;
						data[dist1 + 1] = m_StartCoord.y;
						data[dist1 + 31] = m_StartCoord.y;
					}
				}
			}
			b_End = false;
			b_Start = false;
			int vertexCount = terrain->indexCount * (3 + 4 + 3);
			terrain->VB->Bind();
			terrain->VB->ReDo(terrain->Vertices, sizeof(float) * vertexCount);
			terrain->VB->Unbind();

			Can::Object* road = m_Parent->GenerateStraightRoad("assets/objects/road.obj", "assets/shaders/Object.glsl", "assets/objects/road.png", m_StartCoord, m_EndCoord);
			//m_Parent->UploadObject("assets/objects/road.obj", "assets/shaders/Object.glsl", "assets/objects/road.png");
			//m_Parent->SetTransform(road, (m_EndCoord + m_StartCoord) / 2.0f, { 0.03f, 0.03f, 0.03f }, { 0.0f,glm::radians(glm::degrees(glm::atan(-AB.y / AB.x)) + 90 ),0.0f });
			m_Roads.push_back(Road{
				std::array{ m_StartCoord, m_EndCoord },
				road
				});
		}

		return false;
	}
}
