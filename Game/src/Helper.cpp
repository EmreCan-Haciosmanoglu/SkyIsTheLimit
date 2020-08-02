#include "canpch.h"
#include "Helper.h"

namespace  Can::Helper
{
	bool CheckBoundingBoxHit(const glm::vec3& rayStartPoint, const glm::vec3& ray, const glm::vec3& least, const glm::vec3& most)
	{
		glm::vec3 leftPlaneCollisionPoint = Helper::RayPlaneIntersection(rayStartPoint, ray, { least.x, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f });
		glm::vec3 rigthPlaneCollisionPoint = Helper::RayPlaneIntersection(rayStartPoint, ray, { most.x, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f });

		glm::vec3 bottomPlaneCollisionPoint = Helper::RayPlaneIntersection(rayStartPoint, ray, { 0.0f, least.y, 0.0f }, { 0.0f, 1.0f, 0.0f });
		glm::vec3 topPlaneCollisionPoint = Helper::RayPlaneIntersection(rayStartPoint, ray, { 0.0f, most.y, 0.0f }, { 0.0f, 1.0f, 0.0f });

		glm::vec3 nearPlaneCollisionPoint = Helper::RayPlaneIntersection(rayStartPoint, ray, { 0.0f, 0.0f, least.z }, { 0.0f, 0.0f, 1.0f });
		glm::vec3 farPlaneCollisionPoint = Helper::RayPlaneIntersection(rayStartPoint, ray, { 0.0f, 0.0f, most.z }, { 0.0f, 0.0f, 1.0f });


		return
			(bottomPlaneCollisionPoint.x >= least.x && bottomPlaneCollisionPoint.x <= most.x && bottomPlaneCollisionPoint.z >= least.z && bottomPlaneCollisionPoint.z <= most.z) ||
			(topPlaneCollisionPoint.x >= least.x && topPlaneCollisionPoint.x <= most.x && topPlaneCollisionPoint.z >= least.z && topPlaneCollisionPoint.z <= most.z) ||
			(leftPlaneCollisionPoint.y >= least.y && leftPlaneCollisionPoint.y <= most.y && leftPlaneCollisionPoint.z >= least.z && leftPlaneCollisionPoint.z <= most.z) ||
			(rigthPlaneCollisionPoint.y >= least.y && rigthPlaneCollisionPoint.y <= most.y && rigthPlaneCollisionPoint.z >= least.z && rigthPlaneCollisionPoint.z <= most.z) ||
			(nearPlaneCollisionPoint.x >= least.x && nearPlaneCollisionPoint.x <= most.x && nearPlaneCollisionPoint.y >= least.y && nearPlaneCollisionPoint.y <= most.y) ||
			(farPlaneCollisionPoint.x >= least.x && farPlaneCollisionPoint.x <= most.x && farPlaneCollisionPoint.y >= least.y && farPlaneCollisionPoint.y <= most.y);
	}

	glm::vec3 RayPlaneIntersection(const glm::vec3& X, const glm::vec3& v, const glm::vec3& C, const glm::vec3& n)
	{
		glm::vec3 w = C - X;
		float k = glm::dot(w, n) / glm::dot(v, n);
		return X + k * v;
	}

	glm::vec2 LineSLineSIntersection(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3)
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

	bool RayTriangleIntersection(const glm::vec3& camPos, const glm::vec3& ray, const glm::vec3& A, const glm::vec3& B, const glm::vec3& C, const glm::vec3& normal)
	{
		glm::vec3 u = (C - B) - (glm::dot(C - A, C - B) / glm::dot(C - A, C - A)) * (C - A);
		glm::vec3 v = (B - A) - (glm::dot(B - C, B - A) / glm::dot(B - C, B - C)) * (B - C);
		glm::vec3 I = camPos + ray * (glm::dot(A - camPos, normal) / glm::dot(ray, normal));
		float a = 1 - glm::dot(v, I - A) / glm::dot(v, B - A);
		float b = 1 - glm::dot(u, I - B) / glm::dot(u, C - B);

		return a > 0 && a < 1 && b > 0 && b < 1 && a + b < 1;
	}

	Object* ConstructObject(const std::string& shaderPath, const std::string& texturePath, std::vector<glm::vec3>& vertices, std::vector<glm::vec2>& uvs, std::vector<glm::vec3>& normals)
	{
		Object* object = new  Object();
		object->VA = VertexArray::Create();
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
		object->VB = VertexBuffer::Create(m_Vertices, sizeof(float) * size, true);
		object->VB->SetLayout({
		   {  ShaderDataType::Float3, "a_Position"},
		   {  ShaderDataType::Float2, "a_UV"},
		   {  ShaderDataType::Float3, "a_Normal"}
			});

		object->VA->AddVertexBuffer(object->VB);

		uint32_t* m_Indices = new uint32_t[vSize];

		for (int i = 0; i < vSize; i++)
		{
			m_Indices[i] = i;
		}

		object->Indices = m_Indices;
		object->IB = IndexBuffer::Create(m_Indices, vSize);
		object->VA->SetIndexBuffer(object->IB);

		object->T = Texture2D::Create(texturePath);
		object->S = Shader::Create(shaderPath);

		object->S->Bind();
		object->S->SetInt("u_Texture", 0);
		object->S->SetFloat3("u_LightPos", { 1.0f, 1.0f, -1.0f });

		SetTransform(object, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f });


		return object;
	}

	std::vector<std::string> GetFiles(const std::string& folder, const std::string& filter, const std::string& fileType)
	{
		std::vector<std::string> files;
		namespace fs = std::filesystem;
		for (const auto& entry : std::filesystem::directory_iterator(folder))
		{
			std::string file = entry.path().string();

			if (file.find(filter) != std::string::npos)
			{
				if (file.find(fileType) != std::string::npos)
					files.push_back(file);
			}
		}
		return files;
	}

	void LevelTheTerrain(const glm::vec2& startIndex, const glm::vec2& endIndex, const glm::vec3& startCoord, const glm::vec3& endCoord, Object* terrain, float width)
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

	void GenerateTJunction(Object* roadP, Object* endP, Object* junctionP, int snappedRoadIndex, const glm::vec3& startCoord, const glm::vec3& junctionCoord, const std::string& shaderPath, const std::string& texturePath, std::vector<Road*>& roads)
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

		Object* roadAAd = new  Object();
		Object* roadBBd = new  Object();
		Object* roadCCd = new  Object();

		roadAAd->VA = VertexArray::Create();
		roadBBd->VA = VertexArray::Create();
		roadCCd->VA = VertexArray::Create();

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

		roadAAd->VB = VertexBuffer::Create(roadAAd->Vertices, sizeof(float) * roadAAd->indexCount * (3 + 2 + 3), true);
		roadBBd->VB = VertexBuffer::Create(roadBBd->Vertices, sizeof(float) * roadBBd->indexCount * (3 + 2 + 3), true);
		roadCCd->VB = VertexBuffer::Create(roadCCd->Vertices, sizeof(float) * roadCCd->indexCount * (3 + 2 + 3), true);

		roadBBd->VB->SetLayout({
		   {  ShaderDataType::Float3, "a_Position"},
		   {  ShaderDataType::Float2, "a_UV"},
		   {  ShaderDataType::Float3, "a_Normal"}
			});
		roadAAd->VB->SetLayout({
		   {  ShaderDataType::Float3, "a_Position"},
		   {  ShaderDataType::Float2, "a_UV"},
		   {  ShaderDataType::Float3, "a_Normal"}
			});
		roadCCd->VB->SetLayout({
		   {  ShaderDataType::Float3, "a_Position"},
		   {  ShaderDataType::Float2, "a_UV"},
		   {  ShaderDataType::Float3, "a_Normal"}
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

		roadAAd->IB = IndexBuffer::Create(AAdIndices, roadAAd->indexCount);
		roadBBd->IB = IndexBuffer::Create(BBdIndices, roadBBd->indexCount);
		roadCCd->IB = IndexBuffer::Create(CCdIndices, roadCCd->indexCount);

		roadAAd->VA->SetIndexBuffer(roadAAd->IB);
		roadBBd->VA->SetIndexBuffer(roadBBd->IB);
		roadCCd->VA->SetIndexBuffer(roadCCd->IB);

		roadAAd->T = Texture2D::Create(texturePath);
		roadBBd->T = Texture2D::Create(texturePath);
		roadCCd->T = Texture2D::Create(texturePath);

		roadAAd->S = Shader::Create(shaderPath);
		roadBBd->S = Shader::Create(shaderPath);
		roadCCd->S = Shader::Create(shaderPath);

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

		Renderer3D::AddObject(roadAAd);
		Renderer3D::AddObject(roadBBd);
		Renderer3D::AddObject(roadCCd);


		Renderer3D::DeleteObject(road->object);
		roads.erase(roads.begin() + snappedRoadIndex);

		std::array<glm::vec3, 2> arrAAd = {
			glm::vec3{ A.x, startCoord.y, A.y },
			glm::vec3{ A.x, startCoord.y, A.y } + (glm::normalize(glm::vec3{ AB.x, 0, AB.y }) * lengthAAd)
		};
		std::array<glm::vec3, 2> arrBBd = {
			glm::vec3{ B.x, startCoord.y, B.y },
					glm::vec3{ B.x, startCoord.y, B.y } - (glm::normalize(glm::vec3{ AB.x, 0, AB.y }) * lengthBBd)
		};
		std::array<glm::vec3, 2> arrCCd = {
			glm::vec3{ C.x, startCoord.y, C.y },
					glm::vec3{ C.x, startCoord.y, C.y } + (glm::normalize(glm::vec3{ CD.x, 0, CD.y }) * lengthCCd)
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

	void ReconstructRoad(Road* road, Object* prefab, const std::string& shaderPath, const std::string& texturePath)
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
			Renderer3D::DeleteObject(road->object);
			delete road->object;
		}

		Object* roadObject = new  Object();

		roadObject->VA = VertexArray::Create();

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

		roadObject->VB = VertexBuffer::Create(roadObject->Vertices, sizeof(float) * roadObject->indexCount * (3 + 2 + 3), true);

		roadObject->VB->SetLayout({
		   {  ShaderDataType::Float3, "a_Position"},
		   {  ShaderDataType::Float2, "a_UV"},
		   {  ShaderDataType::Float3, "a_Normal"}
			});

		roadObject->VA->AddVertexBuffer(roadObject->VB);

		uint32_t* roadIndices = new uint32_t[roadObject->indexCount];

		for (int i = 0; i < roadObject->indexCount; i++) roadIndices[i] = i;

		roadObject->Indices = roadIndices;

		roadObject->IB = IndexBuffer::Create(roadIndices, roadObject->indexCount);

		roadObject->VA->SetIndexBuffer(roadObject->IB);

		roadObject->T = Texture2D::Create(texturePath);
		roadObject->S = Shader::Create(shaderPath);

		roadObject->S->Bind();
		roadObject->S->SetInt("u_Texture", 0);
		roadObject->S->SetFloat3("u_LightPos", { 1.0f, 1.0f, -1.0f });

		float rotateAmount = glm::radians(angle);

		SetTransform(roadObject, road->startPos + glm::vec3{ 0.0f, 0.01f, 0.0f }, { 0.01f, 0.01f, 0.01f }, { 0.0f, rotateAmount, 0.0f });
		road->object = roadObject;
		Renderer3D::AddObject(roadObject);
	}

	Ref<Prefab> GetPrefabForTerrain(const std::string& texturePath)
	{
#define COLOR_COUNT 5
#define TEMP_TERRAIN_SHADER "assets/shaders/Cube.glsl"

		std::array<glm::vec4, COLOR_COUNT> colors = {
			glm::vec4{ 9.0f / 255.0f, 255.0f / 255.0f, 4.0f / 255.0f, 1.0f },
			glm::vec4{ 25.0f / 255.0f, 93.0f / 255.0f, 24.0f / 255.0f, 1.0f },
			glm::vec4{ 182.0f / 255.0f, 64.0f / 255.0f, 16.0f / 255.0f, 1.0f },
			glm::vec4{ 61.0f / 255.0f, 28.0f / 255.0f, 10.0f / 255.0f, 1.0f },
			glm::vec4{ 112.0f / 255.0f, 217.0f / 255.0f, 238.0f / 255.0f, 1.0f }
		};

		int width, height, channels;
		unsigned char* data = stbi_load(texturePath.c_str(), &width, &height, &channels, 0);

		size_t w = width - 1;
		size_t h = height - 1;

		size_t indexCount = w * h * 2 * 3;
		size_t vertexCount = indexCount * (3 + 4 + 3);
		float* vertices = new float[vertexCount];

		int vertexIndex = 0;
		for (size_t y = 0; y < h; y++)
		{
			for (size_t x = 0; x < w; x++)
			{
				unsigned char* p1 = &data[((x + 0) + (width) * (y + 0)) * channels];
				unsigned char* p2 = &data[((x + 0) + (width) * (y + 1)) * channels];
				unsigned char* p3 = &data[((x + 1) + (width) * (y + 1)) * channels];
				unsigned char* p4 = &data[((x + 1) + (width) * (y + 0)) * channels];

				{
					float z = (p1[0] / 256.0f) * COLOR_COUNT;
					size_t heightIndex = z;
					vertices[vertexIndex++] = x;
					vertices[vertexIndex++] = z;
					vertices[vertexIndex++] = -y;

					vertices[vertexIndex++] = colors[heightIndex].r;
					vertices[vertexIndex++] = colors[heightIndex].g;
					vertices[vertexIndex++] = colors[heightIndex].b;
					vertices[vertexIndex++] = colors[heightIndex].a;

					vertexIndex += 3;
				}
				{
					float z = (p4[0] / 256.0f) * COLOR_COUNT;
					size_t heightIndex = z;
					vertices[vertexIndex++] = (x + 1);
					vertices[vertexIndex++] = z;
					vertices[vertexIndex++] = -y;

					vertices[vertexIndex++] = colors[heightIndex].r;
					vertices[vertexIndex++] = colors[heightIndex].g;
					vertices[vertexIndex++] = colors[heightIndex].b;
					vertices[vertexIndex++] = colors[heightIndex].a;

					vertexIndex += 3;
				}
				{
					float z = (p3[0] / 256.0f) * COLOR_COUNT;
					size_t heightIndex = z;
					vertices[vertexIndex++] = (x + 1);
					vertices[vertexIndex++] = z;
					vertices[vertexIndex++] = -(y + 1);

					vertices[vertexIndex++] = colors[heightIndex].r;
					vertices[vertexIndex++] = colors[heightIndex].g;
					vertices[vertexIndex++] = colors[heightIndex].b;
					vertices[vertexIndex++] = colors[heightIndex].a;

					vertexIndex += 3;
				}
				{
					float z = (p1[0] / 256.0f) * COLOR_COUNT;
					size_t heightIndex = z;
					vertices[vertexIndex++] = x;
					vertices[vertexIndex++] = z;
					vertices[vertexIndex++] = -y;

					vertices[vertexIndex++] = colors[heightIndex].r;
					vertices[vertexIndex++] = colors[heightIndex].g;
					vertices[vertexIndex++] = colors[heightIndex].b;
					vertices[vertexIndex++] = colors[heightIndex].a;

					vertexIndex += 3;
				}
				{
					float z = (p3[0] / 256.0f) * COLOR_COUNT;
					size_t heightIndex = z;
					vertices[vertexIndex++] = (x + 1);
					vertices[vertexIndex++] = z;
					vertices[vertexIndex++] = -(y + 1);

					vertices[vertexIndex++] = colors[heightIndex].r;
					vertices[vertexIndex++] = colors[heightIndex].g;
					vertices[vertexIndex++] = colors[heightIndex].b;
					vertices[vertexIndex++] = colors[heightIndex].a;

					vertexIndex += 3;
				}
				{
					float z = (p2[0] / 256.0f) * COLOR_COUNT;
					size_t heightIndex = z;
					vertices[vertexIndex++] = x;
					vertices[vertexIndex++] = z;
					vertices[vertexIndex++] = -(y + 1);

					vertices[vertexIndex++] = colors[heightIndex].r;
					vertices[vertexIndex++] = colors[heightIndex].g;
					vertices[vertexIndex++] = colors[heightIndex].b;
					vertices[vertexIndex++] = colors[heightIndex].a;

					vertexIndex += 3;
				}
			}
		}
		vertexIndex = 0;
		for (float x = 0; x < w; x++)
		{
			for (float y = 0; y < h; y++)
			{
				glm::vec3 a00(vertices[vertexIndex + 0 + 0], vertices[vertexIndex + 0 + 1], vertices[vertexIndex + 0 + 2]);
				glm::vec3 a10(vertices[vertexIndex + 10 + 0], vertices[vertexIndex + 10 + 1], vertices[vertexIndex + 10 + 2]);
				glm::vec3 a11(vertices[vertexIndex + 20 + 0], vertices[vertexIndex + 20 + 1], vertices[vertexIndex + 20 + 2]);
				glm::vec3 a01(vertices[vertexIndex + 50 + 0], vertices[vertexIndex + 50 + 1], vertices[vertexIndex + 50 + 2]);

				glm::vec3 u1 = a11 - a00;
				glm::vec3 v1 = a10 - a00;

				glm::vec3 u2 = a01 - a00;
				glm::vec3 v2 = a11 - a00;

				glm::vec3 norm1 = glm::cross(v1, u1);
				glm::vec3 norm2 = glm::cross(v2, u2);

				vertices[vertexIndex + 0 + 7] = norm1.x;
				vertices[vertexIndex + 0 + 8] = norm1.y;
				vertices[vertexIndex + 0 + 9] = norm1.z;

				vertices[vertexIndex + 10 + 7] = norm1.x;
				vertices[vertexIndex + 10 + 8] = norm1.y;
				vertices[vertexIndex + 10 + 9] = norm1.z;

				vertices[vertexIndex + 20 + 7] = norm1.x;
				vertices[vertexIndex + 20 + 8] = norm1.y;
				vertices[vertexIndex + 20 + 9] = norm1.z;

				vertices[vertexIndex + 30 + 7] = norm2.x;
				vertices[vertexIndex + 30 + 8] = norm2.y;
				vertices[vertexIndex + 30 + 9] = norm2.z;

				vertices[vertexIndex + 40 + 7] = norm2.x;
				vertices[vertexIndex + 40 + 8] = norm2.y;
				vertices[vertexIndex + 40 + 9] = norm2.z;

				vertices[vertexIndex + 50 + 7] = norm2.x;
				vertices[vertexIndex + 50 + 8] = norm2.y;
				vertices[vertexIndex + 50 + 9] = norm2.z;
				vertexIndex += 60;
			}
		}

		Ref<Prefab> terrainPrefab = CreateRef<Prefab>("", TEMP_TERRAIN_SHADER, "", vertices, indexCount, vertexCount, BufferLayout{ { ShaderDataType::Float3, "a_Position"}, { ShaderDataType::Float4, "a_Color"}, { ShaderDataType::Float3, "a_Normal"} });
		terrainPrefab->boundingBoxL = { 0.0f, 0.0f, -height };
		terrainPrefab->boundingBoxM = { width, 1.0f * COLOR_COUNT, 0.0f };

		return terrainPrefab;
	}

	glm::vec2 RotateAPointAroundAPoint(const glm::vec2& p1, const glm::vec2& p2, float angleInRadians)
	{
		return glm::vec2{
			glm::cos(angleInRadians) * (p1.x - p2.x) - glm::sin(angleInRadians) * (p1.y - p2.y) + p2.x,
			glm::sin(angleInRadians) * (p1.x - p2.x) + glm::cos(angleInRadians) * (p1.y - p2.y) + p2.y
		};
	}
}