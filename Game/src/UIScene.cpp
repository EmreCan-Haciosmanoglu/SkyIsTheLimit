#include "canpch.h"
#include "UIScene.h"
#include "GameApp.h"
#include "Helper.h"

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

		m_ButtonRoads = new Button(ButtonConstructorParameters{
			m_Scene->m_Registry,
			m_Scene->entityID,
			glm::vec3{ 1.0f, height * (4.0f / 5.0f) - 1.7f, 0.0011f },
			glm::vec3{ 3.0f, 1.0f, 1.0f },
			glm::vec4{ 221.0f / 255.0f, 255.0f / 255.0f, 247.0f / 255.0f, 1.0f },
			nullptr,
			[this, height]() {

				entt::registry& mainRegistry = this->m_Scene->m_Registry;

				entt::entity panelRoadsID = this->m_PanelRoads->entityID;
				entt::entity panelBuildingsID = this->m_PanelBuildings->entityID;

				if (mainRegistry.has<HiddenComponent>(panelBuildingsID))
				{
					entt::entity buttonRoadsID = this->m_ButtonRoads->entityID;
					entt::entity buttonBuildingsID = this->m_ButtonBuildings->entityID;
					TransformComponent& transformR = mainRegistry.get<TransformComponent>(buttonRoadsID);
					TransformComponent& transformB = mainRegistry.get<TransformComponent>(buttonBuildingsID);

					glm::vec3 movement = { 0.0f, height / 5.0f, 0.0f };

					if (mainRegistry.has<HiddenComponent>(panelRoadsID))
					{
						mainRegistry.remove<HiddenComponent>(panelRoadsID);
						transformR.Transform = glm::translate(transformR.Transform, -movement);
						transformB.Transform = glm::translate(transformB.Transform, -movement);
					}
					else
					{
						mainRegistry.emplace<HiddenComponent>(panelRoadsID);
						transformR.Transform = glm::translate(transformR.Transform, movement);
						transformB.Transform = glm::translate(transformB.Transform, movement);
					}
				}
				else
				{
					if (mainRegistry.has<HiddenComponent>(panelRoadsID))
					{
						mainRegistry.remove<HiddenComponent>(panelRoadsID);
						mainRegistry.emplace_or_replace<HiddenComponent>(panelBuildingsID);
					}
					else
					{
						mainRegistry.emplace<HiddenComponent>(panelRoadsID);
					}
				}
			}
			});

		m_ButtonBuildings = new Button(ButtonConstructorParameters{
			m_Scene->m_Registry,
			m_Scene->entityID,
			glm::vec3{ 5.0f, height * (4.0f / 5.0f) - 1.7f, 0.0011f },
			glm::vec3{ 3.0f, 1.0f, 1.0f },
			glm::vec4{ 255.0f / 255.0f, 166.0f / 255.0f, 158.0f / 255.0f, 1.0f },
			nullptr,
			[this, height]() {

				entt::registry& mainRegistry = this->m_Scene->m_Registry;

				entt::entity panelRoadsID = this->m_PanelRoads->entityID;
				entt::entity panelBuildingsID = this->m_PanelBuildings->entityID;

				if (mainRegistry.has<HiddenComponent>(panelRoadsID))
				{
					entt::entity buttonRoadsID = this->m_ButtonRoads->entityID;
					entt::entity buttonBuildingsID = this->m_ButtonBuildings->entityID;
					TransformComponent& transformR = mainRegistry.get<TransformComponent>(buttonRoadsID);
					TransformComponent& transformB = mainRegistry.get<TransformComponent>(buttonBuildingsID);

					glm::vec3 movement = { 0.0f, height / 5.0f, 0.0f };

					if (mainRegistry.has<HiddenComponent>(panelBuildingsID))
					{
						mainRegistry.remove<HiddenComponent>(panelBuildingsID);
						transformR.Transform = glm::translate(transformR.Transform, -movement);
						transformB.Transform = glm::translate(transformB.Transform, -movement);
					}
					else
					{
						mainRegistry.emplace<HiddenComponent>(panelBuildingsID);
						transformR.Transform = glm::translate(transformR.Transform, movement);
						transformB.Transform = glm::translate(transformB.Transform, movement);
					}
				}
				else
				{
					if (mainRegistry.has<HiddenComponent>(panelBuildingsID))
					{
						mainRegistry.remove<HiddenComponent>(panelBuildingsID);
						mainRegistry.emplace_or_replace<HiddenComponent>(panelRoadsID);
					}
					else
					{
						mainRegistry.emplace<HiddenComponent>(panelBuildingsID);
					}
				}
			}
			});

		m_ButtonNeeds = new Button(ButtonConstructorParameters{
			m_Scene->m_Registry,
			m_Scene->entityID,
			glm::vec3{ 0.5f, 0.5f, 0.0011f },
			glm::vec3{ 1.0f, 1.0f, 1.0f },
			glm::vec4{ 170.0f / 255.0f, 68.0f / 255.0f, 101.0f / 255.0f, 1.0f },
			nullptr,
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
			});

		m_ButtonTools = new Button(ButtonConstructorParameters{
			m_Scene->m_Registry,
			m_Scene->entityID,
			glm::vec3{ 2.0f, 0.5f, 0.0011f },
			glm::vec3{ 1.0f, 1.0f, 1.0f },
			glm::vec4{ 147.0f / 255.0f, 225.0f / 255.0f, 216.0f / 255.0f, 1.0f },
			nullptr,
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
			});

		std::vector<entt::entity> buttonList = {
			 m_ButtonRoads->entityID,
			 m_ButtonBuildings->entityID,
			 m_ButtonNeeds->entityID,
			 m_ButtonTools->entityID
		};
		m_Scene->m_Registry.emplace<ChildrenComponent>(m_Scene->entityID, buttonList);


		m_PanelRoads = new Panel(
			m_Scene->m_Registry,
			m_ButtonRoads->entityID,
			glm::vec3{ 0.5f, height - 4.5f, 0.001f },
			glm::vec3{ width - 1.0f, 4.0f, 1.0f },
			glm::vec4{ 221.0f / 255.0f, 255.0f / 255.0f, 247.0f / 255.0f, 1.0f },
			[]() {std::cout << "You clicked the panel that is for Roads!" << std::endl; }
		);
		//m_Scene->m_Registry.emplace<HiddenComponent>(m_PanelRoads->entityID);
		m_Scene->m_Registry.emplace<ChildrenComponent>(m_ButtonRoads->entityID, std::vector<entt::entity>{ m_PanelRoads->entityID });

		m_PanelBuildings = new Panel(
			m_Scene->m_Registry,
			m_ButtonBuildings->entityID,
			glm::vec3{ 0.5f, height - 4.5f, 0.001f },
			glm::vec3{ width - 1.0f, 4.0f, 1.0f },
			glm::vec4{ 255.0f / 255.0f, 166.0f / 255.0f, 158.0f / 255.0f, 1.0f },
			[]() {std::cout << "You clicked the panel that is for Buildings!" << std::endl; }
		);
		m_Scene->m_Registry.emplace<HiddenComponent>(m_PanelBuildings->entityID);
		m_Scene->m_Registry.emplace<ChildrenComponent>(m_ButtonBuildings->entityID, std::vector<entt::entity>{ m_PanelBuildings->entityID });

		m_PanelNeeds = new Panel(
			m_Scene->m_Registry,
			m_ButtonTools->entityID,
			glm::vec3{ 0.5f, 1.6f, 0.001f },
			glm::vec3{ 3.5f, 11.0f, 1.0f },
			glm::vec4{ 170.0f / 255.0f, 68.0f / 255.0f, 101.0f / 255.0f, 1.0f },
			[]() {std::cout << "You clicked the panel that is for Needs!" << std::endl; }
		);
		m_Scene->m_Registry.emplace<HiddenComponent>(m_PanelNeeds->entityID);
		m_Scene->m_Registry.emplace<ChildrenComponent>(m_ButtonNeeds->entityID, std::vector<entt::entity>{ m_PanelNeeds->entityID });

		m_PanelTools = new Panel(
			m_Scene->m_Registry,
			m_ButtonTools->entityID,
			glm::vec3{ 0.5f, 1.6f, 0.001f },
			glm::vec3{ 3.5f, 11.0f, 1.0f },
			glm::vec4{ 147.0f / 255.0f, 225.0f / 255.0f, 216.0f / 255.0f, 1.0f },
			[]() {std::cout << "You clicked the panel that is for Tools!" << std::endl; }
		);
		m_Scene->m_Registry.emplace<HiddenComponent>(m_PanelTools->entityID);
		m_Scene->m_Registry.emplace<ChildrenComponent>(m_ButtonTools->entityID, std::vector<entt::entity>{ m_PanelTools->entityID });

		/*Buttons in the tools panel*/ {
			glm::vec3 buttonSize{ 1.0f, 1.0f, 1.0f };
			glm::vec4 buttonColor{ 1.0f, 1.0f, 1.0f, 1.0f };
			m_ButtonTools_01 = new Button(ButtonConstructorParameters{ m_Scene->m_Registry, m_PanelTools->entityID, glm::vec3{ 1.0f, 2.0f, 0.0011f }, buttonSize, buttonColor,nullptr,
				[]() {std::cout << "You clicked the 1st Button inside the tools panel!" << std::endl; }
				});
			m_ButtonTools_02 = new Button(ButtonConstructorParameters{ m_Scene->m_Registry, m_PanelTools->entityID, glm::vec3{ 2.5f, 2.0f, 0.0011f }, buttonSize, buttonColor,nullptr,
				[]() {std::cout << "You clicked the 2nd Button inside the tools panel!" << std::endl; }
				});
			m_ButtonTools_03 = new Button(ButtonConstructorParameters{ m_Scene->m_Registry, m_PanelTools->entityID, glm::vec3{ 1.0f, 3.5f, 0.0011f }, buttonSize, buttonColor,nullptr,
				[]() {std::cout << "You clicked the 3rd Button inside the tools panel!" << std::endl; }
				});
			m_ButtonTools_04 = new Button(ButtonConstructorParameters{ m_Scene->m_Registry, m_PanelTools->entityID, glm::vec3{ 2.5f, 3.5f, 0.0011f }, buttonSize, buttonColor,nullptr,
				[]() {std::cout << "You clicked the 4th Button inside the tools panel!" << std::endl; }
				});
			m_ButtonTools_05 = new Button(ButtonConstructorParameters{ m_Scene->m_Registry, m_PanelTools->entityID, glm::vec3{ 1.0f, 5.0f, 0.0011f }, buttonSize, buttonColor,nullptr,
				[]() {std::cout << "You clicked the 5th Button inside the tools panel!" << std::endl; }
				});
			m_ButtonTools_06 = new Button(ButtonConstructorParameters{ m_Scene->m_Registry, m_PanelTools->entityID, glm::vec3{ 2.5f, 5.0f, 0.0011f }, buttonSize, buttonColor,nullptr,
				[]() {std::cout << "You clicked the 6th Button inside the tools panel!" << std::endl; }
				});
			m_ButtonTools_07 = new Button(ButtonConstructorParameters{ m_Scene->m_Registry, m_PanelTools->entityID, glm::vec3{ 1.0f, 6.5f, 0.0011f }, buttonSize, buttonColor,nullptr,
				[]() {std::cout << "You clicked the 7th Button inside the tools panel!" << std::endl; }
				});
			m_ButtonTools_08 = new Button(ButtonConstructorParameters{ m_Scene->m_Registry, m_PanelTools->entityID, glm::vec3{ 2.5f, 6.5f, 0.0011f }, buttonSize, buttonColor,nullptr,
				[]() {std::cout << "You clicked the 8th Button inside the tools panel!" << std::endl; }
				});
			m_ButtonTools_09 = new Button(ButtonConstructorParameters{ m_Scene->m_Registry, m_PanelTools->entityID, glm::vec3{ 1.0f, 8.0f, 0.0011f }, buttonSize, buttonColor,nullptr,
				[]() {std::cout << "You clicked the 9th Button inside the tools panel!" << std::endl; }
				});
			m_ButtonTools_10 = new Button(ButtonConstructorParameters{ m_Scene->m_Registry, m_PanelTools->entityID, glm::vec3{ 2.5f, 8.0f, 0.0011f }, buttonSize, buttonColor,nullptr,
				[]() {std::cout << "You clicked the 10th Button inside the tools panel!" << std::endl; }
				});
			m_ButtonTools_11 = new Button(ButtonConstructorParameters{ m_Scene->m_Registry, m_PanelTools->entityID, glm::vec3{ 1.0f, 9.5f, 0.0011f }, buttonSize, buttonColor,nullptr,
				[]() {std::cout << "You clicked the 11th Button inside the tools panel!" << std::endl; }
				});
			m_ButtonTools_12 = new Button(ButtonConstructorParameters{ m_Scene->m_Registry, m_PanelTools->entityID, glm::vec3{ 2.5f, 9.5f, 0.0011f }, buttonSize, buttonColor,nullptr,
				[]() {std::cout << "You clicked the 12th Button inside the tools panel!" << std::endl; }
				});
			m_ButtonTools_13 = new Button(ButtonConstructorParameters{ m_Scene->m_Registry, m_PanelTools->entityID, glm::vec3{ 1.0f, 11.0f, 0.0011f }, buttonSize, buttonColor,nullptr,
				[]() {std::cout << "You clicked the 13th Button inside the tools panel!" << std::endl; }
				});
			m_ButtonTools_14 = new Button(ButtonConstructorParameters{ m_Scene->m_Registry, m_PanelTools->entityID, glm::vec3{ 2.5f, 11.0f, 0.0011f }, buttonSize, buttonColor,nullptr,
				[]() {std::cout << "You clicked the 14th Button inside the tools panel!" << std::endl; }
				});
			std::vector<entt::entity> toolsButtonList = {
			 m_ButtonTools_01->entityID,
			 m_ButtonTools_02->entityID,
			 m_ButtonTools_03->entityID,
			 m_ButtonTools_04->entityID,
			 m_ButtonTools_05->entityID,
			 m_ButtonTools_06->entityID,
			 m_ButtonTools_07->entityID,
			 m_ButtonTools_08->entityID,
			 m_ButtonTools_09->entityID,
			 m_ButtonTools_10->entityID,
			 m_ButtonTools_11->entityID,
			 m_ButtonTools_12->entityID,
			 m_ButtonTools_13->entityID,
			 m_ButtonTools_14->entityID
			};
			m_Scene->m_Registry.emplace<ChildrenComponent>(m_PanelTools->entityID, toolsButtonList);
		}
		/*Buttons in the Needs panel*/ {
			glm::vec3 buttonSize{ 1.0f, 1.0f, 1.0f };
			glm::vec4 buttonColor{ 1.0f, 1.0f, 1.0f, 1.0f };
			m_ButtonNeeds_01 = new Button(ButtonConstructorParameters{ m_Scene->m_Registry, m_PanelNeeds->entityID, glm::vec3{ 1.0f, 2.0f, 0.0011f }, buttonSize, buttonColor, nullptr,
				[]() {std::cout << "You clicked the 1st Button inside the Needs panel!" << std::endl; }
				});
			m_ButtonNeeds_02 = new Button(ButtonConstructorParameters{ m_Scene->m_Registry, m_PanelNeeds->entityID, glm::vec3{ 2.5f, 2.0f, 0.0011f }, buttonSize, buttonColor, nullptr,
				[]() {std::cout << "You clicked the 2nd Button inside the Needs panel!" << std::endl; }
				});
			m_ButtonNeeds_03 = new Button(ButtonConstructorParameters{ m_Scene->m_Registry, m_PanelNeeds->entityID, glm::vec3{ 1.0f, 3.5f, 0.0011f }, buttonSize, buttonColor, nullptr,
				[]() {std::cout << "You clicked the 3rd Button inside the Needs panel!" << std::endl; }
				});
			m_ButtonNeeds_04 = new Button(ButtonConstructorParameters{ m_Scene->m_Registry, m_PanelNeeds->entityID, glm::vec3{ 2.5f, 3.5f, 0.0011f }, buttonSize, buttonColor, nullptr,
				[]() {std::cout << "You clicked the 4th Button inside the Needs panel!" << std::endl; }
				});
			m_ButtonNeeds_05 = new Button(ButtonConstructorParameters{ m_Scene->m_Registry, m_PanelNeeds->entityID, glm::vec3{ 1.0f, 5.0f, 0.0011f }, buttonSize, buttonColor, nullptr,
				[]() {std::cout << "You clicked the 5th Button inside the Needs panel!" << std::endl; }
				});
			m_ButtonNeeds_06 = new Button(ButtonConstructorParameters{ m_Scene->m_Registry, m_PanelNeeds->entityID, glm::vec3{ 2.5f, 5.0f, 0.0011f }, buttonSize, buttonColor, nullptr,
				[]() {std::cout << "You clicked the 6th Button inside the Needs panel!" << std::endl; }
				});
			m_ButtonNeeds_07 = new Button(ButtonConstructorParameters{ m_Scene->m_Registry, m_PanelNeeds->entityID, glm::vec3{ 1.0f, 6.5f, 0.0011f }, buttonSize, buttonColor, nullptr,
				[]() {std::cout << "You clicked the 7th Button inside the Needs panel!" << std::endl; }
				});
			m_ButtonNeeds_08 = new Button(ButtonConstructorParameters{ m_Scene->m_Registry, m_PanelNeeds->entityID, glm::vec3{ 2.5f, 6.5f, 0.0011f }, buttonSize, buttonColor, nullptr,
				[]() {std::cout << "You clicked the 8th Button inside the Needs panel!" << std::endl; }
				});
			m_ButtonNeeds_09 = new Button(ButtonConstructorParameters{ m_Scene->m_Registry, m_PanelNeeds->entityID, glm::vec3{ 1.0f, 8.0f, 0.0011f }, buttonSize, buttonColor, nullptr,
				[]() {std::cout << "You clicked the 9th Button inside the Needs panel!" << std::endl; }
				});
			m_ButtonNeeds_10 = new Button(ButtonConstructorParameters{ m_Scene->m_Registry, m_PanelNeeds->entityID, glm::vec3{ 2.5f, 8.0f, 0.0011f }, buttonSize, buttonColor, nullptr,
				[]() {std::cout << "You clicked the 10th Button inside the Needs panel!" << std::endl; }
				});
			m_ButtonNeeds_11 = new Button(ButtonConstructorParameters{ m_Scene->m_Registry, m_PanelNeeds->entityID, glm::vec3{ 1.0f, 9.5f, 0.0011f }, buttonSize, buttonColor, nullptr,
				[]() {std::cout << "You clicked the 11th Button inside the Needs panel!" << std::endl; }
				});
			m_ButtonNeeds_12 = new Button(ButtonConstructorParameters{ m_Scene->m_Registry, m_PanelNeeds->entityID, glm::vec3{ 2.5f, 9.5f, 0.0011f }, buttonSize, buttonColor, nullptr,
				[]() {std::cout << "You clicked the 12th Button inside the Needs panel!" << std::endl; }
				});
			m_ButtonNeeds_13 = new Button(ButtonConstructorParameters{ m_Scene->m_Registry, m_PanelNeeds->entityID, glm::vec3{ 1.0f, 11.0f, 0.0011f }, buttonSize, buttonColor, nullptr,
				[]() {std::cout << "You clicked the 13th Button inside the Needs panel!" << std::endl; }
				});
			m_ButtonNeeds_14 = new Button(ButtonConstructorParameters{ m_Scene->m_Registry, m_PanelNeeds->entityID, glm::vec3{ 2.5f, 11.0f, 0.0011f }, buttonSize, buttonColor, nullptr,
				[]() {std::cout << "You clicked the 14th Button inside the Needs panel!" << std::endl; }
				});
			std::vector<entt::entity> needsButtonList = {
			 m_ButtonNeeds_01->entityID,
			 m_ButtonNeeds_02->entityID,
			 m_ButtonNeeds_03->entityID,
			 m_ButtonNeeds_04->entityID,
			 m_ButtonNeeds_05->entityID,
			 m_ButtonNeeds_06->entityID,
			 m_ButtonNeeds_07->entityID,
			 m_ButtonNeeds_08->entityID,
			 m_ButtonNeeds_09->entityID,
			 m_ButtonNeeds_10->entityID,
			 m_ButtonNeeds_11->entityID,
			 m_ButtonNeeds_12->entityID,
			 m_ButtonNeeds_13->entityID,
			 m_ButtonNeeds_14->entityID
			};
			m_Scene->m_Registry.emplace<ChildrenComponent>(m_PanelNeeds->entityID, needsButtonList);
		}

		// Move this
		std::vector<Prefab*> result;
		namespace fs = std::filesystem;
		std::string s = fs::current_path().string();
		std::string pathr = s + "\\assets\\objects\\roads";

		std::vector<std::string> roadtumbnailimagefiles = Helper::GetFiles(pathr, "tumbnail_", ".png");

		std::string pathh = s + "\\assets\\objects\\houses";

		std::vector<std::string> buildingtumbnailimagefiles = Helper::GetFiles(pathh, "tumbnail_", ".png");

		/*Buttons in the Roads panel*/ {
			size_t roadCount = m_Parent->roads.size();
			ChildrenComponent& children = m_Scene->m_Registry.emplace<ChildrenComponent>(m_PanelRoads->entityID, std::vector<entt::entity>{});
			for (size_t i = 0; i < roadCount; i++)
			{
				Button* roadPanelbutton = new Button(ButtonConstructorParameters{
					m_Scene->m_Registry,
					m_PanelRoads->entityID,
					glm::vec3{ 1.0f + i * 4.0f, height - 4.0f, 0.0011f },
					glm::vec3{ 3.0f, 3.0f, 1.0f },
					glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f },
					Texture2D::Create(roadtumbnailimagefiles[i]),
					[i, this]() {
						std::cout << "You clicked the " << (i + 1) << "th Button inside the Road panel!" << std::endl;
						this->m_Parent->testScene->SetConstructionMode(ConstructionMode::Road);
						this->m_Parent->testScene->SetRoadConstructionMode(RoadConstructionMode::Construct);
						this->m_Parent->testScene->SetSelectedConstructionRoad(i);
					}
					});
				children.Children.push_back(roadPanelbutton->entityID);
				m_RoadPanelButtonList.push_back(roadPanelbutton);
			}
		}
		/*Buttons in the Building panel*/ {
			size_t buildingCount = m_Parent->buildings.size();
			ChildrenComponent& children = m_Scene->m_Registry.emplace<ChildrenComponent>(m_PanelBuildings->entityID, std::vector<entt::entity>{});
			for (size_t i = 0; i < buildingCount; i++)
			{
				Button* buildingPanelbutton = new Button(ButtonConstructorParameters{
					m_Scene->m_Registry,
					m_PanelBuildings->entityID,
					glm::vec3{ 1.0f + i * 4.0f, height - 4.0f, 0.0011f },
					glm::vec3{ 3.0f, 3.0f, 1.0f },
					glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f },
					nullptr,//Texture2D::Create(buildingtumbnailimagefiles[i]),
					[i, this]() {
						std::cout << "You clicked the " << (i + 1) << "th Button inside the Building panel!" << std::endl;
						this->m_Parent->testScene->SetConstructionMode(ConstructionMode::Building);
						this->m_Parent->testScene->SetBuildingConstructionMode(BuildingConstructionMode::Construct);
						this->m_Parent->testScene->SetSelectedConstructionBuilding(i);
					}
					});
				children.Children.push_back(buildingPanelbutton->entityID);
				m_BuildingPanelButtonList.push_back(buildingPanelbutton);
			}
		}
	}

	UIScene::~UIScene()
	{
		delete m_Scene;

		delete m_ButtonRoads;
		delete  m_ButtonBuildings;
		delete  m_ButtonNeeds;
		delete  m_ButtonTools;

		delete m_PanelRoads;
		delete m_PanelBuildings;
		delete m_PanelNeeds;
		delete m_PanelTools;

		delete m_ButtonTools_01;
		delete m_ButtonTools_02;
		delete m_ButtonTools_03;
		delete m_ButtonTools_04;
		delete m_ButtonTools_05;
		delete m_ButtonTools_06;
		delete m_ButtonTools_07;
		delete m_ButtonTools_08;
		delete m_ButtonTools_09;
		delete m_ButtonTools_10;
		delete m_ButtonTools_11;
		delete m_ButtonTools_12;
		delete m_ButtonTools_13;
		delete m_ButtonTools_14;

		delete m_ButtonNeeds_01;
		delete m_ButtonNeeds_02;
		delete m_ButtonNeeds_03;
		delete m_ButtonNeeds_04;
		delete m_ButtonNeeds_05;
		delete m_ButtonNeeds_06;
		delete m_ButtonNeeds_07;
		delete m_ButtonNeeds_08;
		delete m_ButtonNeeds_09;
		delete m_ButtonNeeds_10;
		delete m_ButtonNeeds_11;
		delete m_ButtonNeeds_12;
		delete m_ButtonNeeds_13;
		delete m_ButtonNeeds_14;

	}

	void UIScene::OnAttach()
	{
	}

	void UIScene::OnUpdate(Can::TimeStep ts)
	{
		float widthHalf = m_AspectRatio * m_ZoomLevel;
		float heightHalf = m_ZoomLevel;

		Renderer2D::BeginScene(m_CameraController.GetCamera());

		glm::vec2 offset = { -widthHalf, heightHalf };
		ChildrenComponent& children = m_Scene->m_Registry.get<ChildrenComponent>(m_Scene->entityID);
		for (auto entity : children)
			Draw(entity, &(m_Scene->m_Registry), offset);

		Can::Renderer2D::EndScene();
	}
	void UIScene::OnEvent(Can::Event::Event& event)
	{
		Can::Event::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Event::MouseButtonPressedEvent>(CAN_BIND_EVENT_FN(UIScene::OnMousePressed));

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
		return false;
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

		Renderer2D::DrawQuad(DrawQuadParameters{ pos - glm::vec3{ 0.0f, 0.0f, 0.00001f }, scale + glm::vec3{ 0.1f, 0.1f, 0.1f }, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f }, nullptr });
		if (spriteRenderer.texture)
			Renderer2D::DrawQuad(newTransform, DrawQuadParameters{ glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, spriteRenderer.color, spriteRenderer.texture });
		else
			Renderer2D::DrawQuad(newTransform, DrawQuadParameters{ glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, spriteRenderer.color, nullptr });
	}
}