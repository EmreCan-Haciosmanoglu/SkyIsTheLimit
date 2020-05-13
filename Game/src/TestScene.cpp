#include "canpch.h"
#include "TestScene.h"
#include "GameApp.h"

TestScene::TestScene(GameApp* parent)
	: m_Parent(parent)
	, m_MainCameraController(45.0f, 1920.0f / 1080.0f, 0.001f, 100.0f)
{
}

void TestScene::OnUpdate(Can::TimeStep ts)
{
	m_MainCameraController.OnUpdate(ts);

	Can::RenderCommand::SetClearColor({ 0.9f, 0.9f, 0.9f, 1.0f });
	Can::RenderCommand::Clear();

	Can::Renderer3D::BeginScene(m_MainCameraController.GetCamera());

	Can::Renderer3D::DrawObjects();

	Can::Renderer3D::EndScene();
}

void TestScene::OnEvent(Can::Event::Event& event)
{
	m_MainCameraController.OnEvent(event);
}
