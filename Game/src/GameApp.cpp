#include "GameApp.h"

#include "../stb/stb_image.h"
#include "Can/EntryPoint.h"

Can::Application* Can::CreateApplication()
{
	return new GameApp();
}

namespace Can
{
	GameApp::GameApp()
		: testScene(new TestScene(this))
		, m_TestObject(nullptr)
		, m_Terrain(new Can::Object())
	{
		m_TestObject = UploadObject("assets/objects/padlock.obj", "assets/shaders/Object.glsl", "assets/objects/padlock.png");
		m_Terrain = UploadTerrain("assets/objects/heightmap_smallest.png");
		PushLayer(testScene);
	}

	GameApp::~GameApp()
	{
	}

	bool GameApp::loadOBJ(const char* path, std::vector<glm::vec3>& out_vertices, std::vector<glm::vec2>& out_uvs, std::vector<glm::vec3>& out_normals)
	{
		std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
		std::vector< glm::vec3 > temp_vertices;
		std::vector< glm::vec2 > temp_uvs;
		std::vector< glm::vec3 > temp_normals;

		FILE* file = fopen(path, "r");
		if (file == NULL) {
			printf("Impossible to open the file !\n");
			return false;
		}
		while (1)
		{
			char lineHeader[128];
			int res = fscanf(file, "%s", lineHeader);
			if (res == EOF)
				break; // EOF = End Of File. Quit the loop.
			if (strcmp(lineHeader, "v") == 0)
			{
				glm::vec3 vertex;
				fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
				temp_vertices.push_back(vertex);
			}
			else if (strcmp(lineHeader, "vt") == 0)
			{
				glm::vec2 uv;
				fscanf(file, "%f %f\n", &uv.x, &uv.y);
				temp_uvs.push_back(uv);
			}
			else if (strcmp(lineHeader, "vn") == 0)
			{
				glm::vec3 normal;
				fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
				temp_normals.push_back(normal);
			}
			else if (strcmp(lineHeader, "f") == 0)
			{
				std::string vertex1, vertex2, vertex3;
				unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
				int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
				if (matches != 9) {
					printf("File can't be read by our simple parser : ( Try exporting with other options\n");
					return false;
				}
				vertexIndices.push_back(vertexIndex[0]);
				vertexIndices.push_back(vertexIndex[1]);
				vertexIndices.push_back(vertexIndex[2]);
				uvIndices.push_back(uvIndex[0]);
				uvIndices.push_back(uvIndex[1]);
				uvIndices.push_back(uvIndex[2]);
				normalIndices.push_back(normalIndex[0]);
				normalIndices.push_back(normalIndex[1]);
				normalIndices.push_back(normalIndex[2]);
			}
		}
		for (unsigned int i = 0; i < vertexIndices.size(); i++)
		{
			unsigned int vertexIndex = vertexIndices[i];
			unsigned int uvIndex = uvIndices[i];
			unsigned int normalIndex = normalIndices[i];

			glm::vec3 vertex = temp_vertices[vertexIndex - 1];
			out_vertices.push_back(vertex);

			glm::vec2 uv = temp_uvs[uvIndex - 1];
			out_uvs.push_back(uv);

			glm::vec3 normal = temp_normals[normalIndex - 1];
			out_normals.push_back(normal);
		}
		return true;
	}

	void GameApp::SetTransform(Can::Object* object, glm::vec3 pos, glm::vec3 scale)
	{
		object->position = pos;
		object->scale = scale;
		object->transform = glm::translate(glm::mat4(1.0f), object->position) * glm::scale(glm::mat4(1.0f), object->scale);
	}

	void GameApp::SetTransform(Can::Object* object, glm::vec3 pos, glm::vec3 scale, glm::vec3 rotation)
	{
		object->position = pos;
		object->scale = scale;
		object->rotation = rotation;
		object->transform = glm::translate(glm::mat4(1.0f), object->position) * glm::scale(glm::mat4(1.0f), object->scale) * glm::rotate(glm::mat4(1.0f), object->rotation.y, { 0,1,0 });

	}

	glm::vec3 GameApp::RayPlaneIntersection(
		const glm::vec3& X,
		const glm::vec3& v,
		const glm::vec3& C,
		const glm::vec3& n
	)
	{
		glm::vec3 w = C - X;
		float k = glm::dot(w, n) / glm::dot(v, n);
		return X + k * v;
	}

	bool GameApp::RayTriangleIntersection(
		const glm::vec3& camPos,
		const glm::vec3& ray,
		const glm::vec3& A,
		const glm::vec3& B,
		const glm::vec3& C,
		const glm::vec3& normal
	)
	{
		glm::vec3 u = (C - B) - (glm::dot(C - A, C - B) / glm::dot(C - A, C - A)) * (C - A);
		glm::vec3 v = (B - A) - (glm::dot(B - C, B - A) / glm::dot(B - C, B - C)) * (B - C);
		glm::vec3 I = camPos + ray * (glm::dot(A - camPos, normal) / glm::dot(ray, normal));
		float a = 1 - glm::dot(v, I - A) / glm::dot(v, B - A);
		float b = 1 - glm::dot(u, I - B) / glm::dot(u, C - B);

		return a > 0 && a < 1 && b > 0 && b < 1 && a + b < 1;
	}

	Can::Object* GameApp::ConstructObject(const std::string& shaderPath, const std::string& texturePath, std::vector<glm::vec3>& vertices, std::vector<glm::vec2>& uvs, std::vector<glm::vec3>& normals)
	{
		Can::Object* object = new Can::Object();
		object->VA = Can::VertexArray::Create();
		int vSize = vertices.size();
		object->indexCount = vSize;
		int size = vSize * (3 + 2 + 3);
		float* m_Vertices = new float[size];

		for (int i = 0; i < vSize; i++)
		{
			int index = i * 8;
			m_Vertices[index + 0] = vertices[i].x;
			m_Vertices[index + 1] = vertices[i].y;
			m_Vertices[index + 2] = vertices[i].z;
			m_Vertices[index + 3] = uvs[i].x;
			m_Vertices[index + 4] = uvs[i].y;
			m_Vertices[index + 5] = normals[i].x;
			m_Vertices[index + 6] = normals[i].y;
			m_Vertices[index + 7] = normals[i].z;
		}

		object->Vertices = m_Vertices;
		object->VB = Can::VertexBuffer::Create(m_Vertices, sizeof(float) * size, true);
		object->VB->SetLayout({
		   { Can::ShaderDataType::Float3, "a_Position"},
		   { Can::ShaderDataType::Float2, "a_UV"},
		   { Can::ShaderDataType::Float3, "a_Normal"}
			});

		object->VA->AddVertexBuffer(object->VB);

		uint32_t* m_Indices = new uint32_t[vSize];

		for (int i = 0; i < vSize; i++)
		{
			m_Indices[i] = i;
		}

		object->Indices = m_Indices;
		object->IB = Can::IndexBuffer::Create(m_Indices, vSize);
		object->VA->SetIndexBuffer(object->IB);

		object->T = Can::Texture2D::Create(texturePath);
		object->S = Can::Shader::Create(shaderPath);

		object->S->Bind();
		object->S->SetInt("u_Texture", 0);
		object->S->SetFloat3("u_LightPos", { 1.0f, 1.0f, -1.0f });

		SetTransform(object, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f });


		return object;
	}

	Can::Object* GameApp::UploadObject(const char* objectPath, const std::string& shaderPath, const std::string& texturePath)
	{
		Can::Object* target = nullptr;
		std::vector< glm::vec3 > vertices;
		std::vector< glm::vec2 > uvs;
		std::vector< glm::vec3 > normals;
		bool res = loadOBJ(objectPath, vertices, uvs, normals);
		if (res)
		{
			target = ConstructObject(shaderPath, texturePath, vertices, uvs, normals);
			Can::Renderer3D::AddObject(target);
			return target;
		}
	}

	Can::Object* GameApp::GenerateStraightRoad(const char* objectPath, const std::string& shaderPath, const std::string& texturePath, const glm::vec3& startCoord, const glm::vec3& endCoord)
	{
		Can::Object* road = new Can::Object();
		Can::Object* road_end = UploadObject("assets/objects/road_end.obj", "assets/shaders/Object.glsl", "assets/objects/road.png");
		Can::Object* road_start = UploadObject("assets/objects/road_start.obj", "assets/shaders/Object.glsl", "assets/objects/road.png");

		std::vector< glm::vec3 > vertices;
		std::vector< glm::vec2 > uvs;
		std::vector< glm::vec3 > normals;
		bool res = loadOBJ(objectPath, vertices, uvs, normals);
		if (res)
		{
			road->VA = Can::VertexArray::Create();
			int vSize = vertices.size();

			//try to use existing ones
			glm::vec2 AB = {
				endCoord.x - startCoord.x,
				endCoord.z - startCoord.z
			};
			float mAB = glm::length(AB);
			float max = 0.0f;
			float min = 0.0f;
			for (size_t i = 0; i < vSize; i++)
			{
				float z = vertices[i].z;
				max = std::max(max, z);
				min = std::min(min, z);
			}
			float l = (max - min);
			float c = 100 * mAB / l + 1;
			float scaleN = c / (int)c;

			road->indexCount = vSize * (int)c + 2 * road_end->indexCount;
			int size = road->indexCount * (3 + 2 + 3);
			float* m_Vertices = new float[size];

			for (int j = 0; j < c; j++)
			{
				for (int i = 0; i < vSize; i++)
				{
					int offset = j * vSize * 8;
					int index = i * 8;
					m_Vertices[offset + index + 0] = vertices[i].x;
					m_Vertices[offset + index + 1] = vertices[i].y;
					m_Vertices[offset + index + 2] = (vertices[i].z + l * j) * scaleN;
					m_Vertices[offset + index + 3] = uvs[i].x;
					m_Vertices[offset + index + 4] = uvs[i].y;
					m_Vertices[offset + index + 5] = normals[i].x;
					m_Vertices[offset + index + 6] = normals[i].y;
					m_Vertices[offset + index + 7] = normals[i].z;
				}
			}
			int offset = vSize * (int)c * 8;
			for (size_t i = 0; i < road_end->indexCount; i++)
			{
				int index = i * 8;
				m_Vertices[offset + index + 0] = road_end->Vertices[index + 0];
				m_Vertices[offset + index + 1] = road_end->Vertices[index + 1];
				m_Vertices[offset + index + 2] = (road_end->Vertices[index + 2] + (c - 0.5f) * l)* scaleN;
				m_Vertices[offset + index + 3] = road_end->Vertices[index + 3];
				m_Vertices[offset + index + 4] = road_end->Vertices[index + 4];
				m_Vertices[offset + index + 5] = road_end->Vertices[index + 5];
				m_Vertices[offset + index + 6] = road_end->Vertices[index + 6];
				m_Vertices[offset + index + 7] = road_end->Vertices[index + 7];
			}
			offset = vSize * (int)c * 8 + road_end->indexCount * 8;
			for (size_t i = 0; i < road_start->indexCount; i++)
			{
				int index = i * 8;
				m_Vertices[offset + index + 0] = road_start->Vertices[index + 0];
				m_Vertices[offset + index + 1] = road_start->Vertices[index + 1];
				m_Vertices[offset + index + 2] = (road_start->Vertices[index + 2] + (0 - 0.5f) * l)* scaleN;
				m_Vertices[offset + index + 3] = road_start->Vertices[index + 3];
				m_Vertices[offset + index + 4] = road_start->Vertices[index + 4];
				m_Vertices[offset + index + 5] = road_start->Vertices[index + 5];
				m_Vertices[offset + index + 6] = road_start->Vertices[index + 6];
				m_Vertices[offset + index + 7] = road_start->Vertices[index + 7];
			}

			road->Vertices = m_Vertices;
			road->VB = Can::VertexBuffer::Create(m_Vertices, sizeof(float) * size, true);
			road->VB->SetLayout({
			   { Can::ShaderDataType::Float3, "a_Position"},
			   { Can::ShaderDataType::Float2, "a_UV"},
			   { Can::ShaderDataType::Float3, "a_Normal"}
				});

			road->VA->AddVertexBuffer(road->VB);

			uint32_t* m_Indices = new uint32_t[road->indexCount];

			for (int i = 0; i < road->indexCount; i++)
			{
				m_Indices[i] = i;
			}

			road->Indices = m_Indices;
			road->IB = Can::IndexBuffer::Create(m_Indices, road->indexCount);
			road->VA->SetIndexBuffer(road->IB);

			road->T = Can::Texture2D::Create(texturePath);
			road->S = Can::Shader::Create(shaderPath);

			road->S->Bind();
			road->S->SetInt("u_Texture", 0);
			road->S->SetFloat3("u_LightPos", { 1.0f, 1.0f, -1.0f });

			int ed = AB.x <= 0 ? 180 : 0;

			SetTransform(road, { startCoord.x,endCoord.y,startCoord.z }, { 0.01f, 0.01f, 0.01f }, { 0.0f,glm::radians(glm::degrees(glm::atan(-AB.y / AB.x)) + 90 + ed),0.0f });
			Can::Renderer3D::AddObject(road);

			Can::Renderer3D::DeleteObject(road_end);
			Can::Renderer3D::DeleteObject(road_start);
			delete road_end;
			delete road_start;

			return road;
		}
		return nullptr;
	}

	Can::Object* GameApp::UploadTerrain(const std::string& texturePath)
	{
		Can::Object* target = new Can::Object();
		target->VA = Can::VertexArray::Create();
		int width, height, channels;


		unsigned char* data = stbi_load(texturePath.c_str(), &width, &height, &channels, 0);

		std::cout << "channels : " << channels << std::endl;
		std::cout << "width : " << width << std::endl;
		std::cout << "height : " << height << std::endl;

		int w = width - 1;
		int h = height - 1;
		float* vertecies = new float[w * h * 2 * 3 * (3 + 4 + 3)];
		glm::vec3 color1 = { 19.0f / 255.0f, 173.0f / 255.0f, 14.0f / 255.0f };
		glm::vec3 color2 = { 25.0f / 255.0f, 93.0f / 255.0f, 24.0f / 255.0f };
		glm::vec3 color3 = { 182.0f / 255.0f, 64.0f / 255.0f, 16.0f / 255.0f };
		glm::vec3 color4 = { 61.0f / 255.0f, 28.0f / 255.0f, 10.0f / 255.0f };
		glm::vec3 color5 = { 112.0f / 255.0f, 217.0f / 255.0f, 238.0f / 255.0f };
		int vertexIndex = 0;
		float z = 0.0f;
		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{
				unsigned char* p1 = &data[((x + 0) + (width) * (y + 0)) * channels];
				unsigned char* p2 = &data[((x + 0) + (width) * (y + 1)) * channels];
				unsigned char* p3 = &data[((x + 1) + (width) * (y + 1)) * channels];
				unsigned char* p4 = &data[((x + 1) + (width) * (y + 0)) * channels];

				{
					z = p1[0] / 255.0f;
					vertecies[vertexIndex++] = x / 50.f;
					vertecies[vertexIndex++] = z;
					vertecies[vertexIndex++] = -y / 50.f;

					if (z <= 0.1f)
					{
						vertecies[vertexIndex++] = color1.r;
						vertecies[vertexIndex++] = color1.g;
						vertecies[vertexIndex++] = color1.b;
					}
					else if (z <= 0.3)
					{
						vertecies[vertexIndex++] = color2.r;
						vertecies[vertexIndex++] = color2.g;
						vertecies[vertexIndex++] = color2.b;
					}
					else if (z <= 0.5)
					{
						vertecies[vertexIndex++] = color3.r;
						vertecies[vertexIndex++] = color3.g;
						vertecies[vertexIndex++] = color3.b;
					}
					else if (z <= 0.7)
					{
						vertecies[vertexIndex++] = color4.r;
						vertecies[vertexIndex++] = color4.g;
						vertecies[vertexIndex++] = color4.b;
					}
					else
					{
						vertecies[vertexIndex++] = color4.r;
						vertecies[vertexIndex++] = color4.g;
						vertecies[vertexIndex++] = color4.b;
					}
					vertecies[vertexIndex++] = 1.0f;
					vertexIndex += 3;
				}
				{
					z = p4[0] / 255.0f;
					vertecies[vertexIndex++] = (x + 1) / 50.f;
					vertecies[vertexIndex++] = z;
					vertecies[vertexIndex++] = -(y + 0) / 50.f;

					if (z <= 0.1f)
					{
						vertecies[vertexIndex++] = color1.r;
						vertecies[vertexIndex++] = color1.g;
						vertecies[vertexIndex++] = color1.b;
					}
					else if (z <= 0.3)
					{
						vertecies[vertexIndex++] = color2.r;
						vertecies[vertexIndex++] = color2.g;
						vertecies[vertexIndex++] = color2.b;
					}
					else if (z <= 0.5)
					{
						vertecies[vertexIndex++] = color3.r;
						vertecies[vertexIndex++] = color3.g;
						vertecies[vertexIndex++] = color3.b;
					}
					else if (z <= 0.7)
					{
						vertecies[vertexIndex++] = color4.r;
						vertecies[vertexIndex++] = color4.g;
						vertecies[vertexIndex++] = color4.b;
					}
					else
					{
						vertecies[vertexIndex++] = color4.r;
						vertecies[vertexIndex++] = color4.g;
						vertecies[vertexIndex++] = color4.b;
					}
					vertecies[vertexIndex++] = 1.0f;
					vertexIndex += 3;
				}
				{
					z = p3[0] / 255.0f;
					vertecies[vertexIndex++] = (x + 1) / 50.f;
					vertecies[vertexIndex++] = z;
					vertecies[vertexIndex++] = -(y + 1) / 50.f;

					if (z <= 0.1f)
					{
						vertecies[vertexIndex++] = color1.r;
						vertecies[vertexIndex++] = color1.g;
						vertecies[vertexIndex++] = color1.b;
					}
					else if (z <= 0.3)
					{
						vertecies[vertexIndex++] = color2.r;
						vertecies[vertexIndex++] = color2.g;
						vertecies[vertexIndex++] = color2.b;
					}
					else if (z <= 0.5)
					{
						vertecies[vertexIndex++] = color3.r;
						vertecies[vertexIndex++] = color3.g;
						vertecies[vertexIndex++] = color3.b;
					}
					else if (z <= 0.7)
					{
						vertecies[vertexIndex++] = color4.r;
						vertecies[vertexIndex++] = color4.g;
						vertecies[vertexIndex++] = color4.b;
					}
					else
					{
						vertecies[vertexIndex++] = color4.r;
						vertecies[vertexIndex++] = color4.g;
						vertecies[vertexIndex++] = color4.b;
					}
					vertecies[vertexIndex++] = 1.0f;
					vertexIndex += 3;
				}
				{
					z = p1[0] / 255.0f;
					vertecies[vertexIndex++] = x / 50.f;
					vertecies[vertexIndex++] = z;
					vertecies[vertexIndex++] = -y / 50.f;

					if (z <= 0.1f)
					{
						vertecies[vertexIndex++] = color1.r;
						vertecies[vertexIndex++] = color1.g;
						vertecies[vertexIndex++] = color1.b;
					}
					else if (z <= 0.3)
					{
						vertecies[vertexIndex++] = color2.r;
						vertecies[vertexIndex++] = color2.g;
						vertecies[vertexIndex++] = color2.b;
					}
					else if (z <= 0.5)
					{
						vertecies[vertexIndex++] = color3.r;
						vertecies[vertexIndex++] = color3.g;
						vertecies[vertexIndex++] = color3.b;
					}
					else if (z <= 0.7)
					{
						vertecies[vertexIndex++] = color4.r;
						vertecies[vertexIndex++] = color4.g;
						vertecies[vertexIndex++] = color4.b;
					}
					else
					{
						vertecies[vertexIndex++] = color4.r;
						vertecies[vertexIndex++] = color4.g;
						vertecies[vertexIndex++] = color4.b;
					}
					vertecies[vertexIndex++] = 1.0f;
					vertexIndex += 3;
				}
				{
					z = p3[0] / 255.0f;
					vertecies[vertexIndex++] = (x + 1) / 50.f;
					vertecies[vertexIndex++] = z;
					vertecies[vertexIndex++] = -(y + 1) / 50.f;

					if (z <= 0.1f)
					{
						vertecies[vertexIndex++] = color1.r;
						vertecies[vertexIndex++] = color1.g;
						vertecies[vertexIndex++] = color1.b;
					}
					else if (z <= 0.3)
					{
						vertecies[vertexIndex++] = color2.r;
						vertecies[vertexIndex++] = color2.g;
						vertecies[vertexIndex++] = color2.b;
					}
					else if (z <= 0.5)
					{
						vertecies[vertexIndex++] = color3.r;
						vertecies[vertexIndex++] = color3.g;
						vertecies[vertexIndex++] = color3.b;
					}
					else if (z <= 0.7)
					{
						vertecies[vertexIndex++] = color4.r;
						vertecies[vertexIndex++] = color4.g;
						vertecies[vertexIndex++] = color4.b;
					}
					else
					{
						vertecies[vertexIndex++] = color4.r;
						vertecies[vertexIndex++] = color4.g;
						vertecies[vertexIndex++] = color4.b;
					}
					vertecies[vertexIndex++] = 1.0f;
					vertexIndex += 3;
				}
				{
					z = p2[0] / 255.0f;
					vertecies[vertexIndex++] = (x + 0) / 50.f;
					vertecies[vertexIndex++] = z;
					vertecies[vertexIndex++] = -(y + 1) / 50.f;

					if (z <= 0.1f)
					{
						vertecies[vertexIndex++] = color1.r;
						vertecies[vertexIndex++] = color1.g;
						vertecies[vertexIndex++] = color1.b;
					}
					else if (z <= 0.3)
					{
						vertecies[vertexIndex++] = color2.r;
						vertecies[vertexIndex++] = color2.g;
						vertecies[vertexIndex++] = color2.b;
					}
					else if (z <= 0.5)
					{
						vertecies[vertexIndex++] = color3.r;
						vertecies[vertexIndex++] = color3.g;
						vertecies[vertexIndex++] = color3.b;
					}
					else if (z <= 0.7)
					{
						vertecies[vertexIndex++] = color4.r;
						vertecies[vertexIndex++] = color4.g;
						vertecies[vertexIndex++] = color4.b;
					}
					else
					{
						vertecies[vertexIndex++] = color4.r;
						vertecies[vertexIndex++] = color4.g;
						vertecies[vertexIndex++] = color4.b;
					}
					vertecies[vertexIndex++] = 1.0f;

					vertexIndex += 3;
				}
			}
		}
		vertexIndex = 0;
		for (float x = 0; x < w; x++)
		{
			for (float y = 0; y < h; y++)
			{
				glm::vec3 a00(vertecies[vertexIndex + 0 + 0], vertecies[vertexIndex + 0 + 1], vertecies[vertexIndex + 0 + 2]);
				glm::vec3 a10(vertecies[vertexIndex + 10 + 0], vertecies[vertexIndex + 10 + 1], vertecies[vertexIndex + 10 + 2]);
				glm::vec3 a11(vertecies[vertexIndex + 20 + 0], vertecies[vertexIndex + 20 + 1], vertecies[vertexIndex + 20 + 2]);
				glm::vec3 a01(vertecies[vertexIndex + 50 + 0], vertecies[vertexIndex + 50 + 1], vertecies[vertexIndex + 50 + 2]);

				glm::vec3 u1 = a11 - a00;
				glm::vec3 v1 = a10 - a00;

				glm::vec3 u2 = a01 - a00;
				glm::vec3 v2 = a11 - a00;

				glm::vec3 norm1 = glm::cross(v1, u1);
				glm::vec3 norm2 = glm::cross(v2, u2);

				vertecies[vertexIndex + 0 + 7] = norm1.x;
				vertecies[vertexIndex + 0 + 8] = norm1.y;
				vertecies[vertexIndex + 0 + 9] = norm1.z;

				vertecies[vertexIndex + 10 + 7] = norm1.x;
				vertecies[vertexIndex + 10 + 8] = norm1.y;
				vertecies[vertexIndex + 10 + 9] = norm1.z;

				vertecies[vertexIndex + 20 + 7] = norm1.x;
				vertecies[vertexIndex + 20 + 8] = norm1.y;
				vertecies[vertexIndex + 20 + 9] = norm1.z;

				vertecies[vertexIndex + 30 + 7] = norm2.x;
				vertecies[vertexIndex + 30 + 8] = norm2.y;
				vertecies[vertexIndex + 30 + 9] = norm2.z;

				vertecies[vertexIndex + 40 + 7] = norm2.x;
				vertecies[vertexIndex + 40 + 8] = norm2.y;
				vertecies[vertexIndex + 40 + 9] = norm2.z;

				vertecies[vertexIndex + 50 + 7] = norm2.x;
				vertecies[vertexIndex + 50 + 8] = norm2.y;
				vertecies[vertexIndex + 50 + 9] = norm2.z;
				vertexIndex += 60;
			}
		}

		int indexSize = w * h * 2 * 3;
		target->indexCount = indexSize;
		uint32_t* indices = new uint32_t[indexSize];
		int indicesIndex = 0;
		vertexIndex = 0;
		for (int i = 0; i < indexSize; i++)
		{
			indices[indicesIndex++] = vertexIndex++;
		}

		target->Vertices = vertecies;
		target->VB = Can::VertexBuffer::Create(target->Vertices, sizeof(float) * w * h * 2 * 3 * (3 + 4 + 3), true);
		target->VB->SetLayout({
		   { Can::ShaderDataType::Float3, "a_Position"},
		   { Can::ShaderDataType::Float4, "a_Color"},
		   { Can::ShaderDataType::Float3, "a_Normal"}
			});
		target->VA->AddVertexBuffer(target->VB);

		target->Indices = indices;
		target->IB = Can::IndexBuffer::Create(target->Indices, indexSize);
		target->VA->SetIndexBuffer(target->IB);

		target->S = Can::Shader::Create("assets/shaders/Cube.glsl");

		target->S->Bind();
		target->S->SetInt("u_Texture", 0);
		target->S->SetFloat3("u_LightPos", { 4.0f, 0.0f, 0.0f });

		target->position = { 0.0f, 0.0f, 0.0f };
		target->scale = { 1.0f, 1.0f, 1.0f };
		target->transform = glm::translate(glm::mat4(1.0f), target->position) * glm::scale(glm::mat4(1.0f), target->scale);
		target->w = width;
		target->h = height;
		Can::Renderer3D::AddObject(target);
		return target;
	}
}
