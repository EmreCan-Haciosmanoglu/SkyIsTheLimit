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

	bool RayTriangleIntersection(const glm::vec3& camPos, const glm::vec3& ray, const glm::vec3& A, const glm::vec3& B, const glm::vec3& C, const glm::vec3& normal, glm::vec3& intersection)
	{
		glm::vec3 u = (C - B) - (glm::dot(C - A, C - B) / glm::dot(C - A, C - A)) * (C - A);
		glm::vec3 v = (B - A) - (glm::dot(B - C, B - A) / glm::dot(B - C, B - C)) * (B - C);
		intersection = camPos + ray * (glm::dot(A - camPos, normal) / glm::dot(ray, normal));
		float a = 1 - glm::dot(v, intersection - A) / glm::dot(v, B - A);
		float b = 1 - glm::dot(u, intersection - B) / glm::dot(u, C - B);

		return a > 0 && a < 1 && b > 0 && b < 1 && a + b < 1;
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
		/*glm::vec2 AB = {
			endCoord.x - startCoord.x ,
			endCoord.z - startCoord.z
		};
		float mAB = glm::length(AB);
		glm::vec3 SE = endCoord - startCoord;

		float* data = terrain->prefab->vertices;

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
		terrain->VB->Unbind();*/
	}

	Ref<Prefab> GetPrefabForTerrain(const std::string& texturePath)
	{
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
					vertices[vertexIndex++] = x / TERRAIN_SCALE_DOWN;
					vertices[vertexIndex++] = z;
					vertices[vertexIndex++] = -(y / TERRAIN_SCALE_DOWN);

					vertices[vertexIndex++] = colors[heightIndex].r;
					vertices[vertexIndex++] = colors[heightIndex].g;
					vertices[vertexIndex++] = colors[heightIndex].b;
					vertices[vertexIndex++] = colors[heightIndex].a;

					vertexIndex += 3;
				}
				{
					float z = (p4[0] / 256.0f) * COLOR_COUNT;
					size_t heightIndex = z;
					vertices[vertexIndex++] = (x + 1) / TERRAIN_SCALE_DOWN;
					vertices[vertexIndex++] = z;
					vertices[vertexIndex++] = -(y / TERRAIN_SCALE_DOWN);

					vertices[vertexIndex++] = colors[heightIndex].r;
					vertices[vertexIndex++] = colors[heightIndex].g;
					vertices[vertexIndex++] = colors[heightIndex].b;
					vertices[vertexIndex++] = colors[heightIndex].a;

					vertexIndex += 3;
				}
				{
					float z = (p3[0] / 256.0f) * COLOR_COUNT;
					size_t heightIndex = z;
					vertices[vertexIndex++] = (x + 1) / TERRAIN_SCALE_DOWN;
					vertices[vertexIndex++] = z;
					vertices[vertexIndex++] = -((y + 1) / TERRAIN_SCALE_DOWN);

					vertices[vertexIndex++] = colors[heightIndex].r;
					vertices[vertexIndex++] = colors[heightIndex].g;
					vertices[vertexIndex++] = colors[heightIndex].b;
					vertices[vertexIndex++] = colors[heightIndex].a;

					vertexIndex += 3;
				}
				{
					float z = (p1[0] / 256.0f) * COLOR_COUNT;
					size_t heightIndex = z;
					vertices[vertexIndex++] = x / TERRAIN_SCALE_DOWN;
					vertices[vertexIndex++] = z;
					vertices[vertexIndex++] = -(y / TERRAIN_SCALE_DOWN);

					vertices[vertexIndex++] = colors[heightIndex].r;
					vertices[vertexIndex++] = colors[heightIndex].g;
					vertices[vertexIndex++] = colors[heightIndex].b;
					vertices[vertexIndex++] = colors[heightIndex].a;

					vertexIndex += 3;
				}
				{
					float z = (p3[0] / 256.0f) * COLOR_COUNT;
					size_t heightIndex = z;
					vertices[vertexIndex++] = (x + 1) / TERRAIN_SCALE_DOWN;
					vertices[vertexIndex++] = z;
					vertices[vertexIndex++] = -((y + 1) / TERRAIN_SCALE_DOWN);

					vertices[vertexIndex++] = colors[heightIndex].r;
					vertices[vertexIndex++] = colors[heightIndex].g;
					vertices[vertexIndex++] = colors[heightIndex].b;
					vertices[vertexIndex++] = colors[heightIndex].a;

					vertexIndex += 3;
				}
				{
					float z = (p2[0] / 256.0f) * COLOR_COUNT;
					size_t heightIndex = z;
					vertices[vertexIndex++] = x / TERRAIN_SCALE_DOWN;
					vertices[vertexIndex++] = z;
					vertices[vertexIndex++] = -((y + 1) / TERRAIN_SCALE_DOWN);

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

				glm::vec3 norm1 = glm::normalize(glm::cross(v1, u1));
				glm::vec3 norm2 = glm::normalize(glm::cross(v2, u2));

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
		terrainPrefab->boundingBoxL = { 0.0f, 0.0f, -(height / TERRAIN_SCALE_DOWN) };
		terrainPrefab->boundingBoxM = { width / TERRAIN_SCALE_DOWN, 1.0f * COLOR_COUNT, 0.0f };

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