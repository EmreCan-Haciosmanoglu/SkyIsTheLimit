#include "GameApp.h"

#include "../stb/stb_image.h"
#include "Can/EntryPoint.h"

Can::Application* Can::CreateApplication()
{
	return new GameApp();
}

GameApp::GameApp()
	: testScene(new TestScene(this))
	, m_TestObject(nullptr)
	, m_Terrain(new Can::Object())
{
	UploadObject(m_TestObject, "assets/objects/padlock.obj", "assets/shaders/Object.glsl", "assets/objects/padlock.png");
	UploadTerrain(m_Terrain, "assets/objects/heightmap.png");
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

Can::Object* GameApp::ConstructObject(const std::string& shaderPath, const std::string& texturePath, std::vector<glm::vec3>& vertices, std::vector<glm::vec2>& uvs, std::vector<glm::vec3>& normals)
{
	Can::Object* object = new Can::Object();
	object->VA = Can::VertexArray::Create();

	int vSize = vertices.size();
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

	object->IB = Can::IndexBuffer::Create(m_Indices, vSize);
	object->VA->SetIndexBuffer(object->IB);

	object->T = Can::Texture2D::Create(texturePath);
	object->S = Can::Shader::Create(shaderPath);

	object->S->Bind();
	object->S->SetInt("u_Texture", 0);
	object->S->SetFloat3("u_LightPos", { 4.0f, 0.0f, 0.0f });

	SetTransform(object, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f });

	delete[] m_Vertices;
	delete[] m_Indices;

	return object;
}

void GameApp::UploadObject(Can::Object* target, const char* objectPath, const std::string& shaderPath, const std::string& texturePath)
{
	std::vector< glm::vec3 > vertices;
	std::vector< glm::vec2 > uvs;
	std::vector< glm::vec3 > normals;
	bool res = loadOBJ(objectPath, vertices, uvs, normals);
	if (res)
	{
		target = ConstructObject(shaderPath, texturePath, vertices, uvs, normals);
		Can::Renderer3D::AddObject(target);
	}
}

void GameApp::UploadTerrain(Can::Object* target, const std::string& texturePath)
{
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
	for (int x = 0; x < w; x++)
	{
		for (int y = 0; y < h; y++)
		{
			unsigned char* p1 = &data[((x + 0) + (h+1) * (y + 0)) * channels];
			unsigned char* p2 = &data[((x + 1) + (h+1) * (y + 0)) * channels];
			unsigned char* p3 = &data[((x + 1) + (h+1) * (y + 1)) * channels];
			unsigned char* p4 = &data[((x + 0) + (h+1) * (y + 1)) * channels];
			{
				z = p1[0] / 255.0f;
				vertecies[vertexIndex++] = x / 50.f;
				vertecies[vertexIndex++] = z;
				vertecies[vertexIndex++] = y / 50.f;

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
				vertecies[vertexIndex++] = x / 50.f;
				vertecies[vertexIndex++] = z;
				vertecies[vertexIndex++] = (y + 1) / 50.f;

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
				vertecies[vertexIndex++] = (y + 1) / 50.f;

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
				vertecies[vertexIndex++] = y / 50.f;

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
				vertecies[vertexIndex++] = (y + 1) / 50.f;

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
				vertecies[vertexIndex++] = (x + 1) / 50.f;
				vertecies[vertexIndex++] = z;
				vertecies[vertexIndex++] = y / 50.f;

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

	uint32_t* indices = new uint32_t[w * h * 2 * 3];
	int indicesIndex = 0;
	vertexIndex = 0;
	for (int j = 0; j < w; j++)
	{
		for (int i = 0; i < h; i++)
		{
			indices[indicesIndex++] = vertexIndex++;
			indices[indicesIndex++] = vertexIndex++;
			indices[indicesIndex++] = vertexIndex++;
			indices[indicesIndex++] = vertexIndex++;
			indices[indicesIndex++] = vertexIndex++;
			indices[indicesIndex++] = vertexIndex++;
		}
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
	target->IB = Can::IndexBuffer::Create(target->Indices, w * h * 2 * 3);
	target->VA->SetIndexBuffer(target->IB);

	target->S = Can::Shader::Create("assets/shaders/Cube.glsl");

	target->S->Bind();
	target->S->SetInt("u_Texture", 0);
	target->S->SetFloat3("u_LightPos", { 4.0f, 0.0f, 0.0f });

	target->position = { 0.0f, 0.0f, -4.0f };
	target->scale = { 0.5f, 0.5f, 0.5f };
	target->transform = glm::translate(glm::mat4(1.0f), target->position) * glm::scale(glm::mat4(1.0f), target->scale);

	Can::Renderer3D::AddObject(target);
}
