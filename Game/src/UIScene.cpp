#include "canpch.h"
#include "UIScene.h"
#include "GameApp.h"
#include <glm/gtx/matrix_decompose.hpp>

namespace Can
{
	UIScene::UIScene(GameApp* parent)
		: m_Parent(parent)
		, m_ZoomLevel(10.0f)
		, m_AspectRatio(1280.0f / 720.0f)
		, m_CameraController(
			m_AspectRatio,
			m_ZoomLevel,
			false
		)
		, m_Scene(new Scene())
	{
		float width = m_AspectRatio * m_ZoomLevel * 2.0f;
		float height = m_ZoomLevel * 2.0f;

		m_ButtonRoads = new Button(
			m_Scene->m_Registry,
			glm::vec3{ 1.0f, height * (4.0f / 5.0f) - 1.6f, 0.0011f },
			glm::vec3{ 3.0f, 1.0f, 1.0f },
			glm::vec4{ 221.0f / 255.0f, 255.0f / 255.0f, 247.0f / 255.0f, 1.0f },
			[this, height]() {

				entt::registry& mainRegistry = this->m_Scene->m_Registry;

				entt::entity panelRoadsID = this->m_PanelRoads->entityID;
				entt::entity panelBuildingsID = this->m_PanelBuildings->entityID;
				entt::entity panelDebugID = this->m_PanelDebug->entityID;

				if (mainRegistry.has<HiddenComponent>(panelBuildingsID) && mainRegistry.has<HiddenComponent>(panelDebugID))
				{
					entt::entity buttonRoadsID = this->m_ButtonRoads->entityID;
					entt::entity buttonBuildingsID = this->m_ButtonBuildings->entityID;
					entt::entity buttonDebugID = this->m_ButtonDebug->entityID;
					TransformComponent& transformR = mainRegistry.get<TransformComponent>(buttonRoadsID);
					TransformComponent& transformB = mainRegistry.get<TransformComponent>(buttonBuildingsID);
					TransformComponent& transformD = mainRegistry.get<TransformComponent>(buttonDebugID);

					glm::vec3 movement = { 0.0f, height / 5.0f, 0.0f };

					if (mainRegistry.has<HiddenComponent>(panelRoadsID))
					{
						mainRegistry.remove<HiddenComponent>(panelRoadsID);
						transformR.Transform = glm::translate(transformR.Transform, -movement);
						transformB.Transform = glm::translate(transformB.Transform, -movement);
						transformD.Transform = glm::translate(transformD.Transform, -movement);
					}
					else
					{
						mainRegistry.emplace<HiddenComponent>(panelRoadsID);
						transformR.Transform = glm::translate(transformR.Transform, movement);
						transformB.Transform = glm::translate(transformB.Transform, movement);
						transformD.Transform = glm::translate(transformD.Transform, movement);
					}
				}
				else
				{
					if (mainRegistry.has<HiddenComponent>(panelRoadsID))
					{
						mainRegistry.remove<HiddenComponent>(panelRoadsID);
						mainRegistry.emplace_or_replace<HiddenComponent>(panelBuildingsID);
						mainRegistry.emplace_or_replace<HiddenComponent>(panelDebugID);
					}
					else
					{
						mainRegistry.emplace<HiddenComponent>(panelRoadsID);
					}
				}
			}
		);
		m_Scene->m_Registry.emplace<ParentComponent>(m_ButtonRoads->entityID, m_Scene->entityID);

		m_ButtonBuildings = new Button(
			m_Scene->m_Registry,
			glm::vec3{ 5.0f, height * (4.0f / 5.0f) - 1.6f, 0.0011f },
			glm::vec3{ 3.0f, 1.0f, 1.0f },
			glm::vec4{ 255.0f / 255.0f, 166.0f / 255.0f, 158.0f / 255.0f, 1.0f },
			[this, height]() {

				entt::registry& mainRegistry = this->m_Scene->m_Registry;

				entt::entity panelRoadsID = this->m_PanelRoads->entityID;
				entt::entity panelBuildingsID = this->m_PanelBuildings->entityID;
				entt::entity panelDebugID = this->m_PanelDebug->entityID;

				if (mainRegistry.has<HiddenComponent>(panelRoadsID) && mainRegistry.has<HiddenComponent>(panelDebugID))
				{
					entt::entity buttonRoadsID = this->m_ButtonRoads->entityID;
					entt::entity buttonBuildingsID = this->m_ButtonBuildings->entityID;
					entt::entity buttonDebugID = this->m_ButtonDebug->entityID;
					TransformComponent& transformR = mainRegistry.get<TransformComponent>(buttonRoadsID);
					TransformComponent& transformB = mainRegistry.get<TransformComponent>(buttonBuildingsID);
					TransformComponent& transformD = mainRegistry.get<TransformComponent>(buttonDebugID);

					glm::vec3 movement = { 0.0f, height / 5.0f, 0.0f };

					if (mainRegistry.has<HiddenComponent>(panelBuildingsID))
					{
						mainRegistry.remove<HiddenComponent>(panelBuildingsID);
						transformR.Transform = glm::translate(transformR.Transform, -movement);
						transformB.Transform = glm::translate(transformB.Transform, -movement);
						transformD.Transform = glm::translate(transformD.Transform, -movement);
					}
					else
					{
						mainRegistry.emplace<HiddenComponent>(panelBuildingsID);
						transformR.Transform = glm::translate(transformR.Transform, movement);
						transformB.Transform = glm::translate(transformB.Transform, movement);
						transformD.Transform = glm::translate(transformD.Transform, movement);
					}
				}
				else
				{
					if (mainRegistry.has<HiddenComponent>(panelBuildingsID))
					{
						mainRegistry.remove<HiddenComponent>(panelBuildingsID);
						mainRegistry.emplace_or_replace<HiddenComponent>(panelRoadsID);
						mainRegistry.emplace_or_replace<HiddenComponent>(panelDebugID);
					}
					else
					{
						mainRegistry.emplace<HiddenComponent>(panelBuildingsID);
					}
				}
			}
		);
		m_Scene->m_Registry.emplace<ParentComponent>(m_ButtonBuildings->entityID, m_Scene->entityID);

		m_ButtonDebug = new Button(
			m_Scene->m_Registry,
			glm::vec3{ 9.0f, height * (4.0f / 5.0f) - 1.6f, 0.0011f },
			glm::vec3{ 3.0f, 1.0f, 1.0f },
			glm::vec4{ 70.0f / 255.0f, 34.0f / 255.0f, 85.0f / 255.0f, 1.0f },
			[this, height]() {

				entt::registry& mainRegistry = this->m_Scene->m_Registry;

				entt::entity panelRoadsID = this->m_PanelRoads->entityID;
				entt::entity panelBuildingsID = this->m_PanelBuildings->entityID;
				entt::entity panelDebugID = this->m_PanelDebug->entityID;

				if (mainRegistry.has<HiddenComponent>(panelRoadsID) && mainRegistry.has<HiddenComponent>(panelBuildingsID))
				{
					entt::entity buttonRoadsID = this->m_ButtonRoads->entityID;
					entt::entity buttonBuildingsID = this->m_ButtonBuildings->entityID;
					entt::entity buttonDebugID = this->m_ButtonDebug->entityID;
					TransformComponent& transformR = mainRegistry.get<TransformComponent>(buttonRoadsID);
					TransformComponent& transformB = mainRegistry.get<TransformComponent>(buttonBuildingsID);
					TransformComponent& transformD = mainRegistry.get<TransformComponent>(buttonDebugID);

					glm::vec3 movement = { 0.0f, height / 5.0f, 0.0f };

					if (mainRegistry.has<HiddenComponent>(panelDebugID))
					{
						mainRegistry.remove<HiddenComponent>(panelDebugID);
						transformR.Transform = glm::translate(transformR.Transform, -movement);
						transformB.Transform = glm::translate(transformB.Transform, -movement);
						transformD.Transform = glm::translate(transformD.Transform, -movement);
					}
					else
					{
						mainRegistry.emplace<HiddenComponent>(panelDebugID);
						transformR.Transform = glm::translate(transformR.Transform, movement);
						transformB.Transform = glm::translate(transformB.Transform, movement);
						transformD.Transform = glm::translate(transformD.Transform, movement);
					}
				}
				else
				{
					if (mainRegistry.has<HiddenComponent>(panelDebugID))
					{
						mainRegistry.remove<HiddenComponent>(panelDebugID);
						mainRegistry.emplace_or_replace<HiddenComponent>(panelRoadsID);
						mainRegistry.emplace_or_replace<HiddenComponent>(panelBuildingsID);
					}
					else
					{
						mainRegistry.emplace<HiddenComponent>(panelDebugID);
					}
				}
			}
		);
		m_Scene->m_Registry.emplace<ParentComponent>(m_ButtonDebug->entityID, m_Scene->entityID);

		m_ButtonNeeds = new Button(
			m_Scene->m_Registry,
			glm::vec3{ 0.5f, 0.5f, 0.0011f },
			glm::vec3{ 1.0f, 1.0f, 1.0f },
			glm::vec4{ 170.0f / 255.0f, 68.0f / 255.0f, 101.0f / 255.0f, 1.0f },
			[this]() {

				entt::registry& mainRegistry = this->m_Scene->m_Registry;

				entt::entity panelNeedsID = this->m_PanelNeeds->entityID;
				entt::entity panelToolsID = this->m_PanelTools->entityID;

				if (mainRegistry.has<HiddenComponent>(panelNeedsID))
				{
					mainRegistry.remove<HiddenComponent>(panelNeedsID);
					mainRegistry.emplace_or_replace<HiddenComponent>(panelToolsID);
				}
				else
				{
					mainRegistry.emplace<HiddenComponent>(panelNeedsID);
				}
			}
		);
		m_Scene->m_Registry.emplace<ParentComponent>(m_ButtonNeeds->entityID, m_Scene->entityID);

		m_ButtonTools = new Button(
			m_Scene->m_Registry,
			glm::vec3{ 2.0f, 0.5f, 0.0011f },
			glm::vec3{ 1.0f, 1.0f, 1.0f },
			glm::vec4{ 147.0f / 255.0f, 225.0f / 255.0f, 216.0f / 255.0f, 1.0f },
			[this]() {

				entt::registry& mainRegistry = this->m_Scene->m_Registry;

				entt::entity panelNeedsID = this->m_PanelNeeds->entityID;
				entt::entity panelToolsID = this->m_PanelTools->entityID;

				if (mainRegistry.has<HiddenComponent>(panelToolsID))
				{
					mainRegistry.remove<HiddenComponent>(panelToolsID);
					mainRegistry.emplace_or_replace<HiddenComponent>(panelNeedsID);
				}
				else
				{
					mainRegistry.emplace<HiddenComponent>(panelToolsID);
				}
			}
		);
		m_Scene->m_Registry.emplace<ParentComponent>(m_ButtonTools->entityID, m_Scene->entityID);

		std::vector<entt::entity> buttonList = {
			 m_ButtonRoads->entityID,
			 m_ButtonBuildings->entityID,
			 m_ButtonDebug->entityID,
			 m_ButtonNeeds->entityID,
			 m_ButtonTools->entityID
		};
		m_Scene->m_Registry.emplace<ChildrenComponent>(m_Scene->entityID, buttonList);


		m_PanelRoads = new Panel(
			m_Scene->m_Registry,
			glm::vec3{ 0.5f, height * (4.0f / 5.0f) - 0.5f, 0.001f },
			glm::vec3{ width - 1.0f, height * (1.0f / 5.0f), 1.0f },
			glm::vec4{ 221.0f / 255.0f, 255.0f / 255.0f, 247.0f / 255.0f, 1.0f },
			[]() {std::cout << "You clicked the panel that is for Roads!" << std::endl; }
		);
		//m_Scene->m_Registry.emplace<HiddenComponent>(m_PanelRoads->entityID);
		m_Scene->m_Registry.emplace<ParentComponent>(m_PanelRoads->entityID, m_ButtonRoads->entityID);
		m_Scene->m_Registry.emplace<ChildrenComponent>(m_ButtonRoads->entityID, std::vector<entt::entity>{ m_PanelRoads->entityID });

		m_PanelBuildings = new Panel(
			m_Scene->m_Registry,
			glm::vec3{ 0.5f, height * (4.0f / 5.0f) - 0.5f, 0.001f },
			glm::vec3{ width - 1.0f, height * (1.0f / 5.0f), 1.0f },
			glm::vec4{ 255.0f / 255.0f, 166.0f / 255.0f, 158.0f / 255.0f, 1.0f },
			[]() {std::cout << "You clicked the panel that is for Buildings!" << std::endl; }
		);
		m_Scene->m_Registry.emplace<HiddenComponent>(m_PanelBuildings->entityID);
		m_Scene->m_Registry.emplace<ParentComponent>(m_PanelBuildings->entityID, m_ButtonBuildings->entityID);
		m_Scene->m_Registry.emplace<ChildrenComponent>(m_ButtonBuildings->entityID, std::vector<entt::entity>{ m_PanelBuildings->entityID });

		m_PanelDebug = new Panel(
			m_Scene->m_Registry,
			glm::vec3{ 0.5f, height * (4.0f / 5.0f) - 0.5f, 0.001f },
			glm::vec3{ width - 1.0f, height * (1.0f / 5.0f), 1.0f },
			glm::vec4{ 70.0f / 255.0f, 34.0f / 255.0f, 85.0f / 255.0f, 1.0f },
			[]() {std::cout << "You clicked the panel that is for the Debug!" << std::endl; }
		);
		m_Scene->m_Registry.emplace<HiddenComponent>(m_PanelDebug->entityID);
		m_Scene->m_Registry.emplace<ParentComponent>(m_PanelDebug->entityID, m_ButtonDebug->entityID);
		m_Scene->m_Registry.emplace<ChildrenComponent>(m_ButtonDebug->entityID, std::vector<entt::entity>{ m_PanelDebug->entityID });

		m_PanelNeeds = new Panel(
			m_Scene->m_Registry,
			glm::vec3{ 0.5f, 1.6f, 0.001f },
			glm::vec3{ width * (1.0f / 10.0f) - 0.5f, height * (3.0f / 5.0f) - 1.0f, 1.0f },
			glm::vec4{ 170.0f / 255.0f, 68.0f / 255.0f, 101.0f / 255.0f, 1.0f },
			[]() {std::cout << "You clicked the panel that is for Needs!" << std::endl; }
		);
		m_Scene->m_Registry.emplace<HiddenComponent>(m_PanelNeeds->entityID);
		m_Scene->m_Registry.emplace<ParentComponent>(m_PanelNeeds->entityID, m_ButtonNeeds->entityID);
		m_Scene->m_Registry.emplace<ChildrenComponent>(m_ButtonNeeds->entityID, std::vector<entt::entity>{ m_PanelNeeds->entityID });

		m_PanelTools = new Panel(
			m_Scene->m_Registry,
			glm::vec3{ 0.5f, 1.6f, 0.001f },
			glm::vec3{ width * (1.0f / 10.0f) - 0.5f, height * (3.0f / 5.0f) - 1.0f, 1.0f },
			glm::vec4{ 147.0f / 255.0f, 225.0f / 255.0f, 216.0f / 255.0f, 1.0f },
			[]() {std::cout << "You clicked the panel that is for Tools!" << std::endl; }
		);
		m_Scene->m_Registry.emplace<HiddenComponent>(m_PanelTools->entityID);
		m_Scene->m_Registry.emplace<ParentComponent>(m_PanelTools->entityID, m_ButtonTools->entityID);
		m_Scene->m_Registry.emplace<ChildrenComponent>(m_ButtonTools->entityID, std::vector<entt::entity>{ m_PanelTools->entityID });


	}

	UIScene::~UIScene()
	{
		delete m_Scene;

		delete m_ButtonRoads;
		delete  m_ButtonBuildings;
		delete  m_ButtonDebug;
		delete  m_ButtonNeeds;
		delete  m_ButtonTools;

		delete m_PanelRoads;
		delete m_PanelBuildings;
		delete m_PanelDebug;
		delete m_PanelNeeds;
		delete m_PanelTools;

	}

	void UIScene::OnUpdate(Can::TimeStep ts)
	{
		//Can::RenderCommand::SetClearColor({ 0.9f, 0.9f, 0.9f, 1.0f });
		//Can::RenderCommand::Clear();

		float widthHalf = m_AspectRatio * m_ZoomLevel;
		float heightHalf = m_ZoomLevel;

		Can::Renderer2D::BeginScene(m_CameraController.GetCamera());

		glm::vec2 offset = { -widthHalf, heightHalf };
		ChildrenComponent& children = m_Scene->m_Registry.get<ChildrenComponent>(m_Scene->entityID);
		for (auto entity : children)
			Draw(entity, &(m_Scene->m_Registry), offset);

		Can::Renderer2D::EndScene();
	}
	void UIScene::OnEvent(Can::Event::Event& event)
	{
		Can::Event::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Can::Event::MouseButtonPressedEvent>(CAN_BIND_EVENT_FN(UIScene::OnMousePressed));

	}
	bool UIScene::OnMousePressed(Can::Event::MouseButtonPressedEvent& event)
	{
		float cameraWidth = m_AspectRatio * m_ZoomLevel * 2.0f;
		float cameraHeight = m_ZoomLevel * 2.0f;

		float mouseX = Can::Input::GetMouseX();
		float mouseY = Can::Input::GetMouseY();
		Application& app = Application::Get();
		unsigned int w = app.GetWindow().GetWidth();
		unsigned int h = app.GetWindow().GetHeight();

		glm::vec2 clickPos = { cameraWidth * (mouseX / w), cameraHeight * (mouseY / h) };

		ChildrenComponent& children = m_Scene->m_Registry.get<ChildrenComponent>(m_Scene->entityID);

		for (auto it = children.end(); it != children.begin();)
		{
			event.m_Handled = CheckCollision((*--it), &(m_Scene->m_Registry), clickPos);
			if (event.m_Handled)
				return true;
		}
		return true;
	}

	bool CheckCollision(entt::entity id, entt::registry* registry, const glm::vec2& clickPosition)
	{
		if (registry->has<HiddenComponent>(id))
			return false;
		if (registry->has<TransformComponent, SpriteRendererComponent>(id) == false)
			return false;

		if (registry->has<ChildrenComponent>(id))
		{
			ChildrenComponent& children = registry->get<ChildrenComponent>(id);
			for (auto it = children.end(); it != children.begin();)
			{
				bool didColide = CheckCollision((*--it), registry, clickPosition);
				if (didColide)
					return true;
			}
		}


		auto& [transform, spriteRenderer] = registry->get< TransformComponent, SpriteRendererComponent>(id);

		glm::vec3 pos;
		glm::vec3 scale;
		glm::quat q;			// Don't care right now
		glm::vec3 skew;			// Don't care right now
		glm::vec4 perspective;	// Don't care right now
		glm::decompose(transform.Transform, scale, q, pos, skew, perspective);

		glm::vec2& size = spriteRenderer.size;
		glm::vec2 leftTop = {
			pos.x,
			pos.y
		};
		glm::vec2 rightBottom = {
			pos.x + size.x * scale.x,
			pos.y + size.y * scale.y
		};

		if (
			clickPosition.x < leftTop.x ||
			clickPosition.y < leftTop.y ||
			clickPosition.x > rightBottom.x ||
			clickPosition.y > rightBottom.y
			)
			return false;

		if (registry->has<OnClickCallbackComponent>(id))
			registry->get<OnClickCallbackComponent>(id)();

		return true;
	}

	void Draw(entt::entity id, entt::registry* registry, const glm::vec2& offset)
	{
		if (registry->has<HiddenComponent>(id))
			return;
		if (registry->has<TransformComponent, SpriteRendererComponent>(id) == false)
			return;

		if (registry->has<ChildrenComponent>(id))
		{
			ChildrenComponent& children = registry->get<ChildrenComponent>(id);

			for (auto entity : children)
				Draw(entity, registry, offset);
		}

		auto& [transform, spriteRenderer] = registry->get< TransformComponent, SpriteRendererComponent>(id);

		glm::vec3 pos;
		glm::vec3 scale;
		glm::quat q;			// Don't care right now
		glm::vec3 skew;			// Don't care right now
		glm::vec4 perspective;	// Don't care right now
		glm::decompose(transform.Transform, scale, q, pos, skew, perspective);

		scale.x *= spriteRenderer.size.x;
		scale.y *= spriteRenderer.size.y;

		pos.x = pos.x + offset.x + scale.x / 2.0f;
		pos.y = -1 * (pos.y - offset.y + scale.y / 2.0f);

		glm::mat4 newTransform = glm::translate(glm::mat4(1.0f), pos) * glm::scale(glm::mat4(1), scale);

		Can::Renderer2D::DrawQuad(pos - glm::vec3{ 0.0f, 0.0f, 0.00001f }, scale + glm::vec3{ 0.1f, 0.1f, 0.1f }, { 0.0f, 0.0f, 0.0f, 1.0f });
		if (spriteRenderer.texture)
			Can::Renderer2D::DrawQuad(newTransform, spriteRenderer.texture, spriteRenderer.color);
		else
			Can::Renderer2D::DrawQuad(newTransform, spriteRenderer.color);
	}
}