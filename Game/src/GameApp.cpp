#include "GameApp.h"

#include "../stb/stb_image.h"
#include "Can/EntryPoint.h"

#include <glm/gtx/vector_angle.hpp> 

Can::Application* Can::CreateApplication()
{
	return new GameApp();
}

namespace Can
{
	GameApp::GameApp()
		: testScene(new TestScene(this))
		, uiScene(new UIScene(this))
		, m_Terrain(new Can::Object())
	{
		//m_Terrain = UploadTerrain("assets/objects/heightmap_smallest.png");
		m_Terrain = UploadTerrain("assets/objects/flat_land.png");
		PushLayer(testScene);
		PushOverlay(uiScene);
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
		object->transform = glm::translate(glm::mat4(1.0f), object->position) * glm::rotate(glm::mat4(1.0f), object->rotation.y, { 0,1,0 }) * glm::scale(glm::mat4(1.0f), object->scale);

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


	glm::vec2 GameApp::LineSLineSIntersection(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3)
	{
		float s_numer, t_numer, denom, t;
		glm::vec2 s10 = p1 - p0;
		glm::vec2 s32 = p3 - p2;

		denom = s10.x * s32.y - s32.x * s10.y;
		if (denom == 0)
			return glm::vec2{ 0,0 }; // Collinear
		bool denomPositive = denom > 0;

		glm::vec2 s02 = p0 - p2;

		s_numer = s10.x * s02.y - s10.y * s02.x;
		if ((s_numer < 0) == denomPositive)
			return glm::vec2{ 0,0 }; // No collision

		t_numer = s32.x * s02.y - s32.y * s02.x;
		if ((t_numer < 0) == denomPositive)
			return glm::vec2{ 0,0 }; // No collision

		if (((s_numer > denom) == denomPositive) || ((t_numer > denom) == denomPositive))
			return glm::vec2{ 0,0 }; // No collision
		// Collision detected
		t = t_numer / denom;

		return p0 + (t * s10);
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
		return nullptr;
	}

	void GameApp::LevelTheTerrain(const glm::vec2& startIndex, const glm::vec2& endIndex, const glm::vec3& startCoord, const glm::vec3& endCoord, Can::Object* terrain, float width)
	{
		glm::vec2 AB = {
			endCoord.x - startCoord.x ,
			endCoord.z - startCoord.z
		};
		float mAB = glm::length(AB);
		glm::vec3 SE = endCoord - startCoord;

		float* data = terrain->Vertices;

		bool xC = startIndex.x < endIndex.x + 1;
		int xA = xC ? 1 : -1;
		bool yC = startIndex.y < endIndex.y + 1;
		int yA = yC ? 1 : -1;
		size_t yMin = startIndex.y - 2 * yA;
		size_t yMax = endIndex.y + 2 * yA;
		size_t xMin = startIndex.x - 2 * xA;
		size_t xMax = endIndex.x + 2 * xA;
		for (size_t y = yMin; (y < yMax) == yC; y += yA)
		{
			for (size_t x = xMin; (x < xMax) == xC; x += xA)
			{
				if (x < 0 || y < 0 || x >= terrain->w - 1 || y >= terrain->h - 1)
					continue;
				size_t dist1 = (x + (terrain->w - 1) * y) * 60;
				size_t dist2 = (x + (terrain->w - 1) * (y - 1)) * 60;
				glm::vec2 AP = {
					data[dist1] - startCoord.x,
					data[dist1 + 2] - startCoord.z
				};
				float mAP = glm::length(AP);
				float angle = glm::acos(glm::dot(AB, AP) / (mAP * mAB));
				float d = mAB * glm::sin(angle);

				if (d < width)
				{
					float c = mAB * glm::cos(angle);
					float yOffset = SE.y * (c * mAB);
					float yCoord = startCoord.y + yOffset;
					if (dist2 - 60 >= 0)
					{
						data[dist2 - 39] = yCoord;
						data[dist2 - 19] = yCoord;
						data[dist2 + 51] = yCoord;
					}
					data[dist1 - 49] = yCoord;
					data[dist1 + 1] = yCoord;
					data[dist1 + 31] = yCoord;
				}
			}
		}

		int vertexCount = terrain->indexCount * (3 + 4 + 3);
		terrain->VB->Bind();
		terrain->VB->ReDo(terrain->Vertices, sizeof(float) * vertexCount);
		terrain->VB->Unbind();
	}

	void GameApp::GenerateTJunction(Can::Object* roadP, Can::Object* endP, Can::Object* junctionP, int snappedRoadIndex, const glm::vec3& startCoord, const glm::vec3& junctionCoord, const std::string& shaderPath, const std::string& texturePath, std::vector<Road*>& roads)
	{
		glm::vec2 unitX = { 1.0f, 0.0f };
		Road* road = roads.at(snappedRoadIndex);

		// Find the length and width of a single road
		float maxRx = 0.0f;
		float minRx = 0.0f;
		float maxRz = 0.0f;
		float minRz = 0.0f;
		int RVSize = roadP->indexCount;
		float* roadVerticies = roadP->Vertices;
		for (size_t i = 0; i < RVSize; i++)
		{
			float x = roadVerticies[i * 8];
			float z = roadVerticies[i * 8 + 2];
			maxRx = std::max(maxRx, x);
			minRx = std::min(minRx, x);
			maxRz = std::max(maxRz, z);
			minRz = std::min(minRz, z);
		}
		float lengthRoad = maxRz - minRz;
		float widthRoad = maxRx - minRx;

		// Find the length of the crosswalk of the T junction
		float maxTJ = 0.0f;
		float maxDiagonal = 0.0f;
		int TJVSize = junctionP->indexCount;
		float* TJVerticies = junctionP->Vertices;
		for (size_t i = 0; i < TJVSize; i++)
		{
			float x = TJVerticies[i * 8];
			float z = TJVerticies[i * 8 + 2];
			maxTJ = std::max(maxTJ, z);
			if (x == z)
				maxDiagonal = std::max(maxDiagonal, x);
		}
		float lengthCrossWalk = maxTJ - maxDiagonal;

		glm::vec2 A = {
			road->startPos.x,
			road->startPos.z
		};
		glm::vec2 B = {
			road->endPos.x,
			road->endPos.z
		};
		glm::vec2 AB = B - A;

		glm::vec2 C{
			startCoord.x,
			startCoord.z
		};
		glm::vec2 D{
			junctionCoord.x,
			junctionCoord.z
		};
		glm::vec2 CD = D - C;

		float edA = AB.x <= 0.0f ? 180.0f : 0.0f;
		float edB = AB.x <= 0.0f ? 180.0f : 0.0f;
		float edC = CD.x <= 0.0f ? 180.0f : 0.0f;

		float angleAB = glm::degrees(glm::atan(-AB.y / AB.x)) + 90.0f + edA;
		float angleCD = glm::degrees(glm::atan(-CD.y / CD.x)) + 90.0f + edC;

		//float angleAB = AB.y < 0 ? glm::degrees(glm::angle(-AB, unitX)) + 180.0f : glm::degrees(glm::angle(AB, unitX));
		//float angleCD = CD.y < 0 ? glm::degrees(glm::angle(-CD, unitX)) + 180.0f : glm::degrees(glm::angle(CD, unitX));

		float diff = std::fmod(angleCD - angleAB + 180.0f, 180.0f);

		float rotateAmount = diff < 180.0f ? glm::radians(90.0f) : glm::radians(-90.0f);
		glm::vec2 shiftABAmount = glm::normalize(glm::rotate(AB, rotateAmount)) * (widthRoad / 100.0f) / 2.0f;
		glm::vec2 Ap = A + shiftABAmount;
		glm::vec2 Bp = B + shiftABAmount;

		glm::vec2 shiftCDAmount = glm::normalize(glm::rotate(CD, glm::radians(-90.0f))) * (widthRoad / 100.0f) / 2.0f;
		glm::vec2 Cp = C + shiftCDAmount;
		glm::vec2 Cn = C - shiftCDAmount;
		glm::vec2 Dp = D + shiftCDAmount;
		glm::vec2 Dn = D - shiftCDAmount;

		glm::vec2 I = LineSLineSIntersection(Ap, Bp, C, D);
		glm::vec2 Ip = LineSLineSIntersection(Ap, Bp, Cp, Dp);
		glm::vec2 In = LineSLineSIntersection(Ap, Bp, Cn, Dn);

		glm::vec2 Center = A + (I - Ap);

		float lengthAAd = glm::length((Center - A) - (I - Ip)) - (lengthCrossWalk / 100.0f);
		float lengthBBd = glm::length((Center - B) - (I - In)) - (lengthCrossWalk / 100.0f);
		float lengthCCd = glm::length((D - C) - (Dp - Ip)) - (lengthCrossWalk / 100.0f);


		int countAAd = (int)(lengthAAd / (lengthRoad / 100.0f));
		int countBBd = (int)(lengthBBd / (lengthRoad / 100.0f));
		int countCCd = (int)(lengthCCd / (lengthRoad / 100.0f));

		float scaleAAd = (lengthAAd / (lengthRoad / 100.0f)) / countAAd;
		float scaleBBd = (lengthBBd / (lengthRoad / 100.0f)) / countBBd;
		float scaleCCd = (lengthCCd / (lengthRoad / 100.0f)) / countCCd;

		Can::Object* roadAAd = new Can::Object();
		Can::Object* roadBBd = new Can::Object();
		Can::Object* roadCCd = new Can::Object();

		roadAAd->VA = Can::VertexArray::Create();
		roadBBd->VA = Can::VertexArray::Create();
		roadCCd->VA = Can::VertexArray::Create();

		int roadIndexCount = roadP->indexCount;

		roadAAd->indexCount = roadIndexCount * countAAd;
		roadBBd->indexCount = roadIndexCount * countBBd;
		roadCCd->indexCount = roadIndexCount * countCCd;

		float* AAdVertices = new float[roadAAd->indexCount * (3 + 2 + 3)];
		float* BBdVertices = new float[roadBBd->indexCount * (3 + 2 + 3)];
		float* CCdVertices = new float[roadCCd->indexCount * (3 + 2 + 3)];

		float* roadPVerticies = roadP->Vertices;
		for (int j = 0; j < countAAd; j++)
		{
			for (int i = 0; i < roadIndexCount; i++)
			{
				int offset = j * roadIndexCount * 8;
				int index = i * 8;
				AAdVertices[offset + index + 0] = roadPVerticies[index + 0];
				AAdVertices[offset + index + 1] = roadPVerticies[index + 1];
				AAdVertices[offset + index + 2] = (roadPVerticies[index + 2] + lengthRoad * (j + 0.5f)) * scaleAAd;
				AAdVertices[offset + index + 3] = roadPVerticies[index + 3];
				AAdVertices[offset + index + 4] = roadPVerticies[index + 4];
				AAdVertices[offset + index + 5] = roadPVerticies[index + 5];
				AAdVertices[offset + index + 6] = roadPVerticies[index + 6];
				AAdVertices[offset + index + 7] = roadPVerticies[index + 7];
			}
		}
		for (int j = 0; j < countBBd; j++)
		{
			for (int i = 0; i < roadIndexCount; i++)
			{
				int offset = j * roadIndexCount * 8;
				int index = i * 8;
				BBdVertices[offset + index + 0] = roadPVerticies[index + 0];
				BBdVertices[offset + index + 1] = roadPVerticies[index + 1];
				BBdVertices[offset + index + 2] = (roadPVerticies[index + 2] + lengthRoad * (j + 0.5f)) * scaleBBd;
				BBdVertices[offset + index + 3] = roadPVerticies[index + 3];
				BBdVertices[offset + index + 4] = roadPVerticies[index + 4];
				BBdVertices[offset + index + 5] = roadPVerticies[index + 5];
				BBdVertices[offset + index + 6] = roadPVerticies[index + 6];
				BBdVertices[offset + index + 7] = roadPVerticies[index + 7];
			}
		}
		for (int j = 0; j < countCCd; j++)
		{
			for (int i = 0; i < roadIndexCount; i++)
			{
				int offset = j * roadIndexCount * 8;
				int index = i * 8;
				CCdVertices[offset + index + 0] = roadPVerticies[index + 0];
				CCdVertices[offset + index + 1] = roadPVerticies[index + 1];
				CCdVertices[offset + index + 2] = (roadPVerticies[index + 2] + lengthRoad * (j + 0.5f)) * scaleCCd;
				CCdVertices[offset + index + 3] = roadPVerticies[index + 3];
				CCdVertices[offset + index + 4] = roadPVerticies[index + 4];
				CCdVertices[offset + index + 5] = roadPVerticies[index + 5];
				CCdVertices[offset + index + 6] = roadPVerticies[index + 6];
				CCdVertices[offset + index + 7] = roadPVerticies[index + 7];
			}
		}

		roadAAd->Vertices = AAdVertices;
		roadBBd->Vertices = BBdVertices;
		roadCCd->Vertices = CCdVertices;

		roadAAd->VB = Can::VertexBuffer::Create(roadAAd->Vertices, sizeof(float) * roadAAd->indexCount * (3 + 2 + 3), true);
		roadBBd->VB = Can::VertexBuffer::Create(roadBBd->Vertices, sizeof(float) * roadBBd->indexCount * (3 + 2 + 3), true);
		roadCCd->VB = Can::VertexBuffer::Create(roadCCd->Vertices, sizeof(float) * roadCCd->indexCount * (3 + 2 + 3), true);

		roadBBd->VB->SetLayout({
		   { Can::ShaderDataType::Float3, "a_Position"},
		   { Can::ShaderDataType::Float2, "a_UV"},
		   { Can::ShaderDataType::Float3, "a_Normal"}
			});
		roadAAd->VB->SetLayout({
		   { Can::ShaderDataType::Float3, "a_Position"},
		   { Can::ShaderDataType::Float2, "a_UV"},
		   { Can::ShaderDataType::Float3, "a_Normal"}
			});
		roadCCd->VB->SetLayout({
		   { Can::ShaderDataType::Float3, "a_Position"},
		   { Can::ShaderDataType::Float2, "a_UV"},
		   { Can::ShaderDataType::Float3, "a_Normal"}
			});

		roadAAd->VA->AddVertexBuffer(roadAAd->VB);
		roadBBd->VA->AddVertexBuffer(roadBBd->VB);
		roadCCd->VA->AddVertexBuffer(roadCCd->VB);

		uint32_t* AAdIndices = new uint32_t[roadAAd->indexCount];
		uint32_t* BBdIndices = new uint32_t[roadBBd->indexCount];
		uint32_t* CCdIndices = new uint32_t[roadCCd->indexCount];

		for (int i = 0; i < roadAAd->indexCount; i++) AAdIndices[i] = i;
		for (int i = 0; i < roadBBd->indexCount; i++) BBdIndices[i] = i;
		for (int i = 0; i < roadCCd->indexCount; i++) CCdIndices[i] = i;

		roadAAd->Indices = AAdIndices;
		roadBBd->Indices = BBdIndices;
		roadCCd->Indices = CCdIndices;

		roadAAd->IB = Can::IndexBuffer::Create(AAdIndices, roadAAd->indexCount);
		roadBBd->IB = Can::IndexBuffer::Create(BBdIndices, roadBBd->indexCount);
		roadCCd->IB = Can::IndexBuffer::Create(CCdIndices, roadCCd->indexCount);

		roadAAd->VA->SetIndexBuffer(roadAAd->IB);
		roadBBd->VA->SetIndexBuffer(roadBBd->IB);
		roadCCd->VA->SetIndexBuffer(roadCCd->IB);

		roadAAd->T = Can::Texture2D::Create(texturePath);
		roadBBd->T = Can::Texture2D::Create(texturePath);
		roadCCd->T = Can::Texture2D::Create(texturePath);

		roadAAd->S = Can::Shader::Create(shaderPath);
		roadBBd->S = Can::Shader::Create(shaderPath);
		roadCCd->S = Can::Shader::Create(shaderPath);

		roadAAd->S->Bind();
		roadAAd->S->SetInt("u_Texture", 0);
		roadAAd->S->SetFloat3("u_LightPos", { 1.0f, 1.0f, -1.0f });

		roadBBd->S->Bind();
		roadBBd->S->SetInt("u_Texture", 0);
		roadBBd->S->SetFloat3("u_LightPos", { 1.0f, 1.0f, -1.0f });

		roadCCd->S->Bind();
		roadCCd->S->SetInt("u_Texture", 0);
		roadCCd->S->SetFloat3("u_LightPos", { 1.0f, 1.0f, -1.0f });

		float rotateAAdAmount = glm::radians(angleAB);
		float rotateBBdAmount = glm::radians(angleAB + 180.0f);
		float rotateCCdAmount = glm::radians(angleCD);

		SetTransform(roadAAd, { A.x, startCoord.y, A.y }, { 0.01f, 0.01f, 0.01f }, { 0.0f, rotateAAdAmount, 0.0f });
		SetTransform(roadBBd, { B.x, startCoord.y, B.y }, { 0.01f, 0.01f, 0.01f }, { 0.0f, rotateBBdAmount, 0.0f });
		SetTransform(roadCCd, { C.x, startCoord.y, C.y }, { 0.01f, 0.01f, 0.01f }, { 0.0f, rotateCCdAmount, 0.0f });

		Can::Renderer3D::AddObject(roadAAd);
		Can::Renderer3D::AddObject(roadBBd);
		Can::Renderer3D::AddObject(roadCCd);


		Can::Renderer3D::DeleteObject(road->object);
		roads.erase(roads.begin() + snappedRoadIndex);

		std::array<glm::vec3, 2> arrAAd = {
			glm::vec3{ A.x, startCoord.y, A.y },
			glm::vec3{ A.x, startCoord.y, A.y } +(glm::normalize(glm::vec3{ AB.x, 0, AB.y }) * lengthAAd)
		};
		std::array<glm::vec3, 2> arrBBd = {
			glm::vec3{ B.x, startCoord.y, B.y },
					glm::vec3{ B.x, startCoord.y, B.y } -(glm::normalize(glm::vec3{ AB.x, 0, AB.y }) * lengthBBd)
		};
		std::array<glm::vec3, 2> arrCCd = {
			glm::vec3{ C.x, startCoord.y, C.y },
					glm::vec3{ C.x, startCoord.y, C.y } +(glm::normalize(glm::vec3{ CD.x, 0, CD.y }) * lengthCCd)
		};

		std::array<glm::vec2, 2> arrAAdIndex = {
			glm::vec2{ glm::abs((int)arrAAd[0].x * 50.f), glm::abs((int)arrAAd[0].z * 50.f) },
			glm::vec2{ glm::abs((int)arrAAd[1].x * 50.f), glm::abs((int)arrAAd[1].z * 50.f) },
		};

		std::array<glm::vec2, 2> arrBBdIndex = {
			glm::vec2{ glm::abs((int)arrBBd[0].x * 50.f), glm::abs((int)arrBBd[0].z * 50.f) },
			glm::vec2{ glm::abs((int)arrBBd[1].x * 50.f), glm::abs((int)arrBBd[1].z * 50.f) },
		};

		std::array<glm::vec2, 2> arrCCdIndex = {
			glm::vec2{ glm::abs((int)arrCCd[0].x * 50.f), glm::abs((int)arrCCd[0].z * 50.f) },
			glm::vec2{ glm::abs((int)arrCCd[1].x * 50.f), glm::abs((int)arrCCd[1].z * 50.f) },
		};

		roads.push_back(new Road{ arrAAd[0], arrAAd[1], roadAAd, nullptr, nullptr });
		roads.push_back(new Road{ arrBBd[0], arrBBd[1], roadBBd, nullptr, nullptr });
		roads.push_back(new Road{ arrCCd[0], arrCCd[1], roadCCd, nullptr, nullptr });

		LevelTheTerrain(arrAAdIndex[0], arrAAdIndex[1], arrAAd[0], arrAAd[1], m_Terrain, testScene->roadPrefabWidth / 200.0f);
		LevelTheTerrain(arrBBdIndex[0], arrBBdIndex[1], arrBBd[0], arrBBd[1], m_Terrain, testScene->roadPrefabWidth / 200.0f);
		LevelTheTerrain(arrCCdIndex[0], arrCCdIndex[1], arrCCd[0], arrCCd[1], m_Terrain, testScene->roadPrefabWidth / 200.0f);
	}

	void GameApp::UpdateTheJunction(Junction* junction, Can::Object* prefab, const std::string& shaderPath, const std::string& texturePath)
	{
		float maxJZ = 0.0f;
		float minJZ = 0.0f;
		float maxJX = 0.0f;
		float minJX = 0.0f;
		int JVSize = prefab->indexCount;
		float* JVerticies = prefab->Vertices;
		for (size_t i = 0; i < JVSize; i++)
		{
			float x = JVerticies[i * 8];
			float z = JVerticies[i * 8 + 2];

			maxJZ = std::max(maxJZ, z);
			minJZ = std::min(minJZ, z);

			maxJX = std::max(maxJX, x);
			minJX = std::min(minJX, x);
		}
		float length = maxJZ - minJZ;
		float width = maxJX - minJX;

		glm::vec2 juncPos = {
			junction->position.x ,
			junction->position.z
		};

		std::sort(junction->connectedRoads.begin(), junction->connectedRoads.end(), sort_with_angle());

		std::vector<glm::vec2> Intersections;

		size_t roadCount = junction->connectedRoads.size();
		for (size_t i = 0; i < roadCount; i++)
		{
			size_t iNext = (i + 1) % roadCount;
			Road* r1 = junction->connectedRoads[i];
			Road* r2 = junction->connectedRoads[iNext];

			glm::vec2 R0_1 = junction == r1->startJunction ? glm::vec2{ r1->startPos.x, r1->startPos.z } : glm::vec2{ r1->endPos.x, r1->endPos.z };
			glm::vec2 R1_1 = junction == r1->startJunction ? glm::vec2{ r1->endPos.x, r1->endPos.z } : glm::vec2{ r1->startPos.x, r1->startPos.z };
			glm::vec2 R0_2 = junction == r2->startJunction ? glm::vec2{ r2->startPos.x, r2->startPos.z } : glm::vec2{ r2->endPos.x, r2->endPos.z };
			glm::vec2 R1_2 = junction == r2->startJunction ? glm::vec2{ r2->endPos.x, r2->endPos.z } : glm::vec2{ r2->startPos.x, r2->startPos.z };


			glm::vec2 R0R1_1 = R1_1 - R0_1;
			glm::vec2 R0R1_2 = R1_2 - R0_2;

			float ed1 = R0R1_1.x <= 0.0f ? 180.0f : 0.0f;
			float ed2 = R0R1_2.x <= 0.0f ? 180.0f : 0.0f;

			float angleR0R1_1 = glm::degrees(glm::atan(-R0R1_1.y / R0R1_1.x)) + ed1;
			float angleR0R1_2 = glm::degrees(glm::atan(-R0R1_2.y / R0R1_2.x)) + ed2;

			glm::vec2 shiftR0R1_1Amount = glm::normalize(glm::rotate(R0R1_1, glm::radians(-90.0f))) * (width / 100.0f) / 2.0f;
			glm::vec2 shiftR0R1_2Amount = glm::normalize(glm::rotate(R0R1_2, glm::radians(90.0f))) * (width / 100.0f) / 2.0f;

			glm::vec3 R0_1d = {
				R0_1.x + shiftR0R1_1Amount.x,
				R0_1.y + shiftR0R1_1Amount.y,
				0
			};
			glm::vec2 R1_1d = {
				R1_1.x + shiftR0R1_1Amount.x,
				R1_1.y + shiftR0R1_1Amount.y
			};
			glm::vec3 R0_2d = {
				R0_2.x + shiftR0R1_2Amount.x,
				R0_2.y + shiftR0R1_2Amount.y,
				0
			};

			glm::vec3 R0R1_1V = {
				R0R1_1.x,
				R0R1_1.y,
				0.0f
			};
			glm::vec3 R0R1_2V = {
				R0R1_2.x,
				R0R1_2.y,
				0.0f
			};
			
			R0R1_2V = glm::rotate(R0R1_2V, glm::radians(90.0f), { 0.0f, 0.0f, 1.0f });
			glm::vec3 I = RayPlaneIntersection(R0_1d, R0R1_1V, R0_2d, R0R1_2V);

			Intersections.push_back(glm::vec2{ I.x, I.y });
		}

		if (junction->object != nullptr)
		{
			Can::Renderer3D::DeleteObject(junction->object);
			delete junction->object;
		}

		Can::Object* junctionObject = new Can::Object();
		junction->object = junctionObject;

		junctionObject->VA = Can::VertexArray::Create();

		size_t prefabIndexCount = prefab->indexCount;
		junctionObject->indexCount = prefabIndexCount * roadCount;

		float* junctionVertices = new float[junctionObject->indexCount * (3 + 2 + 3)];
		float* prefabVerticies = prefab->Vertices;
		glm::vec2 center = { 0.0f, 0.0f };
		for (size_t i = 0; i < roadCount; i++)
		{
			Road* r = junction->connectedRoads[i];
			glm::vec2 intersection1 = Intersections[i];
			glm::vec2 intersection2 = Intersections[(roadCount + i - 1) % roadCount];

			glm::vec2 R0 = junction == r->startJunction ? glm::vec2{ r->startPos.x, r->startPos.z } : glm::vec2{ r->endPos.x, r->endPos.z };
			glm::vec2 R1 = junction == r->startJunction ? glm::vec2{ r->endPos.x, r->endPos.z } : glm::vec2{ r->startPos.x, r->startPos.z };

			glm::vec2 JR1 = R1 - juncPos;
			float lJR1 = glm::length(JR1);
			glm::vec2 shiftAmount = glm::normalize(glm::rotate(JR1, glm::radians(90.0f))) * (width / 100.0f) / 2.0f;

			glm::vec2 R1p = R1 + shiftAmount;
			glm::vec2 R1n = R1 - shiftAmount;

			float lengthN = lJR1 - glm::length(R1n - intersection1);
			float lengthP = lJR1 - glm::length(R1p - intersection2);
			float l = 0.0f;
			if (lengthN < lengthP)
			{
				float size = glm::length(R1p - intersection2) - (length / 100.0f);
				glm::vec2 temp = R1 - glm::normalize(JR1) * size;
				l = lengthP;
				if (junction == r->startJunction)
				{
					r->startPos.x = temp.x;
					r->startPos.z = temp.y;
				}
				else
				{
					r->endPos.x = temp.x;
					r->endPos.z = temp.y;
				}
			}
			else
			{
				float size = glm::length(R1n - intersection1) - (length / 100.0f);
				glm::vec2 temp = R1 - glm::normalize(JR1) * size;
				l = lengthN;
				if (junction == r->startJunction)
				{
					r->startPos.x = temp.x;
					r->startPos.z = temp.y;
				}
				else
				{
					r->endPos.x = temp.x;
					r->endPos.z = temp.y;
				}
			}
			
			ReconstructRoad(r, testScene->roadPrefab, "assets/shaders/Object.glsl", "assets/objects/road.png");

			float ed = JR1.x <= 0.0f ? 180.0f : 0.0f;
			float angle = glm::radians(glm::degrees(glm::atan(-JR1.y / JR1.x)) + ed + 90.0f);

			for (size_t j = 0; j < prefabIndexCount; j++)
			{
				size_t offset = i * prefabIndexCount * 8;
				size_t index = j * 8;
				glm::vec2 point = {
					prefabVerticies[index + 0],
					prefabVerticies[index + 2]
				};
				if (prefabVerticies[index + 2] < 0.001f && prefabVerticies[index + 2] > -0.001f)
				{
					float percent = std::abs(point.x / (width / 2.0f));
					if (prefabVerticies[index + 0] < 0.0f)
						point.y += percent * lengthP * 100.0f;
					else if (prefabVerticies[index + 0] > 0.0f)
						point.y += percent * lengthN * 100.0f;
				}
				else
				{
					point.y += l * 100.0f;
				}

				glm::vec2 rotatedPoint = RotateAPointAroundAPoint(point, center, -angle);

				junctionVertices[offset + index + 0] = rotatedPoint.x;
				junctionVertices[offset + index + 1] = prefabVerticies[index + 1];
				junctionVertices[offset + index + 2] = rotatedPoint.y;
				junctionVertices[offset + index + 3] = prefabVerticies[index + 3];
				junctionVertices[offset + index + 4] = prefabVerticies[index + 4];
				junctionVertices[offset + index + 5] = prefabVerticies[index + 5];
				junctionVertices[offset + index + 6] = prefabVerticies[index + 6];
				junctionVertices[offset + index + 7] = prefabVerticies[index + 7];
			}
		}

		junctionObject->Vertices = junctionVertices;

		junctionObject->VB = Can::VertexBuffer::Create(junctionObject->Vertices, sizeof(float) * junctionObject->indexCount * (3 + 2 + 3), true);

		junctionObject->VB->SetLayout({
		   { Can::ShaderDataType::Float3, "a_Position"},
		   { Can::ShaderDataType::Float2, "a_UV"},
		   { Can::ShaderDataType::Float3, "a_Normal"}
			});

		junctionObject->VA->AddVertexBuffer(junctionObject->VB);

		uint32_t* junctionIndices = new uint32_t[junctionObject->indexCount];

		for (int i = 0; i < junctionObject->indexCount; i++) junctionIndices[i] = i;

		junctionObject->Indices = junctionIndices;

		junctionObject->IB = Can::IndexBuffer::Create(junctionIndices, junctionObject->indexCount);

		junctionObject->VA->SetIndexBuffer(junctionObject->IB);

		junctionObject->T = Can::Texture2D::Create(texturePath);

		junctionObject->S = Can::Shader::Create(shaderPath);

		junctionObject->S->Bind();
		junctionObject->S->SetInt("u_Texture", 0);
		junctionObject->S->SetFloat3("u_LightPos", { 1.0f, 1.0f, -1.0f });

		SetTransform(junctionObject, junction->position + glm::vec3{ 0.0f, 0.01f, 0.0f }, { 0.01f, 0.01f, 0.01f });
		Can::Renderer3D::AddObject(junctionObject);
	}

	void GameApp::ReconstructRoad(Road* road, Can::Object* prefab, const std::string& shaderPath, const std::string& texturePath)
	{
		float maxRz = 0.0f;
		float minRz = 0.0f;
		int RVSize = prefab->indexCount;
		float* roadVerticies = prefab->Vertices;
		for (size_t i = 0; i < RVSize; i++)
		{
			float z = roadVerticies[i * 8 + 2];
			maxRz = std::max(maxRz, z);
			minRz = std::min(minRz, z);
		}
		float lengthRoad = maxRz - minRz;


		glm::vec2 A = {
			road->startPos.x,
			road->startPos.z
		};
		glm::vec2 B = {
			road->endPos.x,
			road->endPos.z
		};
		glm::vec2 AB = B - A;


		float edA = AB.x <= 0.0f ? 180.0f : 0.0f;
		float angle = glm::degrees(glm::atan(-AB.y / AB.x)) + 90.0f + edA;

		float lengthAB = glm::length(AB);

		int count = (int)(lengthAB / (lengthRoad / 100.0f));
		float scale = (lengthAB / (lengthRoad / 100.0f)) / count;

		if (road->object != nullptr)
		{
			Can::Renderer3D::DeleteObject(road->object);
			delete road->object;
		}

		Can::Object* roadObject = new Can::Object();

		roadObject->VA = Can::VertexArray::Create();

		int roadIndexCount = prefab->indexCount;

		roadObject->indexCount = roadIndexCount * count;

		float* roadVertices = new float[roadObject->indexCount * (3 + 2 + 3)];

		float* prefabVerticies = prefab->Vertices;

		for (int j = 0; j < count; j++)
		{
			for (int i = 0; i < roadIndexCount; i++)
			{
				int offset = j * roadIndexCount * 8;
				int index = i * 8;
				roadVertices[offset + index + 0] = prefabVerticies[index + 0];
				roadVertices[offset + index + 1] = prefabVerticies[index + 1];
				roadVertices[offset + index + 2] = (prefabVerticies[index + 2] + lengthRoad * (j + 0.5f)) * scale;
				roadVertices[offset + index + 3] = prefabVerticies[index + 3];
				roadVertices[offset + index + 4] = prefabVerticies[index + 4];
				roadVertices[offset + index + 5] = prefabVerticies[index + 5];
				roadVertices[offset + index + 6] = prefabVerticies[index + 6];
				roadVertices[offset + index + 7] = prefabVerticies[index + 7];
			}
		}

		roadObject->Vertices = roadVertices;

		roadObject->VB = Can::VertexBuffer::Create(roadObject->Vertices, sizeof(float) * roadObject->indexCount * (3 + 2 + 3), true);

		roadObject->VB->SetLayout({
		   { Can::ShaderDataType::Float3, "a_Position"},
		   { Can::ShaderDataType::Float2, "a_UV"},
		   { Can::ShaderDataType::Float3, "a_Normal"}
			});

		roadObject->VA->AddVertexBuffer(roadObject->VB);

		uint32_t* roadIndices = new uint32_t[roadObject->indexCount];

		for (int i = 0; i < roadObject->indexCount; i++) roadIndices[i] = i;

		roadObject->Indices = roadIndices;

		roadObject->IB = Can::IndexBuffer::Create(roadIndices, roadObject->indexCount);

		roadObject->VA->SetIndexBuffer(roadObject->IB);

		roadObject->T = Can::Texture2D::Create(texturePath);
		roadObject->S = Can::Shader::Create(shaderPath);

		roadObject->S->Bind();
		roadObject->S->SetInt("u_Texture", 0);
		roadObject->S->SetFloat3("u_LightPos", { 1.0f, 1.0f, -1.0f });

		float rotateAmount = glm::radians(angle);

		SetTransform(roadObject, road->startPos + glm::vec3{ 0.0f, 0.01f, 0.0f }, { 0.01f, 0.01f, 0.01f }, { 0.0f, rotateAmount, 0.0f });
		road->object = roadObject;
		Can::Renderer3D::AddObject(roadObject);
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
		glm::vec3 color1 = { 9.0f / 255.0f, 255.0f / 255.0f, 4.0f / 255.0f };
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
