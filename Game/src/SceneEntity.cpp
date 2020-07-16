#include "canpch.h"
#include "SceneEntity.h"
#include "Can/ECS/UI/Panel.h"

namespace Can
{
	void SceneEntity::Draw(const glm::vec3& offset)
	{
		std::vector<Panel*> panels = GetChildren<Panel>();
		for (auto it = panels.begin(); it!=panels.end();it++)
		{
			(*it)->Draw(offset);
		}
	}
}