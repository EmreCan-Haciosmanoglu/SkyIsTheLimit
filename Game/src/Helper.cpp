#include "canpch.h"
#include "Helper.h"

#include "Scenes/GameScene.h"

namespace  Can::Helper
{
	bool CheckBoundingBoxHit(const v3& rayStartPoint, const v3& ray, const v3& least, const v3& most)
	{
		v3 leftPlaneCP = Helper::RayPlaneIntersection(rayStartPoint, ray, { least.x, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f });
		v3 rigthPlaneCP = Helper::RayPlaneIntersection(rayStartPoint, ray, { most.x, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f });

		v3 bottomPlaneCP = Helper::RayPlaneIntersection(rayStartPoint, ray, { 0.0f, least.y, 0.0f }, { 0.0f, 1.0f, 0.0f });
		v3 topPlaneCP = Helper::RayPlaneIntersection(rayStartPoint, ray, { 0.0f, most.y, 0.0f }, { 0.0f, 1.0f, 0.0f });

		v3 nearPlaneCP = Helper::RayPlaneIntersection(rayStartPoint, ray, { 0.0f, 0.0f, least.z }, { 0.0f, 0.0f, 1.0f });
		v3 farPlaneCP = Helper::RayPlaneIntersection(rayStartPoint, ray, { 0.0f, 0.0f, most.z }, { 0.0f, 0.0f, 1.0f });


		return
			(bottomPlaneCP.x >= least.x && bottomPlaneCP.x <= most.x && bottomPlaneCP.z >= least.z && bottomPlaneCP.z <= most.z) ||
			(topPlaneCP.x >= least.x && topPlaneCP.x <= most.x && topPlaneCP.z >= least.z && topPlaneCP.z <= most.z) ||
			(leftPlaneCP.y >= least.y && leftPlaneCP.y <= most.y && leftPlaneCP.z >= least.z && leftPlaneCP.z <= most.z) ||
			(rigthPlaneCP.y >= least.y && rigthPlaneCP.y <= most.y && rigthPlaneCP.z >= least.z && rigthPlaneCP.z <= most.z) ||
			(nearPlaneCP.x >= least.x && nearPlaneCP.x <= most.x && nearPlaneCP.y >= least.y && nearPlaneCP.y <= most.y) ||
			(farPlaneCP.x >= least.x && farPlaneCP.x <= most.x && farPlaneCP.y >= least.y && farPlaneCP.y <= most.y);
	}

	//delete later
	std::array<v2, 4> getAxis(const std::array<v2, 4>& c1, const std::array<v2, 4>& c2)
	{
		return std::array<v2, 4>{
			glm::normalize(c1[1] - c1[0]) ,
			glm::normalize(c1[3] - c1[0]) ,
			glm::normalize(c2[1] - c2[0]) ,
			glm::normalize(c2[3] - c2[0])
		};
	}

	v2 CheckRotatedRectangleCollision(const v2& r1l, const v2& r1m, f32 rot1, const v2& pos1, const v2& r2l, const v2& r2m, f32 rot2, const v2& pos2)
	{
		std::array<v2, 4> rotated_rect1 = {
			RotateAPointAroundAPoint(v2{ r1l.x, r1l.y }, -rot1) + pos1,
			RotateAPointAroundAPoint(v2{ r1l.x, r1m.y }, -rot1) + pos1,
			RotateAPointAroundAPoint(v2{ r1m.x, r1m.y }, -rot1) + pos1,
			RotateAPointAroundAPoint(v2{ r1m.x, r1l.y }, -rot1) + pos1
		};
		std::array<v2, 4> rotated_rect2 = {
			RotateAPointAroundAPoint(v2{ r2l.x, r2l.y }, -rot2) + pos2,
			RotateAPointAroundAPoint(v2{ r2l.x, r2m.y }, -rot2) + pos2,
			RotateAPointAroundAPoint(v2{ r2m.x, r2m.y }, -rot2) + pos2,
			RotateAPointAroundAPoint(v2{ r2m.x, r2l.y }, -rot2) + pos2
		};

		std::array<v2, 4> axis = getAxis(rotated_rect1, rotated_rect2);
		std::array<v2, 4> mtvs;
		for (u64 i = 0; i < 4; i++)
		{
			f32 scalers1[] = {
				glm::dot(axis[i], rotated_rect1[0]),
				glm::dot(axis[i], rotated_rect1[1]),
				glm::dot(axis[i], rotated_rect1[2]),
				glm::dot(axis[i], rotated_rect1[3])
			};
			f32 scalers2[] = {
				glm::dot(axis[i], rotated_rect2[0]),
				glm::dot(axis[i], rotated_rect2[1]),
				glm::dot(axis[i], rotated_rect2[2]),
				glm::dot(axis[i], rotated_rect2[3])
			};

			f32 s1max = *(std::max_element(scalers1, scalers1 + 4));
			f32 s1min = *(std::min_element(scalers1, scalers1 + 4));

			f32 s2max = *(std::max_element(scalers2, scalers2 + 4));
			f32 s2min = *(std::min_element(scalers2, scalers2 + 4));

			if (s1min >= s2max || s2min >= s1max)
				return v2(0.0f);
			f32 overlap = s1max > s2max ? s1min - s2max : s1max - s2min;

			mtvs[i] = axis[i] * overlap;
		}
		struct less_than_key
		{
			inline bool operator() (const v2& v1, const v2& v2)
			{
				return (glm::length(v1) < glm::length(v2));
			}
		};
		std::sort(mtvs.begin(), mtvs.end(), less_than_key());
		return mtvs[0];
	}

	v3 GetRayHitPointOnTerrain(void* s, const v3& cameraPosition, const v3& cameraDirection)
	{
		Can::GameScene* scene = (Can::GameScene*)s; //I have no idea how to change this

		f32* data = scene->m_Terrain->prefab->vertices;
		v3 bottomPlaneCollisionPoint = Helper::RayPlaneIntersection(cameraPosition, cameraDirection, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });
		v3 topPlaneCollisionPoint = Helper::RayPlaneIntersection(cameraPosition, cameraDirection, { 0.0f, 1.0f * COLOR_COUNT, 0.0f }, { 0.0f, 1.0f, 0.0f });

		bottomPlaneCollisionPoint.z *= -1;
		topPlaneCollisionPoint.z *= -1;

		f32 terrainW = scene->m_Terrain->prefab->boundingBoxM.x * TERRAIN_SCALE_DOWN;
		f32 terrainH = -scene->m_Terrain->prefab->boundingBoxL.z * TERRAIN_SCALE_DOWN;

		v2 minCoord = {
			std::max(0.0f, TERRAIN_SCALE_DOWN * (std::min(bottomPlaneCollisionPoint.x, topPlaneCollisionPoint.x) - scene->m_Terrain->position.x) - 1),
			std::max(0.0f, TERRAIN_SCALE_DOWN * (std::min(bottomPlaneCollisionPoint.z, topPlaneCollisionPoint.z) - scene->m_Terrain->position.z) - 1)
		};

		v2 maxCoord = {
			std::min(terrainW, TERRAIN_SCALE_DOWN * (std::max(bottomPlaneCollisionPoint.x, topPlaneCollisionPoint.x) - scene->m_Terrain->position.x) + 1),
			std::min(terrainH, TERRAIN_SCALE_DOWN * (std::max(bottomPlaneCollisionPoint.z, topPlaneCollisionPoint.z) - scene->m_Terrain->position.z) + 1)
		};
		for (u64 y = (u64)(minCoord.y); y < maxCoord.y; y++)
		{
			for (u64 x = (u64)(minCoord.x); x < maxCoord.x; x++)
			{
				for (u64 z = 0; z < 2; z++)
				{
					u64 index = (x + ((int)terrainW - 1) * y) * 60 + z * 30;
					f32* A = &data[index + 0];
					f32* B = &data[index + 10];
					f32* C = &data[index + 20];
					v3 intersection;
					bool result = RayTriangleIntersection(
						cameraPosition,
						cameraDirection,
						{ A[0], A[1], A[2] },
						{ B[0], B[1], B[2] },
						{ C[0], C[1], C[2] },
						{ A[7], A[8], A[9] },
						intersection
					);
					if (result)
					{
						return intersection;
					}
				}
			}
		}
		return v3(-1.0f);
	}

	v3 RayPlaneIntersection(const v3& X, const v3& v, const v3& C, const v3& n)
	{
		v3 w = C - X;
		f32 k = glm::dot(w, n) / glm::dot(v, n);
		return X + k * v;
	}

	f32 DistanceBetweenLineSLineS(v2 p1, v2 p2, v2 p3, v2 p4)
	{
		v2 u = p1 - p2;
		v2 v = p3 - p4;
		v2 w = p2 - p4;

		double a = glm::dot(u, u);
		double b = glm::dot(u, v);
		double c = glm::dot(v, v);
		double d = glm::dot(u, w);
		double e = glm::dot(v, w);
		double D = a * c - b * b;
		double sD = D;
		double tD = D;
		double sN = 0.0;
		double tN = 0.0;
		double sc = 0.0;
		double tc = 0.0;

		const double SMALL_NUM = 0.00000001f;


		if (D < SMALL_NUM)
		{

			sN = 0.0;
			sD = 1.0;
			tN = e;
			tD = c;
		}
		else
		{
			sN = (b * e - c * d);
			tN = (a * e - b * d);
			if (sN < 0.0)
			{
				sN = 0.0;
				tN = e;
				tD = c;
			}
			else if (sN > sD)
			{
				sN = sD;
				tN = e + b;
				tD = c;
			}
		}

		if (tN < 0.0)
		{

			tN = 0.0;
			if (-d < 0.0)
				sN = 0.0;
			else if (-d > a)
				sN = sD;
			else
			{
				sN = -d;
				sD = a;
			}
		}
		else if (tN > tD)
		{
			tN = tD;
			if ((-d + b) < 0.0)
				sN = 0;
			else if ((-d + b) > a)
				sN = sD;
			else
			{
				sN = (-d + b);
				sD = a;
			}
		}

		if (abs(sN) < SMALL_NUM)
			sc = 0.0;
		else
			sc = sN / sD;

		if (abs(tN) < SMALL_NUM)
			tc = 0.0;
		else
			tc = tN / tD;

		v2 dP = w + (u * (f32)sc) - (v * (f32)tc);
		return glm::length(dP);
	}

	bool RayTriangleIntersection(const v3& camPos, const v3& ray, const v3& A, const v3& B, const v3& C, const v3& normal, v3& intersection)
	{
		v3 u = (C - B) - (glm::dot(C - A, C - B) / glm::dot(C - A, C - A)) * (C - A);
		v3 v = (B - A) - (glm::dot(B - C, B - A) / glm::dot(B - C, B - C)) * (B - C);
		intersection = camPos + ray * (glm::dot(A - camPos, normal) / glm::dot(ray, normal));
		f32 a = 1 - glm::dot(v, intersection - A) / glm::dot(v, B - A);
		f32 b = 1 - glm::dot(u, intersection - B) / glm::dot(u, C - B);

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

	void LevelTheTerrain(const v2& startIndex, const v2& endIndex, const v3& startCoord, const v3& endCoord, Object* terrain, f32 width, bool reset)
	{
		/*v2 AB = {
			endCoord.x - startCoord.x ,
			endCoord.z - startCoord.z
		};
		f32 mAB = glm::length(AB);
		v3 SE = endCoord - startCoord;

		f32* data = terrain->prefab->vertices;

		bool xC = startIndex.x < endIndex.x + 1;
		int xA = xC ? 1 : -1;
		bool yC = startIndex.y < endIndex.y + 1;
		int yA = yC ? 1 : -1;
		u64 yMin = startIndex.y - 2 * yA;
		u64 yMax = endIndex.y + 2 * yA;
		u64 xMin = startIndex.x - 2 * xA;
		u64 xMax = endIndex.x + 2 * xA;
		for (u64 y = yMin; (y < yMax) == yC; y += yA)
		{
			for (u64 x = xMin; (x < xMax) == xC; x += xA)
			{
				if (x < 0 || y < 0 || x >= terrain->w - 1 || y >= terrain->h - 1)
					continue;
				u64 dist1 = (x + (terrain->w - 1) * y) * 60;
				u64 dist2 = (x + (terrain->w - 1) * (y - 1)) * 60;
				v2 AP = {
					data[dist1] - startCoord.x,
					data[dist1 + 2] - startCoord.z
				};
				f32 mAP = glm::length(AP);
				f32 angle = glm::acos(glm::dot(AB, AP) / (mAP * mAB));
				f32 d = mAB * glm::sin(angle);

				if (d < width)
				{
					f32 c = mAB * glm::cos(angle);
					f32 yOffset = SE.y * (c * mAB);
					f32 yCoord = startCoord.y + yOffset;
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
		terrain->VB->ReDo(terrain->Vertices, sizeof(f32) * vertexCount);
		terrain->VB->Unbind();*/
	}

	void UpdateTheTerrain(const RoadSegment& rs, bool reset)
	{
	}

	Prefab* GetPrefabForTerrain(const std::string& texturePath)
	{
#define TEMP_TERRAIN_SHADER "assets/shaders/Cube.glsl"

		std::array<v4, COLOR_COUNT> colors = {
			v4{ 9.0f / 255.0f, 255.0f / 255.0f, 4.0f / 255.0f, 1.0f },
			v4{ 25.0f / 255.0f, 93.0f / 255.0f, 24.0f / 255.0f, 1.0f },
			v4{ 182.0f / 255.0f, 64.0f / 255.0f, 16.0f / 255.0f, 1.0f },
			v4{ 61.0f / 255.0f, 28.0f / 255.0f, 10.0f / 255.0f, 1.0f },
			v4{ 112.0f / 255.0f, 217.0f / 255.0f, 238.0f / 255.0f, 1.0f }
		};

		int width, height, channels;
		unsigned char* data = stbi_load(texturePath.c_str(), &width, &height, &channels, 0);

		u64 w = width - 1;
		u64 h = height - 1;

		u64 indexCount = w * h * 2 * 3;
		u64 vertexCount = indexCount * (3 + 4 + 3);
		f32* vertices = new f32[vertexCount];

		int vertexIndex = 0;
		for (u64 y = 0; y < h; y++)
		{
			for (u64 x = 0; x < w; x++)
			{
				unsigned char* p1 = &data[((x + 0) + (width) * (y + 0)) * channels];
				unsigned char* p2 = &data[((x + 0) + (width) * (y + 1)) * channels];
				unsigned char* p3 = &data[((x + 1) + (width) * (y + 1)) * channels];
				unsigned char* p4 = &data[((x + 1) + (width) * (y + 0)) * channels];

				{
					f32 z = (p1[0] / 256.0f) * COLOR_COUNT;
					u64 heightIndex = (u64)z;
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
					f32 z = (p4[0] / 256.0f) * COLOR_COUNT;
					u64 heightIndex = (u64)z;
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
					f32 z = (p3[0] / 256.0f) * COLOR_COUNT;
					u64 heightIndex = (u64)z;
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
					f32 z = (p1[0] / 256.0f) * COLOR_COUNT;
					u64 heightIndex = (u64)z;
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
					f32 z = (p3[0] / 256.0f) * COLOR_COUNT;
					u64 heightIndex = (u64)z;
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
					f32 z = (p2[0] / 256.0f) * COLOR_COUNT;
					u64 heightIndex = (u64)z;
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
		for (f32 x = 0; x < w; x++)
		{
			for (f32 y = 0; y < h; y++)
			{
				v3 a00(vertices[vertexIndex + 0 + 0], vertices[vertexIndex + 0 + 1], vertices[vertexIndex + 0 + 2]);
				v3 a10(vertices[vertexIndex + 10 + 0], vertices[vertexIndex + 10 + 1], vertices[vertexIndex + 10 + 2]);
				v3 a11(vertices[vertexIndex + 20 + 0], vertices[vertexIndex + 20 + 1], vertices[vertexIndex + 20 + 2]);
				v3 a01(vertices[vertexIndex + 50 + 0], vertices[vertexIndex + 50 + 1], vertices[vertexIndex + 50 + 2]);

				v3 u1 = a11 - a00;
				v3 v1 = a10 - a00;

				v3 u2 = a01 - a00;
				v3 v2 = a11 - a00;

				v3 norm1 = glm::normalize(glm::cross(v1, u1));
				v3 norm2 = glm::normalize(glm::cross(v2, u2));

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

		Prefab* terrainPrefab = new Prefab("", TEMP_TERRAIN_SHADER, "", vertices, indexCount, vertexCount, BufferLayout{ { ShaderDataType::Float3, "a_Position"}, { ShaderDataType::Float4, "a_Color"}, { ShaderDataType::Float3, "a_Normal"} });
		terrainPrefab->boundingBoxL = { 0.0f, 0.0f, -(height / TERRAIN_SCALE_DOWN) };
		terrainPrefab->boundingBoxM = { width / TERRAIN_SCALE_DOWN, 1.0f * COLOR_COUNT, 0.0f };

		return terrainPrefab;
	}

	v2 RotateAPointAroundAPoint(const v2& p1, f32 angleInRadians, const v2& p2)
	{
		return v2{
			glm::cos(angleInRadians) * (p1.x - p2.x) - glm::sin(angleInRadians) * (p1.y - p2.y) + p2.x,
			glm::sin(angleInRadians) * (p1.x - p2.x) + glm::cos(angleInRadians) * (p1.y - p2.y) + p2.y
		};
	}
}