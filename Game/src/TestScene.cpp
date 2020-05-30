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
		m_RoadGuidelinesStart->isEnabled = false;
		m_RoadGuidelinesEnd->isEnabled = false;

		auto road = new Can::Object();
		road = m_Parent->UploadObject("assets/objects/road.obj", "assets/shaders/Object.glsl", "assets/objects/road.png");
		road->isEnabled = false;
		m_RoadGuidelines.push_back(road);
	}

	void TestScene::OnUpdate(Can::TimeStep ts)
	{
		m_MainCameraController.OnUpdate(ts);

		Can::RenderCommand::SetClearColor({ 0.9f, 0.9f, 0.9f, 1.0f });
		Can::RenderCommand::Clear();

		if (b_Start)
		{
			for (size_t i = 0; i < m_RoadGuidelines.size(); i++)
			{
				auto& road = m_RoadGuidelines[i];
				road->isEnabled = true;
			}
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
			if (isnan(I.x) == false)
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
				int c = 100 * mAB / l + 1;


				if (c > m_RoadGuidelines.size())
				{
					for (size_t i = m_RoadGuidelines.size(); i < c; i++)
					{
						auto road = new Can::Object();
						road = m_Parent->UploadObject("assets/objects/road.obj", "assets/shaders/Object.glsl", "assets/objects/road.png");
						m_RoadGuidelines.push_back(road);
					}
				}

				for (size_t i = 0; i < c; i++)
				{
					auto& road = m_RoadGuidelines[i];
					road->isEnabled == true;
					m_Parent->SetTransform(road, glm::vec3{ m_StartCoord.x + glm::normalize(AB).x * i * (l / 100.0f), m_StartCoord.y, m_StartCoord.z + glm::normalize(AB).z * i * (l / 100.0f) }, { 0.01f, 0.01f, 0.01f }, { 0.0f,glm::radians(glm::degrees(glm::atan(-AB.z / AB.x)) + 90),0.0f });
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
							m_RoadGuidelinesStart->isEnabled = false;
							m_RoadGuidelinesEnd->isEnabled = false;
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
							m_RoadGuidelinesStart->isEnabled = true;
							m_RoadGuidelinesEnd->isEnabled = true;
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
