#include "canpch.h"
#include "Helper.h"

#include "Scenes/GameScene.h"
#include "Can/Math.h"
#include "GameApp.h"
#include "Types/RoadNode.h"
#include "Types/Transition.h"
#include "Building.h"

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
			glm::normalize(c1[1] - c1[0]),
				glm::normalize(c1[3] - c1[0]),
				glm::normalize(c2[1] - c2[0]),
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

	void name_me_normals(u64 w, u64 h, u64 min_x, u64 max_x, u64 min_y, u64 max_y, f32* vertices)
	{
		for (u64 x = min_x; x < max_x; x++)
		{
			for (u64 y = min_y; y < max_y; y++)
			{
				u64 vertexIndex = (x + (w - 1) * y) * 60;
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
			}
		}
	}

	void name_me_cutting(u64 w, u64 h, v3 AB, v3 current_point, f32* vertices)
	{

		f32 len = glm::length(AB);
		v3 dir = AB / (len * 3.0f);

		u64 count = (u64)(len * 3.0f);
		for (u64 indexd = 0; indexd < count; indexd++)
		{
			u64 x = (u64)current_point.x;
			u64 y = (u64)current_point.y;
			u64 dist_00 = ((x + 0) + (w - 1) * (y + 0)) * 60;

			vertices[dist_00 + 2U] = 0.0f;
			vertices[dist_00 + 12] = 0.0f;
			vertices[dist_00 + 22] = 0.0f;
			vertices[dist_00 + 32] = 0.0f;
			vertices[dist_00 + 42] = 0.0f;
			vertices[dist_00 + 52] = 0.0f;

			if (x < w - 1)
			{
				u64 dist_x0 = ((x + 1) + (w - 1) * (y + 0)) * 60;
				vertices[dist_x0 + 2U] = 0.0f;
				vertices[dist_x0 + 12] = 0.0f;
				vertices[dist_x0 + 22] = 0.0f;
				vertices[dist_x0 + 32] = 0.0f;
				vertices[dist_x0 + 42] = 0.0f;
				vertices[dist_x0 + 52] = 0.0f;
			}
			if (y < h - 1)
			{
				u64 dist_0y = ((x + 0) + (w - 1) * (y + 1)) * 60;
				vertices[dist_0y + 2U] = 0.0f;
				vertices[dist_0y + 12] = 0.0f;
				vertices[dist_0y + 22] = 0.0f;
				vertices[dist_0y + 32] = 0.0f;
				vertices[dist_0y + 42] = 0.0f;
				vertices[dist_0y + 52] = 0.0f;
			}
			if (x < w - 1 && y < h - 1)
			{
				u64 dist_xy = ((x + 1) + (w - 1) * (y + 1)) * 60;
				vertices[dist_xy + 2U] = 0.0f;
				vertices[dist_xy + 12] = 0.0f;
				vertices[dist_xy + 22] = 0.0f;
				vertices[dist_xy + 32] = 0.0f;
				vertices[dist_xy + 42] = 0.0f;
				vertices[dist_xy + 52] = 0.0f;
			}

			if (x > 0)
			{
				u64 dist_x0 = ((x - 1) + (w - 1) * (y + 0)) * 60;
				vertices[dist_x0 + 2U] = 0.0f;
				vertices[dist_x0 + 12] = 0.0f;
				vertices[dist_x0 + 22] = 0.0f;
				vertices[dist_x0 + 32] = 0.0f;
				vertices[dist_x0 + 42] = 0.0f;
				vertices[dist_x0 + 52] = 0.0f;
			}
			if (y > 0)
			{
				u64 dist_0y = ((x + 0) + (w - 1) * (y - 1)) * 60;
				vertices[dist_0y + 2U] = 0.0f;
				vertices[dist_0y + 12] = 0.0f;
				vertices[dist_0y + 22] = 0.0f;
				vertices[dist_0y + 32] = 0.0f;
				vertices[dist_0y + 42] = 0.0f;
				vertices[dist_0y + 52] = 0.0f;
			}
			if (x > 0 && y > 0)
			{
				u64 dist_xy = ((x - 1) + (w - 1) * (y - 1)) * 60;
				vertices[dist_xy + 2U] = 0.0f;
				vertices[dist_xy + 12] = 0.0f;
				vertices[dist_xy + 22] = 0.0f;
				vertices[dist_xy + 32] = 0.0f;
				vertices[dist_xy + 42] = 0.0f;
				vertices[dist_xy + 52] = 0.0f;
			}
			current_point += dir;
		}
	}

	std::array<u64, 4> name_me_digging(u64 w, u64 h, const std::vector<std::array<v3, 3>>& polygon, f32* vertices, bool reset)
	{
		u64 minX = w - 1;
		u64 maxX = 0;
		u64 minY = h - 1;
		u64 maxY = 0;

		u64 count = polygon.size();
		for (u64 index = 0; index < count; index += 1)
		{
			std::array<v3, 3> tr = polygon[index];
			tr[0].x *= TERRAIN_SCALE_DOWN;
			tr[1].x *= TERRAIN_SCALE_DOWN;
			tr[2].x *= TERRAIN_SCALE_DOWN;
			tr[0].y *= TERRAIN_SCALE_DOWN;
			tr[1].y *= TERRAIN_SCALE_DOWN;
			tr[2].y *= TERRAIN_SCALE_DOWN;

			f32 margin = 0.01f;
			f32 val = reset ? 0.0f : std::min({ tr[0].z, tr[1].z, tr[2].z }) - margin;


			v3* minvx = &tr[0];
			v3* medvx = &tr[1];
			v3* maxvx = &tr[2];
			{
				if (minvx->x > medvx->x)
				{
					v3* temp = medvx;
					medvx = minvx;
					minvx = temp;
				}
				if (medvx->x > maxvx->x)
				{
					v3* temp = maxvx;
					maxvx = medvx;
					medvx = temp;
				}
				if (minvx->x > medvx->x)
				{
					v3* temp = medvx;
					medvx = minvx;
					minvx = temp;
				}
				minvx->x -= 1.0f;
				maxvx->x += 1.5f;
				minX = std::min(minX, (u64)minvx->x);
				maxX = std::max(maxX, (u64)maxvx->x);

				v3* minvy = &tr[0];
				v3* medvy = &tr[1];
				v3* maxvy = &tr[2];
				if (minvy->y > medvy->y)
				{
					v3* temp = medvy;
					medvy = minvy;
					minvy = temp;
				}
				if (medvy->y > maxvy->y)
				{
					v3* temp = maxvy;
					maxvy = medvy;
					medvy = temp;
				}
				if (minvy->y > medvy->y)
				{
					v3* temp = medvy;
					medvy = minvy;
					minvy = temp;
				}
				minvy->y -= 1.0f;
				maxvy->y += 1.5f;
				minY = std::min(minY, (u64)minvy->y);
				maxY = std::max(maxY, (u64)maxvy->y);
			}

			f32 dx1 = std::max(medvx->x - minvx->x, 0.001f);
			f32 dx2 = std::max(maxvx->x - minvx->x, 0.001f);
			f32 dx3 = std::max(maxvx->x - medvx->x, 0.001f);

			f32 dy1 = medvx->y - minvx->y;
			f32 dy2 = maxvx->y - minvx->y;
			f32 dy3 = maxvx->y - medvx->y;

			for (f32 x = minvx->x; x < maxvx->x; x += 0.5f)
			{
				f32 perc1 = (x - minvx->x) / dx1;
				f32 perc2 = (x - minvx->x) / dx2;
				f32 perc3 = (x - medvx->x) / dx3;

				f32 y1 = minvx->y + dy1 * perc1;
				f32 y2 = minvx->y + dy2 * perc2;
				f32 y3 = medvx->y + dy3 * perc3;

				u64 ly = (u64)std::min(x < medvx->x ? y1 : y3, y2);
				u64 my = (u64)std::max(x < medvx->x ? y1 : y3, y2);
				//u64 ly = std::min(y1, y2);
				//u64 my = std::max(y1, y2);

				for (u64 y = ly; y <= my; y++)
				{
					if ((u64)x > w - 1 || y > h - 1)
						continue;

					u64 dist = ((u64)x + (w - 1) * y) * 60;

					vertices[dist + 2U] = val;
					vertices[dist + 32] = val;

					if (x > 0)
						vertices[dist - 60 + 12] = val;
					if (y > 0)
						vertices[dist - 60 * (w - 1) + 52] = val;
					if (x > 0 && y > 0)
					{
						vertices[dist - 60 * w + 22] = val;
						vertices[dist - 60 * w + 42] = val;
					}
				}
			}
		}
		return { minX, maxX, minY, maxY };
	}

	std::string trim_path_and_extension(std::string& path)
	{
		u64 found = path.find_last_of("/\\");
		std::string file = path.substr(found + 1);
		found = file.find_last_of(".");
		return file.substr(0, found);
	}

	u64 find_special(const std::vector<std::pair<u64, std::vector<u64>>>& paths, u64 node) {
		u64 size = paths.size();
		for (u64 i = 0; i < size; i++)
			if (paths[i].second[paths[i].second.size() - 1] == node) return i;
		return (u64)(-1);
	}
	std::vector<Transition*> get_path(Building* start, u8 dist)
	{
		auto& road_segments = GameScene::ActiveGameScene->m_RoadManager.road_segments;
		auto& road_nodes = GameScene::ActiveGameScene->m_RoadManager.road_nodes;

		u64 current_road_segment_index = start->connectedRoadSegment;
		RoadSegment& current_road_segment = road_segments[current_road_segment_index];

		std::vector<Transition*> path{};

		RS_Transition* rs_transition = new RS_Transition();
		path.push_back(rs_transition);
		rs_transition->road_segment_index = current_road_segment_index;
		rs_transition->from_path_array_index = start->snapped_t_index;
		bool go_right_from_house = Utility::Random::Integer(2) == 1;
		u64 next_node = 0;
		rs_transition->from_right = go_right_from_house;
		if (go_right_from_house == start->snapped_to_right)
		{
			next_node = current_road_segment.EndNode;
			rs_transition->to_path_array_index = current_road_segment.curve_samples.size() - 1;
		}
		else
		{
			next_node = current_road_segment.StartNode;
			rs_transition->to_path_array_index = 0;
		}
		rs_transition->distance_from_middle = 0.5f;

		RN_Transition* rn_transition = new RN_Transition();
		path.push_back(rn_transition);
		rn_transition->road_node_index = next_node;

		for (u64 i = 0; i < dist; i++)
		{
			RoadNode& road_node = road_nodes[next_node];
			auto it = std::find(road_node.roadSegments.begin(), road_node.roadSegments.end(), current_road_segment_index);
			if (it == road_node.roadSegments.end()) assert(false);

			u64 index = std::distance(road_node.roadSegments.begin(), it);
			rn_transition->from_road_segments_array_index = index;
			if (rs_transition->from_right)
				rn_transition->sub_index = 1;
			else
				rn_transition->sub_index = 2;

			rs_transition = new RS_Transition();
			path.push_back(rs_transition);

			rs_transition->from_right = true;
			rs_transition->distance_from_middle = 0.5f;
			std::vector<u64>& roads = road_node.roadSegments;
			int size = (int)roads.size();
			int new_road_index = 0;
			if (size > 1)
				while (current_road_segment_index == roads[new_road_index])
					new_road_index = Utility::Random::Integer(size);

			u64 next_road_segment_index = roads[new_road_index];
			RoadSegment& next_road_segment = road_segments[next_road_segment_index];

			rn_transition->to_road_segments_array_index = new_road_index;
			rs_transition->road_segment_index = next_road_segment_index;
			if (new_road_index > index)
				rn_transition->accending = (f32)(new_road_index - index) < (size / 2.0f);
			else if (new_road_index == index)
				rn_transition->accending = rs_transition->from_right;
			else
				rn_transition->accending = (f32)(index - new_road_index) > (size / 2.0f);

			if (next_road_segment.StartNode == next_node)
			{
				rs_transition->from_path_array_index = 0;
				rs_transition->to_path_array_index = next_road_segment.curve_samples.size() - 1;
				next_node = next_road_segment.EndNode;
			}
			else if (next_road_segment.EndNode == next_node)
			{
				rs_transition->from_path_array_index = next_road_segment.curve_samples.size() - 1;
				rs_transition->to_path_array_index = 0;
				next_node = next_road_segment.StartNode;
			}
			else
			{
				assert(false);
			}

			rn_transition = new RN_Transition();
			path.push_back(rn_transition);
			current_road_segment_index = next_road_segment_index;
			rn_transition->road_node_index = next_node;
		}

		auto& node_road_segments = road_nodes[rn_transition->road_node_index].roadSegments;
		auto it = std::find(node_road_segments.begin(), node_road_segments.end(), current_road_segment_index);
		assert(it != node_road_segments.end());
		rn_transition->from_road_segments_array_index = std::distance(node_road_segments.begin(), it);
		rn_transition->to_road_segments_array_index = rn_transition->from_road_segments_array_index;
		if (rs_transition->from_right)
			rn_transition->sub_index = 1;
		else
			rn_transition->sub_index = 2;
		rn_transition->accending = rs_transition->from_right;


		for (u64 i = path.size() - 1; i > 0; i--)
		{
			RS_Transition* mirror_rs_transition = (RS_Transition*)path[i - 1];
			RS_Transition* rs_transition = new RS_Transition();
			path.push_back(rs_transition);
			rs_transition->from_path_array_index = mirror_rs_transition->to_path_array_index;
			rs_transition->to_path_array_index = mirror_rs_transition->from_path_array_index;
			rs_transition->distance_from_middle = mirror_rs_transition->distance_from_middle;
			rs_transition->from_right = mirror_rs_transition->from_right;
			rs_transition->road_segment_index = mirror_rs_transition->road_segment_index;
			i--;
			if (i == 0)
			{
				rs_transition->from_right = !rs_transition->from_right;
				break;
			}
			RN_Transition* mirror_rn_transition = (RN_Transition*)path[i - 1];
			rn_transition = new RN_Transition();
			rn_transition->from_road_segments_array_index = mirror_rn_transition->to_road_segments_array_index;
			rn_transition->to_road_segments_array_index = mirror_rn_transition->from_road_segments_array_index;

			RoadNode& road_node = road_nodes[rn_transition->road_node_index];
			std::vector<u64>& roads = road_node.roadSegments;
			int size = (int)roads.size();
			f32 diff = rn_transition->to_road_segments_array_index - rn_transition->from_road_segments_array_index;
			if (diff > 0.0f)
				rn_transition->accending = diff < (size / 2.0f);
			else if (diff == 0.0f)
				rn_transition->accending = rs_transition->from_right;
			else
				rn_transition->accending = diff * -1.0f > (size / 2.0f);

			rn_transition->road_node_index = mirror_rn_transition->road_node_index;
			if (rs_transition->from_right)
				rn_transition->sub_index = 1;
			else
				rn_transition->sub_index = 2;

			path.push_back(rn_transition);
		}
		return path;
	}
	
	//TODO memory leak possible
	std::vector<Transition*> get_path(Building* start, Building* end)
	{
		auto& road_segments = GameScene::ActiveGameScene->m_RoadManager.road_segments;
		auto& road_nodes = GameScene::ActiveGameScene->m_RoadManager.road_nodes;
		auto& types = GameScene::ActiveGameScene->MainApplication->road_types;
		if (start->connectedRoadSegment == end->connectedRoadSegment)
		{
			if (start->snapped_to_right == end->snapped_to_right)
			{
				RS_Transition* rs_transition = new RS_Transition();
				rs_transition->from_path_array_index = start->snapped_t_index;
				rs_transition->to_path_array_index = end->snapped_t_index;
				rs_transition->distance_from_middle = 0.5f;
				u64 diff = end->snapped_t_index - start->snapped_t_index;
				if (diff > 0)
					rs_transition->from_right = start->snapped_to_right;
				else if (diff == 0)
					rs_transition->from_right = start->snapped_to_right == (end->snapped_t > start->snapped_t);
				else
					rs_transition->from_right = !start->snapped_to_right;
				rs_transition->road_segment_index = start->connectedRoadSegment;
				return { rs_transition };
			}
			else
			{
				RoadSegment& road_segment = road_segments[start->connectedRoadSegment];
				bool from_start = road_segment.curve_samples.size() > start->snapped_t_index + end->snapped_t_index;

				RS_Transition* rs_transition_s = new RS_Transition();
				rs_transition_s->from_path_array_index = start->snapped_t_index;
				rs_transition_s->to_path_array_index = from_start ? 0 : road_segment.curve_samples.size() - 1;
				rs_transition_s->distance_from_middle = 0.5f;
				rs_transition_s->from_right = !start->snapped_to_right == from_start;
				rs_transition_s->road_segment_index = start->connectedRoadSegment;

				u64 road_node_index = from_start ? road_segment.StartNode : road_segment.EndNode;
				RoadNode& road_node = road_nodes[road_node_index];
				auto it = std::find(road_node.roadSegments.begin(), road_node.roadSegments.end(), start->connectedRoadSegment);
				assert(it != road_node.roadSegments.end());
				u64 index = std::distance(road_node.roadSegments.begin(), it);
				RN_Transition* rn_transition = new RN_Transition();
				rn_transition->from_road_segments_array_index = index;
				rn_transition->to_road_segments_array_index = index;
				rn_transition->road_node_index = road_node_index;
				if (rs_transition_s->from_right)
				{
					rn_transition->accending = true;
					rn_transition->sub_index = 1;
				}
				else
				{
					rn_transition->accending = false;
					rn_transition->sub_index = 2;
				}

				RS_Transition* rs_transition_e = new RS_Transition();
				rs_transition_e->from_path_array_index = rs_transition_e->to_path_array_index;
				rs_transition_e->to_path_array_index = end->snapped_t_index;
				rs_transition_e->distance_from_middle = 0.5f;
				rs_transition_s->from_right = rs_transition_s->from_right == from_start;
				rs_transition_e->road_segment_index = end->connectedRoadSegment;
				return { rs_transition_s, rn_transition, rs_transition_e };
			}
		}

		RoadSegment& start_road_segment = road_segments[start->connectedRoadSegment];
		RoadSegment& end_road_segment = road_segments[end->connectedRoadSegment];

		std::vector<std::tuple<s64, s64, s64, s64>> linqs{};
		std::vector<std::tuple<s64, s64, s64, s64>> visited_nodes{};
		linqs.push_back(std::tuple<s64, s64, s64, s64>{1, start->connectedRoadSegment, -1, start_road_segment.StartNode});
		linqs.push_back(std::tuple<s64, s64, s64, s64>{1, start->connectedRoadSegment, -1, start_road_segment.EndNode});

		std::vector<u64> visited_junctions{};
		std::vector<std::pair<u64, std::vector<u64>>> paths{}; // instead of this use links(watch the video again) 
		std::vector<std::pair<u64, std::vector<u64>>> no_exit_paths{}; // delete me
		paths.push_back(std::pair<u64, std::vector<u64>>{ 1, std::vector<u64>{ (u64)start->connectedRoadSegment, start_road_segment.StartNode } });
		paths.push_back(std::pair<u64, std::vector<u64>>{ 1, std::vector<u64>{ (u64)start->connectedRoadSegment, start_road_segment.EndNode } });


		while (paths.empty() == false)
		{
			std::sort(linqs.begin(), linqs.end(), Helper::sort_by_distance());
			auto closest = linqs[linqs.size() - 1];
			linqs.pop_back();
			visited_nodes.push_back(closest);

			s64 road_node_index = std::get<3>(closest);
			RoadNode& road_node = road_nodes[road_node_index];
			auto& connected_road_segments = road_node.roadSegments;
			for (u64 i = 0; i < connected_road_segments.size(); i++)
			{
				u64 road_segment_index = connected_road_segments[i];
				if (road_segment_index == end->connectedRoadSegment)
				{
					std::vector<Transition*> the_path{};
					assert(false);
					return the_path;
				}
				RoadSegment& road_segment = road_segments[road_segment_index];
				s64 new_distance = std::get<0>(closest) + 1;
				if (types[road_segment.type].zoneable == false) continue;

				s64 next_road_node_index = road_segment.StartNode == road_node_index ? road_segment.EndNode : road_segment.StartNode;

				auto it = std::find_if(visited_nodes.begin(), visited_nodes.end(), [next_road_node_index](const std::tuple<s64, s64, s64, s64>& el) {
					return std::get<2>(el) == next_road_node_index;
					});
				if (it != visited_nodes.end())
				{
					it = std::find_if(linqs.begin(), linqs.end(), [next_road_node_index](const std::tuple<s64, s64, s64, s64>& el) {
						return std::get<2>(el) == next_road_node_index;
						});
					if (it == linqs.end())
						if (std::get<0>(*it) <= new_distance)
							continue;
					linqs.push_back(std::tuple<s64, s64, s64, s64>{new_distance, road_segment_index, road_node_index, next_road_node_index});
					assert(false); // check this
				}

			}
		}
		return get_path(start, 4);
	}


	void UpdateTheTerrain(const std::vector<std::array<v3, 3>>& polygon, bool reset)
	{
		GameApp* app = GameScene::ActiveGameScene->MainApplication;
		float* vertices = app->terrainPrefab->vertices;
		u64 w = app->terrainTexture->GetWidth();
		u64 h = app->terrainTexture->GetHeight();

		auto [minX, maxX, minY, maxY] = name_me_digging(w, h, polygon, vertices, reset);

		minX -= 1;
		maxX += 1;
		minY -= 1;
		maxY += 1;

		minX = std::max((u64)0, minX);
		maxX = std::min(w - 1, maxX);
		minY = std::max((u64)0, minY);
		maxY = std::min(h - 1, maxY);

		name_me_normals(w, h, minX, maxX, minY, maxY, vertices);

		u64 vertexCount = app->terrainPrefab->indexCount * (3 + 4 + 3);
		app->terrainPrefab->vertexBuffer->Bind();
		app->terrainPrefab->vertexBuffer->ReDo(app->terrainPrefab->vertices, (u32)(sizeof(f32) * vertexCount));
		app->terrainPrefab->vertexBuffer->Unbind();
	}

	void UpdateTheTerrain(RoadSegment* rs, bool reset)
	{
		GameApp* app = GameScene::ActiveGameScene->MainApplication;
		auto& road_nodes = app->gameScene->m_RoadManager.road_nodes;
		if (rs->StartNode >= road_nodes.capacity)
			return;
		if (rs->EndNode >= road_nodes.capacity)
			return;
		if (road_nodes.values[rs->StartNode].valid == false)
			return;
		if (road_nodes.values[rs->EndNode].valid == false)
			return;

		RoadNode& startNode = road_nodes[rs->StartNode];
		RoadNode& endNode = road_nodes[rs->EndNode];
		bool start_node_is_tunnel = startNode.elevation_type == -1;
		bool end_node_is_tunnel = endNode.elevation_type == -1;

		float* vertices = app->terrainPrefab->vertices;
		u64 w = app->terrainTexture->GetWidth();
		u64 h = app->terrainTexture->GetHeight();

		auto [minX, maxX, minY, maxY] = name_me_digging(w, h, rs->bounding_polygon, vertices, reset);

		if (start_node_is_tunnel)
		{
			auto tr = rs->bounding_polygon[0];
			tr[0].x *= TERRAIN_SCALE_DOWN;
			tr[1].x *= TERRAIN_SCALE_DOWN;
			tr[2].x *= TERRAIN_SCALE_DOWN;
			tr[0].y *= TERRAIN_SCALE_DOWN;
			tr[1].y *= TERRAIN_SCALE_DOWN;
			tr[2].y *= TERRAIN_SCALE_DOWN;
			v3 AB = tr[1] - tr[0];
			v3 current_point = tr[0];
			name_me_cutting(w, h, AB, current_point, vertices);
		}
		else if (end_node_is_tunnel)
		{
			auto tr = rs->bounding_polygon[rs->bounding_polygon.size() - 1];
			tr[0].x *= TERRAIN_SCALE_DOWN;
			tr[1].x *= TERRAIN_SCALE_DOWN;
			tr[2].x *= TERRAIN_SCALE_DOWN;
			tr[0].y *= TERRAIN_SCALE_DOWN;
			tr[1].y *= TERRAIN_SCALE_DOWN;
			tr[2].y *= TERRAIN_SCALE_DOWN;
			v3 AB = tr[1] - tr[2];
			v3 current_point = tr[2];
			name_me_cutting(w, h, AB, current_point, vertices);
		}

		minX -= 1;
		maxX += 1;
		minY -= 1;
		maxY += 1;

		minX = std::max((u64)0, minX);
		maxX = std::min(w - 1, maxX);
		minY = std::max((u64)0, minY);
		maxY = std::min(h - 1, maxY);

		name_me_normals(w, h, minX, maxX, minY, maxY, vertices);

		u64 vertexCount = app->terrainPrefab->indexCount * (3 + 4 + 3);
		app->terrainPrefab->vertexBuffer->Bind();
		app->terrainPrefab->vertexBuffer->ReDo(app->terrainPrefab->vertices, sizeof(f32) * vertexCount);
		app->terrainPrefab->vertexBuffer->Unbind();
	}

	struct TerrainVertices
	{
		v3 position;
		v4 color;
		v3 normal;
	};
	Prefab* GetPrefabForTerrain(const std::string& texturePath)
	{
		Prefab* terrain = nullptr;
		const v4 COLOR{ 9.0f / 255.0f, 255.0f / 255.0f, 4.0f / 255.0f, 1.0f };

		int texture_width, texture_height, texture_channels;
		u8* data = (u8*)stbi_load(texturePath.c_str(), &texture_width, &texture_height, &texture_channels, 0);

		u64 terrain_width = texture_width - (u64)1;
		u64 terrain_depth = texture_height - (u64)1;
		u64 terrain_vertex_count = terrain_width * terrain_depth * 2 * 3;

		TerrainVertices* terrain_vertices = new TerrainVertices[terrain_vertex_count];
		u64 index = 0;
#define TEMP 5
		for (u64 y = 0; y < terrain_depth; y++)
		{
			u8 p_00 = data[(texture_width * (y + 0)) * texture_channels];
			u8 p_0y = data[(texture_width * (y + 1)) * texture_channels];
			for (u64 x = 0; x < terrain_width; x++)
			{
				u8 p_x0 = data[((x + 1) + texture_width * (y + 0)) * texture_channels];
				u8 p_xy = data[((x + 1) + texture_width * (y + 1)) * texture_channels];

				f32 ratio = Math::lerp((x * 1.0f) / terrain_width, 1.0f, 0.5);
				{
					terrain_vertices[index].position.x = ((x + 0) / TERRAIN_SCALE_DOWN);
					terrain_vertices[index].position.y = ((y + 0) / TERRAIN_SCALE_DOWN);
					terrain_vertices[index].position.z = (p_00 / (255.0f / TEMP));
					terrain_vertices[index].color = COLOR * ratio;
					index++;
				}
				{
					terrain_vertices[index].position.x = ((x + 1) / TERRAIN_SCALE_DOWN);
					terrain_vertices[index].position.y = ((y + 0) / TERRAIN_SCALE_DOWN);
					terrain_vertices[index].position.z = (p_x0 / (255.0f / TEMP));
					terrain_vertices[index].color = COLOR * ratio;
					index++;
				}
				{
					terrain_vertices[index].position.x = ((x + 1) / TERRAIN_SCALE_DOWN);
					terrain_vertices[index].position.y = ((y + 1) / TERRAIN_SCALE_DOWN);
					terrain_vertices[index].position.z = (p_xy / (255.0f / TEMP));
					terrain_vertices[index].color = COLOR * ratio;
					index++;
				}
				{
					terrain_vertices[index].position.x = ((x + 0) / TERRAIN_SCALE_DOWN);
					terrain_vertices[index].position.y = ((y + 0) / TERRAIN_SCALE_DOWN);
					terrain_vertices[index].position.z = (p_00 / (255.0f / TEMP));
					terrain_vertices[index].color = COLOR * ratio;
					index++;
				}
				{
					terrain_vertices[index].position.x = ((x + 1) / TERRAIN_SCALE_DOWN);
					terrain_vertices[index].position.y = ((y + 1) / TERRAIN_SCALE_DOWN);
					terrain_vertices[index].position.z = (p_xy / (255.0f / TEMP));
					terrain_vertices[index].color = COLOR * ratio;
					index++;
				}
				{
					terrain_vertices[index].position.x = ((x + 0) / TERRAIN_SCALE_DOWN);
					terrain_vertices[index].position.y = ((y + 1) / TERRAIN_SCALE_DOWN);
					terrain_vertices[index].position.z = (p_0y / (255.0f / TEMP));
					terrain_vertices[index].color = COLOR * ratio;
					index++;
				}
				p_00 = p_x0;
				p_0y = p_xy;
			}
		}
		index = 0;
		for (u64 y = 0; y < terrain_depth; y++)
		{
			v3 v_00 = terrain_vertices[index + 0].position;
			v3 v_0y = terrain_vertices[index + 5].position;
			for (u64 x = 0; x < terrain_width; x++)
			{
				v3 v_x0 = terrain_vertices[index + 1].position;
				v3 v_xy = terrain_vertices[index + 2].position;

				v3 u_1 = v_xy - v_00;
				v3 v_1 = v_x0 - v_00;
				v3 normal_vector_1 = glm::normalize(glm::cross(v_1, u_1));

				v3 u_2 = v_0y - v_00;
				v3 v_2 = v_xy - v_00;
				v3 normal_vector_2 = glm::normalize(glm::cross(v_2, u_2));

				terrain_vertices[index + 0].normal = normal_vector_1;
				terrain_vertices[index + 1].normal = normal_vector_1;
				terrain_vertices[index + 2].normal = normal_vector_1;
				terrain_vertices[index + 3].normal = normal_vector_2;
				terrain_vertices[index + 4].normal = normal_vector_2;
				terrain_vertices[index + 5].normal = normal_vector_2;
				index += 6;

				v_00 = v_x0;
				v_0y = v_xy;
			}
		}

		terrain = new Prefab(
			"",
			"assets/shaders/Cube.glsl",
			"",
			(float*)terrain_vertices,
			terrain_vertex_count,
			terrain_vertex_count * (sizeof(TerrainVertices) / sizeof(float)),
			BufferLayout{
				{ShaderDataType::Float3, "a_Position"},
				{ShaderDataType::Float4, "a_Color"},
				{ShaderDataType::Float3, "a_Normal"}
			});
		terrain->boundingBoxL = { 0.0f, 0.0f,0.0f };
		terrain->boundingBoxM = {
			texture_width / TERRAIN_SCALE_DOWN,
			texture_height / TERRAIN_SCALE_DOWN,
			1.0f * TEMP };

		return terrain;
	}

	v2 RotateAPointAroundAPoint(const v2& p1, f32 angleInRadians, const v2& p2)
	{
		return v2{
			glm::cos(angleInRadians) * (p1.x - p2.x) - glm::sin(angleInRadians) * (p1.y - p2.y) + p2.x,
			glm::sin(angleInRadians) * (p1.x - p2.x) + glm::cos(angleInRadians) * (p1.y - p2.y) + p2.y
		};
	}
}