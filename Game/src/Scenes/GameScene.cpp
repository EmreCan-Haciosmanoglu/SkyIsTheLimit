#include "canpch.h"
#include "GameScene.h"

#include "GameApp.h"
#include "Helper.h"
#include "Can/Math.h"

#include "Types/RoadNode.h"

namespace Can
{
	GameScene* GameScene::ActiveGameScene = nullptr;

	GameScene::GameScene(GameApp* application)
		: MainApplication(application)
		, m_Terrain(new Object(MainApplication->terrainPrefab))
		, m_RoadManager(this)
		, m_TreeManager(this)
		, m_BuildingManager(this)
		, m_CarManager(this)
		, m_MainCameraController(
			45.0f,
			16.0f / 9.0f,
			0.1f,
			1000.0f,
			glm::vec3{ 22.0f, 15.0f, -0.0f },
			glm::vec3{ -60.0f, 0.0f, 0.0f }
		)
	{
		m_Terrain->owns_prefab = true;
		m_LightDirection = glm::normalize(m_LightDirection);
		m_ShadowMapMasterRenderer = new ShadowMapMasterRenderer(&m_MainCameraController);
	}
	GameScene::~GameScene()
	{
	}
	void GameScene::OnAttach()
	{
		ActiveGameScene = this;
	}
	void GameScene::OnDetach()
	{
	}
	void GameScene::OnUpdate(TimeStep ts)
	{
		m_MainCameraController.OnUpdate(ts);

		RenderCommand::SetClearColor({ 0.9f, 0.9f, 0.9f, 1.0f });
		RenderCommand::Clear();

		glm::vec3 camPos = m_MainCameraController.GetCamera().GetPosition();
		glm::vec3 forward = GetRayCastedFromScreen();

		glm::vec3 I = Helper::RayPlaneIntersection(camPos, forward, { 0.0f, 0.0f, 0.0f, }, { 0.0f, 1.0f, 0.0f, });

		if (isnan(I.x) == false)
		{
			switch (e_ConstructionMode)
			{
			case ConstructionMode::Road:
				m_RoadManager.OnUpdate(I, ts);
				break;
			case ConstructionMode::Building:
				m_BuildingManager.OnUpdate(I, camPos, forward);
				break;
			case ConstructionMode::Tree:
				m_TreeManager.OnUpdate(I, camPos, forward);
				break;
			case ConstructionMode::Car:
				m_CarManager.OnUpdate(I, camPos, forward);
				break;
			case ConstructionMode::None:
				break;
			}
		}

		for (uint8_t i = 0; i < (uint8_t)e_SpeedMode; i++)
			MoveMe2AnotherFile(ts);


		Renderer3D::BeginScene(m_MainCameraController.GetCamera());
		m_ShadowMapMasterRenderer->Render(m_LightDirection);

		Renderer3D::DrawObjects(
			m_ShadowMapMasterRenderer->GetLS(),
			m_ShadowMapMasterRenderer->GetShadowMap(),
			m_MainCameraController.GetCamera(),
			m_LightPosition
		);

		Renderer3D::EndScene();
		//m_Framebuffer->Unbind();
	}
	void GameScene::OnEvent(Event::Event& event)
	{
		m_MainCameraController.OnEvent(event);
		Can::Event::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Can::Event::MouseButtonPressedEvent>(CAN_BIND_EVENT_FN(GameScene::OnMousePressed));
	}
	bool GameScene::OnMousePressed(Event::MouseButtonPressedEvent& event)
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

		switch (e_ConstructionMode)
		{
		case ConstructionMode::Road:
			m_RoadManager.OnMousePressed(button);
			break;
		case ConstructionMode::Building:
			m_BuildingManager.OnMousePressed(button);
			break;
		case ConstructionMode::Tree:
			m_TreeManager.OnMousePressed(button);
			break;
		case ConstructionMode::Car:
			m_CarManager.OnMousePressed(button);
			break;
		case ConstructionMode::None:
			break;
		}
		return false;
	}
	void GameScene::SetConstructionMode(ConstructionMode mode)
	{
		if (e_ConstructionMode == mode) return;
		e_ConstructionMode = mode;
		switch (e_ConstructionMode)
		{
		case ConstructionMode::Road:
			m_TreeManager.ResetStates();
			m_BuildingManager.ResetStates();
			m_CarManager.ResetStates();
			m_RoadManager.SetConstructionMode(m_RoadManager.GetConstructionMode());
			break;
		case ConstructionMode::Building:
			m_RoadManager.ResetStates();
			m_TreeManager.ResetStates();
			m_CarManager.ResetStates();
			m_BuildingManager.SetConstructionMode(m_BuildingManager.GetConstructionMode());
			break;
		case ConstructionMode::Tree:
			m_RoadManager.ResetStates();
			m_BuildingManager.ResetStates();
			m_CarManager.ResetStates();
			m_TreeManager.SetConstructionMode(m_TreeManager.GetConstructionMode());
			break;
		case ConstructionMode::Car:
			m_RoadManager.ResetStates();
			m_BuildingManager.ResetStates();
			m_TreeManager.ResetStates();
			m_CarManager.SetConstructionMode(m_CarManager.GetConstructionMode());
			break;
		case ConstructionMode::None:
			m_RoadManager.ResetStates();
			m_TreeManager.ResetStates();
			m_BuildingManager.ResetStates();
			m_CarManager.ResetStates();
			break;
		default:
			break;
		}
	}
	void GameScene::SetSpeedMode(SpeedMode mode)
	{
		// More Stuff???
		e_SpeedMode = mode;
	}
	glm::vec3 GameScene::GetRayCastedFromScreen()
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
	void GameScene::MoveMe2AnotherFile(float ts)
	{
		auto& cars = m_CarManager.GetCars();
		for (Car* car : cars)
		{
			glm::vec3 targeT = car->target;
			glm::vec3 pos = car->position;
			glm::vec3 ab = targeT - pos;
			glm::vec3 unit = glm::normalize(ab);
			float journeyLength = ts * car->speed;
			float leftLenght = glm::length(ab);
			RoadSegment& road = m_RoadManager.m_Segments[car->roadSegment];

			if (car->inJunction)
			{
				car->t += ts / 1.5f;
				glm::vec3 driftPos = Math::QuadraticCurve(car->driftpoints, car->t);
				glm::vec3 d1rection = glm::normalize(driftPos - car->position);
				car->position = driftPos;

				car->object->SetTransform(car->position, glm::vec3{
					0.0f,
					glm::radians(180.0f) + glm::acos(d1rection.x) * ((float)(d1rection.z < 0.0f) * 2.0f - 1.0f),
					0.0f });
				if (car->t >= 1.0f)
				{
					car->inJunction = false;
				}

			}
			else
			{
				if (journeyLength < leftLenght)
				{
					car->position = car->position + unit * journeyLength;
					car->object->SetTransform(car->position);
				}
				else
				{
					car->position = car->target;
					car->object->SetTransform(car->position);
					std::vector<float> ts{ 0 };
					float lengthRoad = road.type.road_length;
					std::vector<glm::vec3> samples = Math::GetCubicCurveSamples(road.GetCurvePoints(), lengthRoad, ts);

					if ((samples.size() - 2 == car->t_index && car->fromStart) || (1 == car->t_index && !car->fromStart))
					{
						//////new road
						u64 nodeIndex = car->fromStart ? road.EndNode : road.StartNode;
						RoadNode& node = m_RoadManager.m_Nodes[nodeIndex];
						if (node.roadSegments.size() > 1)
						{
							car->driftpoints[0] = car->position;
							car->driftpoints[1] = node.position;

							std::vector<u64>& roads = node.roadSegments;
							int newRoadIndex = Utility::Random::Integer((int)roads.size());
							RoadSegment& rs = m_RoadManager.m_Segments[car->roadSegment];

							while (car->roadSegment == roads[newRoadIndex])
							{
								newRoadIndex = Utility::Random::Integer((int)roads.size());
							}

							rs.Cars.erase(std::find(rs.Cars.begin(), rs.Cars.end(), car));
							car->roadSegment = roads[newRoadIndex];

							m_RoadManager.m_Segments[car->roadSegment].Cars.push_back(car);
							std::vector<float> ts2{ 0 };
							float lengthRoad2 = m_RoadManager.m_Segments[car->roadSegment].type.road_length;
							std::vector<glm::vec3> samples2 = Math::GetCubicCurveSamples(m_RoadManager.m_Segments[car->roadSegment].GetCurvePoints(), lengthRoad2, ts2);

							if (nodeIndex == m_RoadManager.m_Segments[car->roadSegment].StartNode)
							{
								car->t_index = 0;
								car->target = samples2[1];
								car->fromStart = true;
								car->driftpoints[2] = m_RoadManager.m_Segments[car->roadSegment].GetStartPosition();
							}
							else
							{
								car->t_index = samples2.size();
								car->target = samples2[samples2.size() - 1];
								car->fromStart = false;
								car->driftpoints[2] = m_RoadManager.m_Segments[car->roadSegment].GetEndPosition();
							}
							car->t = 0;
							car->inJunction = true;

						}
						else
						{
							if (!car->fromStart)
							{
								car->t_index = 0;
								car->target = samples[1];
								car->fromStart = true;
							}
							else
							{
								car->t_index = samples.size();
								car->target = samples[samples.size() - 1];
								car->fromStart = false;
							}
						}
					}
					else
					{
						glm::vec3 oldTarget = car->target;
						car->target = samples[car->t_index + (car->fromStart ? +1 : -1)];
						car->t_index += (car->fromStart ? +1 : -1);

						glm::vec3 d1rection = glm::normalize(car->target - oldTarget);
						car->object->SetTransform(car->position,glm::vec3{
							0.0f,
							glm::radians(180.0f) + glm::acos(d1rection.x) * ((float)(d1rection.z < 0.0f) * 2.0f - 1.0f),
							0.0f });
					}

				}
			}

		}
	}
}
