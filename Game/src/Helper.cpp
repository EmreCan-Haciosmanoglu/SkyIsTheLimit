#include "canpch.h"
#include "Helper.h"

#include "Scenes/GameScene.h"
#include "Can/Math.h"
#include "GameApp.h"
#include "Types/RoadNode.h"
#include "Building.h"

namespace  Can::Helper
{
	bool CheckBoundingBoxHit(const v3& rayStartPoint, const v3& ray, const v3& least, const v3& most)
	{
		v3 leftPlaneCP = Math::ray_plane_intersection(rayStartPoint, ray, { least.x, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f });
		v3 rigthPlaneCP = Math::ray_plane_intersection(rayStartPoint, ray, { most.x, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f });

		v3 bottomPlaneCP = Math::ray_plane_intersection(rayStartPoint, ray, { 0.0f, least.y, 0.0f }, { 0.0f, 1.0f, 0.0f });
		v3 topPlaneCP = Math::ray_plane_intersection(rayStartPoint, ray, { 0.0f, most.y, 0.0f }, { 0.0f, 1.0f, 0.0f });

		v3 nearPlaneCP = Math::ray_plane_intersection(rayStartPoint, ray, { 0.0f, 0.0f, least.z }, { 0.0f, 0.0f, 1.0f });
		v3 farPlaneCP = Math::ray_plane_intersection(rayStartPoint, ray, { 0.0f, 0.0f, most.z }, { 0.0f, 0.0f, 1.0f });


		return
			(bottomPlaneCP.x >= least.x && bottomPlaneCP.x <= most.x && bottomPlaneCP.z >= least.z && bottomPlaneCP.z <= most.z) ||
			(topPlaneCP.x >= least.x && topPlaneCP.x <= most.x && topPlaneCP.z >= least.z && topPlaneCP.z <= most.z) ||
			(leftPlaneCP.y >= least.y && leftPlaneCP.y <= most.y && leftPlaneCP.z >= least.z && leftPlaneCP.z <= most.z) ||
			(rigthPlaneCP.y >= least.y && rigthPlaneCP.y <= most.y && rigthPlaneCP.z >= least.z && rigthPlaneCP.z <= most.z) ||
			(nearPlaneCP.x >= least.x && nearPlaneCP.x <= most.x && nearPlaneCP.y >= least.y && nearPlaneCP.y <= most.y) ||
			(farPlaneCP.x >= least.x && farPlaneCP.x <= most.x && farPlaneCP.y >= least.y && farPlaneCP.y <= most.y);
	}

	//delete later
	static std::array<v2, 4> getAxis(const std::array<v2, 4>& c1, const std::array<v2, 4>& c2)
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
		v3 bottomPlaneCollisionPoint = Math::ray_plane_intersection(cameraPosition, cameraDirection, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });
		v3 topPlaneCollisionPoint = Math::ray_plane_intersection(cameraPosition, cameraDirection, { 0.0f, 1.0f * COLOR_COUNT, 0.0f }, { 0.0f, 1.0f, 0.0f });

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

	static void fill_points_stack(std::vector<RS_Transition_For_Vehicle*>& path, Building* start, Building* end)
	{
		auto& road_segments{ GameScene::ActiveGameScene->m_RoadManager.road_segments };
		auto& road_nodes{ GameScene::ActiveGameScene->m_RoadManager.road_nodes };
		auto& road_types{ GameScene::ActiveGameScene->MainApplication->road_types };
		u64 transition_count{ path.size() };

		RS_Transition_For_Vehicle* current_transition{ path[0] };
		if (transition_count == 1)
		{
			RoadSegment& current_road_segment = road_segments[current_transition->road_segment_index];
			RoadType& current_road_type = road_types[current_road_segment.type];
			auto& current_road_segment_curve_samples = current_road_segment.curve_samples;
			u64 curve_sample_count = current_road_segment_curve_samples.size();
			f32 dist_from_center = 0.0f;
			if (current_transition->lane_index < current_road_type.lanes_backward.size())
				dist_from_center = current_road_type.lanes_backward[current_transition->lane_index].distance_from_center;
			else
				dist_from_center = current_road_type.lanes_forward[current_transition->lane_index - current_road_type.lanes_backward.size()].distance_from_center;

			u64 curve_sample_index_start = std::min(start->snapped_t_index, end->snapped_t_index);
			u64 curve_sample_index_end = std::max(start->snapped_t_index, end->snapped_t_index);

			v3 p0 = current_road_segment_curve_samples[curve_sample_index_start];
			// points on the roads to travel
			for (u64 curve_sample_index = curve_sample_index_start + 1; curve_sample_index < curve_sample_index_end; curve_sample_index++)
			{
				v3 p1 = current_road_segment_curve_samples[curve_sample_index];
				v3 dir_to_p1 = p1 - p0;
				v3 cw_rotated_dir = glm::normalize(v3{ dir_to_p1.y, -dir_to_p1.x, 0.0f });
				v3 path_point = p0 + cw_rotated_dir * dist_from_center;
				current_transition->points_stack.push_back(path_point);
				p0 = p1;
			}
			v3 dir_to_p1 = current_road_segment.GetEndDirection() * -1.0f;
			v3 cw_rotated_dir = glm::normalize(v3{ dir_to_p1.y, -dir_to_p1.x, 0.0f });
			v3 path_point = p0 + cw_rotated_dir * dist_from_center;
			current_transition->points_stack.push_back(path_point);
			if (start->snapped_t_index < end->snapped_t_index)
				std::reverse(current_transition->points_stack.begin(), current_transition->points_stack.end());
			return;
		}
		s64 prev_road_node_index = -1;
		for (u64 i = 1; i < transition_count; i++)
		{
			RS_Transition_For_Vehicle* next_transition{ path[i] };

			RoadSegment& current_road_segment = road_segments[current_transition->road_segment_index];
			RoadType& current_road_type = road_types[current_road_segment.type];

			auto& next_road_node_connected_road_segments = road_nodes[current_transition->next_road_node_index].roadSegments;

			auto curr_it = std::find(
				next_road_node_connected_road_segments.begin(),
				next_road_node_connected_road_segments.end(),
				current_transition->road_segment_index
			);
			assert(curr_it != next_road_node_connected_road_segments.end());
			s64 start_index = std::distance(
				next_road_node_connected_road_segments.begin(),
				curr_it
			);

			auto next_it = std::find(
				next_road_node_connected_road_segments.begin(),
				next_road_node_connected_road_segments.end(),
				next_transition->road_segment_index
			);
			assert(next_it != next_road_node_connected_road_segments.end());
			s64 end_index = std::distance(
				next_road_node_connected_road_segments.begin(),
				next_it
			);
			if (start_index == end_index) {
				if (current_transition->next_road_node_index == current_road_segment.EndNode)
				{
					current_transition->lane_index = 0;
					current_transition->lane_index += (u32)current_road_type.lanes_backward.size();
				}
				else
				{
					current_transition->lane_index += (u32)(current_road_type.lanes_backward.size() - 1);
				}
			}
			else
			{
				s64 road_end_counts = next_road_node_connected_road_segments.size();
				end_index = (end_index + road_end_counts - start_index) % road_end_counts;
				road_end_counts -= 2;
				start_index = 0;
				end_index--;
				if (end_index < road_end_counts * 0.3f)
				{
					if (current_transition->next_road_node_index == current_road_segment.EndNode)
					{
						current_transition->lane_index = (u32)(current_road_type.lanes_forward.size() - 1);
						if (current_road_type.zoneable) current_transition->lane_index -= 1;
						current_transition->lane_index += (u32)(current_road_type.lanes_backward.size());
					}
					else
					{
						current_transition->lane_index = 0;
						if (current_road_type.zoneable) current_transition->lane_index += 1;
					}
				}
				else if (end_index > road_end_counts * 0.7f)
				{
					if (current_transition->next_road_node_index == current_road_segment.EndNode)
					{
						current_transition->lane_index = 0;
						current_transition->lane_index += (u32)(current_road_type.lanes_backward.size());
					}
					else
					{
						current_transition->lane_index = (u32)(current_road_type.lanes_backward.size() - 1);
					}
				}
				else
				{
					if (current_transition->next_road_node_index == current_road_segment.EndNode)
					{
						s64 lane_count = current_road_type.lanes_forward.size();
						if (current_road_type.zoneable) lane_count -= 1;
						current_transition->lane_index = (u32)((f32)lane_count * 0.5f);
						current_transition->lane_index += (u32)current_road_type.lanes_backward.size();
					}
					else
					{
						s64 lane_count = current_road_type.lanes_backward.size();
						if (current_road_type.zoneable)
						{
							lane_count -= 1;
							current_transition->lane_index = 1;
						}
						current_transition->lane_index += (u32)((f32)lane_count * 0.5f);
					}
				}
			}

			auto& current_road_segment_curve_samples = current_road_segment.curve_samples;
			u64 curve_sample_count = current_road_segment_curve_samples.size();
			f32 dist_from_center = 0.0f;
			if (current_transition->lane_index < current_road_type.lanes_backward.size())
				dist_from_center = current_road_type.lanes_backward[current_transition->lane_index].distance_from_center;
			else
				dist_from_center = current_road_type.lanes_forward[current_transition->lane_index - current_road_type.lanes_backward.size()].distance_from_center;
			v3 p0 = current_road_segment_curve_samples[0];

			// points on the roads to travel
			for (u64 curve_sample_index = 1; curve_sample_index < curve_sample_count; curve_sample_index++)
			{
				v3 p1 = current_road_segment_curve_samples[curve_sample_index];
				v3 dir_to_p1 = p1 - p0;
				v3 cw_rotated_dir = glm::normalize(v3{ dir_to_p1.y, -dir_to_p1.x, 0.0f });
				v3 path_point = p0 + cw_rotated_dir * dist_from_center;
				current_transition->points_stack.push_back(path_point);
				p0 = p1;
			}
			v3 dir_to_p1 = current_road_segment.GetEndDirection() * -1.0f;
			v3 cw_rotated_dir = glm::normalize(v3{ dir_to_p1.y, -dir_to_p1.x, 0.0f });
			v3 path_point = p0 + cw_rotated_dir * dist_from_center;
			current_transition->points_stack.push_back(path_point);
			if (current_transition->next_road_node_index == current_road_segment.EndNode)
				std::reverse(current_transition->points_stack.begin(), current_transition->points_stack.end());

			prev_road_node_index = current_transition->next_road_node_index;
			current_transition = next_transition;
		}

		current_transition = path[transition_count - 1];
		RoadSegment& current_road_segment = road_segments[current_transition->road_segment_index];
		RoadType& current_road_type = road_types[current_road_segment.type];
		if (transition_count > 2)
		{
			RS_Transition_For_Vehicle* t{ path[transition_count - 2] };
			u64 n_index{ (u64)t->next_road_node_index };
			if (end->snapped_to_right)
			{
				if (n_index == current_road_segment.StartNode)
				{
					current_transition->lane_index = (u32)(current_road_type.lanes_forward.size() - 2);
					current_transition->lane_index += (u32)current_road_type.lanes_backward.size();
				}
				else
				{
					current_transition->lane_index = (u32)(current_road_type.lanes_backward.size() - 1);
				}
			}
			else
			{
				if (n_index == current_road_segment.StartNode)
				{
					current_transition->lane_index = 0;
					current_transition->lane_index += (u32)current_road_type.lanes_backward.size();
				}
				else
				{
					current_transition->lane_index = 1;
				}
			}
		}
		else
		{
			//???
		}

		auto& current_road_segment_curve_samples{ current_road_segment.curve_samples };
		f32 dist_from_center{ 0.0f };
		if (current_transition->lane_index < current_road_type.lanes_backward.size())
			dist_from_center = current_road_type.lanes_backward[current_transition->lane_index].distance_from_center;
		else
			dist_from_center = current_road_type.lanes_forward[current_transition->lane_index - current_road_type.lanes_backward.size()].distance_from_center;

		if (prev_road_node_index == current_road_segment.StartNode)
		{
			v3 p0{ current_road_segment_curve_samples[0] };
			for (u64 curve_sample_index = 1; curve_sample_index <= (u64)end->snapped_t_index; curve_sample_index++)
			{
				v3 p1 = current_road_segment_curve_samples[curve_sample_index];
				v3 dir_to_p1 = p1 - p0;
				v3 cw_rotated_dir = glm::normalize(v3{ dir_to_p1.y, -dir_to_p1.x, 0.0f });
				v3 path_point = p0 + cw_rotated_dir * dist_from_center;
				current_transition->points_stack.push_back(path_point);
				p0 = p1;
			}
			std::reverse(current_transition->points_stack.begin(), current_transition->points_stack.end());
		}
		else
		{
			v3 p0 = current_road_segment_curve_samples[end->snapped_t_index];
			for (u64 curve_sample_index = end->snapped_t_index + 1; curve_sample_index < current_road_segment_curve_samples.size(); curve_sample_index++)
			{
				v3 p1 = current_road_segment_curve_samples[curve_sample_index];
				v3 dir_to_p1 = p1 - p0;
				v3 cw_rotated_dir = glm::normalize(v3{ dir_to_p1.y, -dir_to_p1.x, 0.0f });
				v3 path_point = p0 + cw_rotated_dir * dist_from_center;
				current_transition->points_stack.push_back(path_point);
				p0 = p1;
			}
			v3 dir_to_p1 = current_road_segment.GetEndDirection() * -1.0f;
			v3 cw_rotated_dir = glm::normalize(v3{ dir_to_p1.y, -dir_to_p1.x, 0.0f });
			v3 path_point = p0 + cw_rotated_dir * dist_from_center;
			current_transition->points_stack.push_back(path_point);
		}

		RS_Transition_For_Vehicle* first_path{ path[0] };
		RoadSegment& first_road_segment{ road_segments[first_path->road_segment_index] };
		if (first_path->next_road_node_index == first_road_segment.EndNode)
		{
			for (u64 k = 0; k < (u64)start->snapped_t_index; k++)
				first_path->points_stack.pop_back();
		}
		else
		{
			for (u64 k = 0; k < first_road_segment.curve_samples.size() - start->snapped_t_index - 1; k++)
				first_path->points_stack.pop_back();
		}
	}
	static u64 find_special(const std::vector<std::pair<u64, std::vector<u64>>>& paths, u64 node) {
		u64 size = paths.size();
		for (u64 i = 0; i < size; i++)
			if (paths[i].second[paths[i].second.size() - 1] == node) return i;
		return (u64)(-1);
	}
	std::vector<Transition*> get_path(Building* start, u8 dist)
	{
		auto& road_segments = GameScene::ActiveGameScene->m_RoadManager.road_segments;
		auto& road_types = GameScene::ActiveGameScene->MainApplication->road_types;
		auto& road_nodes = GameScene::ActiveGameScene->m_RoadManager.road_nodes;

		u64 current_road_segment_index = start->connectedRoadSegment;
		RoadSegment& current_road_segment = road_segments[current_road_segment_index];

		std::vector<Transition*> path{};

		RS_Transition_For_Walking* rs_transition = new RS_Transition_For_Walking();
		path.push_back(rs_transition);
		rs_transition->road_segment_index = current_road_segment_index;
		bool go_right_from_house = Utility::Random::signed_32(2) == 1;
		u64 next_node = 0;
		rs_transition->from_right = go_right_from_house;
		if (go_right_from_house == start->snapped_to_right)
		{
			next_node = current_road_segment.EndNode;
			rs_transition->from_start = true;
		}
		else
		{
			next_node = current_road_segment.StartNode;
			rs_transition->from_start = false;
		}

		RN_Transition_For_Walking* rn_transition = new RN_Transition_For_Walking();
		path.push_back(rn_transition);
		rn_transition->road_node_index = next_node;

		for (u64 i = 0; i < dist; i++)
		{
			RoadNode& road_node = road_nodes[next_node];
			auto it = std::find(road_node.roadSegments.begin(), road_node.roadSegments.end(), current_road_segment_index);
			assert(it != road_node.roadSegments.end());
			u64 index = std::distance(road_node.roadSegments.begin(), it);

			rn_transition->from_road_segments_array_index = index;
			if (rs_transition->from_right)
				rn_transition->sub_index = 2;
			else
				rn_transition->sub_index = 1;

			assert(rs_transition->at_path_array_index < 0xFFFF);
			rs_transition = new RS_Transition_For_Walking();
			path.push_back(rs_transition);

			rs_transition->from_right = true;
			std::vector<u64>& roads = road_node.roadSegments;
			std::vector<u64> available_road_segment_indexes{};
			for (u64 road_segment_index : roads)
				if (
					(road_segment_index != current_road_segment_index) &&
					(road_types[road_segments[road_segment_index].type].zoneable)
					)
					available_road_segment_indexes.push_back(road_segment_index);

			int size = (int)available_road_segment_indexes.size();
			u64 road_segment_i = (size == 0) ? current_road_segment_index : available_road_segment_indexes[Utility::Random::signed_32(size)];
			it = std::find(road_node.roadSegments.begin(), road_node.roadSegments.end(), road_segment_i);
			assert(it != road_node.roadSegments.end());
			int new_road_index = std::distance(road_node.roadSegments.begin(), it);

			u64 next_road_segment_index = roads[new_road_index];
			RoadSegment& next_road_segment = road_segments[next_road_segment_index];

			rn_transition->to_road_segments_array_index = new_road_index;
			rs_transition->road_segment_index = next_road_segment_index;
			if (new_road_index > index)
				rn_transition->accending = (f32)(new_road_index - index) < (size / 2.0f);
			else if (new_road_index == index)
				rn_transition->accending = !rs_transition->from_right;
			else
				rn_transition->accending = (f32)(index - new_road_index) > (size / 2.0f);

			if (next_road_segment.StartNode == next_node)
			{
				rs_transition->from_start = true;
				next_node = next_road_segment.EndNode;
			}
			else if (next_road_segment.EndNode == next_node)
			{
				rs_transition->from_start = false;
				next_node = next_road_segment.StartNode;
			}
			else
			{
				assert(false);
			}

			rn_transition = new RN_Transition_For_Walking();
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
			rn_transition->sub_index = 2;
		else
			rn_transition->sub_index = 1;
		rn_transition->accending = !rs_transition->from_right;


		for (u64 i = path.size() - 1; i > 0; i--)
		{
			RS_Transition_For_Walking* mirror_rs_transition = (RS_Transition_For_Walking*)path[i - 1];
			RS_Transition_For_Walking* rs_transition = new RS_Transition_For_Walking();
			RoadSegment& road_segment = road_segments[rs_transition->road_segment_index];
			path.push_back(rs_transition);
			rs_transition->from_start = !mirror_rs_transition->from_start;

			rs_transition->from_right = mirror_rs_transition->from_right;
			rs_transition->road_segment_index = mirror_rs_transition->road_segment_index;
			i--;
			if (i == 0)
			{
				rs_transition->from_right = !rs_transition->from_right;
				break;
			}
			RN_Transition_For_Walking* mirror_rn_transition = (RN_Transition_For_Walking*)path[i - 1];
			rn_transition = new RN_Transition_For_Walking();
			rn_transition->from_road_segments_array_index = mirror_rn_transition->to_road_segments_array_index;
			rn_transition->to_road_segments_array_index = mirror_rn_transition->from_road_segments_array_index;

			RoadNode& road_node = road_nodes[rn_transition->road_node_index];
			std::vector<u64>& roads = road_node.roadSegments;
			int size = (int)roads.size();
			f32 diff = rn_transition->to_road_segments_array_index - rn_transition->from_road_segments_array_index;
			if (diff > 0.0f)
				rn_transition->accending = diff < (size / 2.0f);
			else if (diff == 0.0f)
				rn_transition->accending = !rs_transition->from_right;
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
	std::vector<RS_Transition_For_Vehicle*> get_path_for_a_car(Building* start, u8 dist)
	{
		auto& road_segments = GameScene::ActiveGameScene->m_RoadManager.road_segments;
		auto& road_nodes = GameScene::ActiveGameScene->m_RoadManager.road_nodes;
		auto& road_types = GameScene::ActiveGameScene->MainApplication->road_types;

		std::vector<Transition*> path{};

		u64 current_road_segment_index = start->connectedRoadSegment;
		RoadSegment& current_road_segment = road_segments[current_road_segment_index];

		RoadType& start_road_type = road_types[current_road_segment.type];

		std::vector<Dijkstra_Node> linqs{};
		std::vector<Dijkstra_Node> fastest_road_to_these_nodes{};
		std::vector<std::tuple<s64, bool>> visited_road_segments{};
		if (start_road_type.has_median)
		{
			Dijkstra_Node node = Dijkstra_Node{
				(s64)(start->snapped_to_right ? current_road_segment.curve_samples.size() - start->snapped_t_index : start->snapped_t_index),
				start->connectedRoadSegment,
				-1,
				(s64)(start->snapped_to_right ? current_road_segment.EndNode : current_road_segment.StartNode)
			};
			linqs.push_back(node);
			visited_road_segments.push_back({ start->connectedRoadSegment, start->snapped_to_right });
			fastest_road_to_these_nodes.push_back(node);
		}
		else if (start_road_type.two_way)
		{
			linqs.push_back(Dijkstra_Node{
					(s64)(current_road_segment.curve_samples.size() - start->snapped_t_index),
					start->connectedRoadSegment,
					-1,
					(s64)(current_road_segment.EndNode)
				});
			//visited_road_segments.push_back({ start->connectedRoadSegment, true });
			linqs.push_back(Dijkstra_Node{
					(s64)(start->snapped_t_index),
					start->connectedRoadSegment,
					-1,
					(s64)(current_road_segment.StartNode)
				});
			//visited_road_segments.push_back({ start->connectedRoadSegment, false });
		}
		else
		{
			Dijkstra_Node node = Dijkstra_Node{
					(s64)(current_road_segment.curve_samples.size() - start->snapped_t_index),
					start->connectedRoadSegment,
					-1,
					(s64)(current_road_segment.EndNode)
			};
			linqs.push_back(node);
			visited_road_segments.push_back({ start->connectedRoadSegment, true });
			fastest_road_to_these_nodes.push_back(node);
		}


		for (u64 i = 0; i < dist; ++i)
		{
			u64 size = linqs.size();
			for (u64 j = size - 1; j != (u64)-1; --j)
			{
				auto& next_linq = linqs[j];

				auto fastest_it = std::find_if(
					fastest_road_to_these_nodes.begin(),
					fastest_road_to_these_nodes.end(),
					[next_linq](const Dijkstra_Node& el) {
						return el.next_road_node_index == next_linq.next_road_node_index;
					});
				if (fastest_it == fastest_road_to_these_nodes.end())
				{
					fastest_road_to_these_nodes.push_back(next_linq);
				}

				s64 road_node_index = next_linq.next_road_node_index;
				RoadNode& road_node = road_nodes[road_node_index];
				auto& connected_road_segments = road_node.roadSegments;
				for (u64 i = 0; i < connected_road_segments.size(); i++)
				{
					u64 connected_road_segment_index = connected_road_segments[i];
					RoadSegment& connected_road_segment = road_segments[connected_road_segment_index];
					u64 curve_samples_count = connected_road_segment.curve_samples.size();
					bool from_start = road_node_index == connected_road_segment.StartNode;
					RoadType& type = road_types[connected_road_segment.type];
					if (type.two_way == false && connected_road_segment.EndNode == road_node_index) continue;

					s64 new_distance = next_linq.distance + curve_samples_count;
					s64 next_road_node_index = connected_road_segment.StartNode == road_node_index ? connected_road_segment.EndNode : connected_road_segment.StartNode;
					s64 prev_road_node_index = road_node_index;
					Dijkstra_Node next_node = Dijkstra_Node{
						(s64)new_distance,
						(s64)connected_road_segment_index,
						(s64)prev_road_node_index,
						(s64)next_road_node_index
					};
					linqs.push_back(next_node);
				}
			}
			linqs.erase(linqs.begin(), linqs.begin() + size);
		}

		while (linqs.empty() == false)
		{
			std::sort(linqs.begin(), linqs.end(), Helper::sort_by_distance());
			Dijkstra_Node closest = linqs[linqs.size() - 1];
			s64 road_node_index = closest.next_road_node_index;
			linqs.pop_back();

			auto fastest_it = std::find_if(
				fastest_road_to_these_nodes.begin(),
				fastest_road_to_these_nodes.end(),
				[road_node_index](const Dijkstra_Node& el) {
					return el.next_road_node_index == road_node_index;
				});
			if (fastest_it == fastest_road_to_these_nodes.end())
			{
				fastest_road_to_these_nodes.push_back(closest);
			}

			RoadNode& road_node = road_nodes[road_node_index];
			auto& connected_road_segments = road_node.roadSegments;
			for (u64 i = 0; i < connected_road_segments.size(); i++)
			{
				u64 connected_road_segment_index = connected_road_segments[i];
				RoadSegment& connected_road_segment = road_segments[connected_road_segment_index];
				u64 curve_samples_count = connected_road_segment.curve_samples.size();
				bool from_start = road_node_index == connected_road_segment.StartNode;
				RoadType& type = road_types[connected_road_segment.type];
				if (type.two_way == false && connected_road_segment.EndNode == road_node_index) continue;

				if (connected_road_segment_index == start->connectedRoadSegment)
				{
					if (from_start == start->snapped_to_right || type.has_median == false)
					{
						u64 rs_index{ connected_road_segment_index };
						std::vector<RS_Transition_For_Vehicle*> the_temp_path{};

						RS_Transition_For_Vehicle* temp_rs_transition{ new RS_Transition_For_Vehicle() };
						the_temp_path.push_back(temp_rs_transition);
						temp_rs_transition->road_segment_index = rs_index;

						while (road_node_index != -1)
						{
							auto linq_it = std::find_if(
								fastest_road_to_these_nodes.begin(),
								fastest_road_to_these_nodes.end(),
								[road_node_index](const Dijkstra_Node& el) {
									return el.next_road_node_index == road_node_index;
								});
							assert(linq_it != fastest_road_to_these_nodes.end());
							rs_index = linq_it->road_segment_index;

							temp_rs_transition = new RS_Transition_For_Vehicle();
							the_temp_path.push_back(temp_rs_transition);
							temp_rs_transition->road_segment_index = rs_index;
							temp_rs_transition->next_road_node_index = road_node_index;

							road_node_index = linq_it->prev_road_node_index;
						}

						u64 transition_count{ the_temp_path.size() };
						std::vector<RS_Transition_For_Vehicle*> the_path{};
						the_path.reserve(transition_count);
						while (the_temp_path.size() > 0)
						{
							the_path.push_back(the_temp_path[the_temp_path.size() - 1]);
							the_temp_path.pop_back();
						}

						fill_points_stack(the_path, start, start);
						return the_path;
					}
				}
				auto v_it = std::find_if(
					visited_road_segments.begin(),
					visited_road_segments.end(),
					[connected_road_segment_index, from_start](const std::tuple<s64, bool>& el) {
						return std::get<0>(el) == connected_road_segment_index && std::get<1>(el) == from_start;
					});
				if (v_it != visited_road_segments.end())
					continue;

				visited_road_segments.push_back({ connected_road_segment_index, from_start });


				s64 new_distance = closest.distance + curve_samples_count;
				s64 next_road_node_index = connected_road_segment.StartNode == road_node_index ? connected_road_segment.EndNode : connected_road_segment.StartNode;
				s64 prev_road_node_index = road_node_index;
				linqs.push_back(Dijkstra_Node{
					(s64)new_distance,
					(s64)connected_road_segment_index,
					(s64)prev_road_node_index,
					(s64)next_road_node_index
					});
			}
		}

		return {};
	}

	//TODO memory leak possible
	std::vector<Transition*> get_path(Building* start, Building* end)
	{
		auto& road_segments = GameScene::ActiveGameScene->m_RoadManager.road_segments;
		auto& road_nodes = GameScene::ActiveGameScene->m_RoadManager.road_nodes;
		auto& road_types = GameScene::ActiveGameScene->MainApplication->road_types;
		if (start->connectedRoadSegment == end->connectedRoadSegment)
		{
			if (start->snapped_to_right == end->snapped_to_right)
			{
				RS_Transition_For_Walking* rs_transition = new RS_Transition_For_Walking();
				u64 diff = end->snapped_t_index - start->snapped_t_index;
				if (diff > 0)
				{
					rs_transition->from_start = true;
					rs_transition->from_right = start->snapped_to_right;
				}
				else if (diff == 0)
				{
					rs_transition->from_start = end->snapped_t > start->snapped_t;
					rs_transition->from_right = start->snapped_to_right == (end->snapped_t > start->snapped_t);
				}
				else
				{
					rs_transition->from_start = false;
					rs_transition->from_right = !start->snapped_to_right;
				}
				rs_transition->road_segment_index = start->connectedRoadSegment;
				return { rs_transition };
			}
			else
			{
				RoadSegment& road_segment = road_segments[start->connectedRoadSegment];
				bool from_start = road_segment.curve_samples.size() > start->snapped_t_index + end->snapped_t_index;

				RS_Transition_For_Walking* rs_transition_s = new RS_Transition_For_Walking();
				rs_transition_s->from_start = !from_start;
				rs_transition_s->from_right = !start->snapped_to_right == from_start;
				rs_transition_s->road_segment_index = start->connectedRoadSegment;

				u64 road_node_index = from_start ? road_segment.StartNode : road_segment.EndNode;
				RoadNode& road_node = road_nodes[road_node_index];
				auto it = std::find(road_node.roadSegments.begin(), road_node.roadSegments.end(), start->connectedRoadSegment);
				assert(it != road_node.roadSegments.end());
				u64 index = std::distance(road_node.roadSegments.begin(), it);
				RN_Transition_For_Walking* rn_transition = new RN_Transition_For_Walking();
				rn_transition->from_road_segments_array_index = index;
				rn_transition->to_road_segments_array_index = index;
				rn_transition->road_node_index = road_node_index;
				if (rs_transition_s->from_right)
				{
					rn_transition->accending = false;
					rn_transition->sub_index = 2;
				}
				else
				{
					rn_transition->accending = true;
					rn_transition->sub_index = 1;
				}

				RS_Transition_For_Walking* rs_transition_e = new RS_Transition_For_Walking();

				rs_transition_e->from_start = !rs_transition_s->from_start;
				rs_transition_s->from_right = rs_transition_s->from_right == from_start;
				rs_transition_e->road_segment_index = end->connectedRoadSegment;
				return { rs_transition_s, rn_transition, rs_transition_e };
			}
		}

		RoadSegment& start_road_segment = road_segments[start->connectedRoadSegment];
		RoadSegment& end_road_segment = road_segments[end->connectedRoadSegment];

		std::vector<Dijkstra_Node> linqs{};
		std::vector<Dijkstra_Node> fastest_road_to_these_nodes{};
		std::vector<u64> visited_road_segments{};
		linqs.push_back(Dijkstra_Node{
			start->snapped_t_index,
			start->connectedRoadSegment,
			-1,
			(s64)(start_road_segment.StartNode)
			});
		linqs.push_back(Dijkstra_Node{
			(s64)(start_road_segment.curve_samples.size() - start->snapped_t_index),
			start->connectedRoadSegment,
			-1,
			(s64)(start_road_segment.EndNode)
			});
		visited_road_segments.push_back(start->connectedRoadSegment);


		while (linqs.empty() == false)
		{
			std::sort(linqs.begin(), linqs.end(), Helper::sort_by_distance());
			auto closest = linqs[linqs.size() - 1];
			linqs.pop_back();
			auto it = std::find_if(
				fastest_road_to_these_nodes.begin(),
				fastest_road_to_these_nodes.end(),
				[closest](const Dijkstra_Node& el) {
					return el.next_road_node_index == closest.next_road_node_index;
				});
			if (it == fastest_road_to_these_nodes.end())
			{
				fastest_road_to_these_nodes.push_back(closest);
			}

			RoadNode& road_node = road_nodes[closest.next_road_node_index];
			auto& connected_road_segments = road_node.roadSegments;
			for (u64 i = 0; i < connected_road_segments.size(); i++)
			{
				u64 connected_road_segment_index = connected_road_segments[i];
				RoadSegment& connected_road_segment = road_segments[connected_road_segment_index];
				u64 curve_samples_count = connected_road_segment.curve_samples.size();
				if (connected_road_segment_index == end->connectedRoadSegment)
				{
					u64 rn_index = closest.next_road_node_index;
					u64 rs_index = connected_road_segment_index;
					std::vector<Transition*> the_temp_path{};

					RS_Transition_For_Walking* rs_transition = new RS_Transition_For_Walking();
					the_temp_path.push_back(rs_transition);
					rs_transition->road_segment_index = rs_index;
					rs_transition->from_start = connected_road_segment.StartNode == closest.next_road_node_index;
					rs_transition->from_right = end->snapped_to_right == rs_transition->from_start;

					auto& rss = road_nodes[rn_index].roadSegments;
					auto to_it = std::find(rss.begin(), rss.end(), rs_index);
					assert(to_it != rss.end());
					u64 to_index = std::distance(rss.begin(), to_it);

					RN_Transition_For_Walking* rn_transition = new RN_Transition_For_Walking();
					the_temp_path.push_back(rn_transition);
					rn_transition->road_node_index = rn_index;
					rn_transition->to_road_segments_array_index = to_index;
					while (true)
					{
						auto linq_it = std::find_if(
							fastest_road_to_these_nodes.begin(),
							fastest_road_to_these_nodes.end(),
							[rn_index](const Dijkstra_Node& el) {
								return el.next_road_node_index == rn_index;
							});
						rs_index = linq_it->road_segment_index;
						std::vector<u64>& roads = road_nodes[rn_index].roadSegments;
						auto from_it = std::find(roads.begin(), roads.end(), rs_index);
						assert(from_it != roads.end());
						u64 from_index = std::distance(roads.begin(), from_it);
						rn_transition->from_road_segments_array_index = from_index;
						rn_transition->sub_index = rs_transition->from_right ? 2 : 1;

						int size = (int)roads.size();
						f32 diff = rn_transition->to_road_segments_array_index - rn_transition->from_road_segments_array_index;
						if (diff > 0.0f)
							rn_transition->accending = diff < (size / 2.0f);
						else if (diff == 0.0f)
							rn_transition->accending = !rs_transition->from_right;
						else
							rn_transition->accending = diff * -1.0f > (size / 2.0f);


						RoadSegment& prev_road_segment = road_segments[rs_index];
						assert(rs_transition->at_path_array_index < 0xFFFF);
						rs_transition = new RS_Transition_For_Walking();
						the_temp_path.push_back(rs_transition);
						rs_transition->road_segment_index = rs_index;
						rs_transition->from_right = true;
						rs_transition->from_start = prev_road_segment.EndNode == rn_index;

						rn_index = linq_it->prev_road_node_index;

						if (rn_index == -1)
						{
							rs_transition->from_right = start->snapped_to_right == rs_transition->from_start;
							break;
						}

						auto& rss = road_nodes[rn_index].roadSegments;
						to_it = std::find(rss.begin(), rss.end(), rs_index);
						assert(to_it != rss.end());
						to_index = std::distance(rss.begin(), to_it);

						rn_transition = new RN_Transition_For_Walking();
						the_temp_path.push_back(rn_transition);
						rn_transition->road_node_index = rn_index;
						rn_transition->to_road_segments_array_index = to_index;
					}
					std::vector<Transition*> the_path{};
					the_path.reserve(the_temp_path.size());
					while (the_temp_path.size() > 0)
					{
						Transition* transition = the_temp_path[the_temp_path.size() - 1];
						the_temp_path.pop_back();
						the_path.push_back(transition);
					}
					return the_path;
				}
				if (road_types[connected_road_segment.type].zoneable == false) continue;

				auto v_it = std::find(visited_road_segments.begin(), visited_road_segments.end(), connected_road_segment_index);
				if (v_it != visited_road_segments.end())
					continue;

				visited_road_segments.push_back(connected_road_segment_index);


				s64 new_distance = closest.distance + curve_samples_count;
				s64 next_road_node_index = connected_road_segment.StartNode == closest.next_road_node_index ? connected_road_segment.EndNode : connected_road_segment.StartNode;
				s64 prev_road_node_index = connected_road_segment.StartNode == closest.next_road_node_index ? connected_road_segment.StartNode : connected_road_segment.EndNode;
				linqs.push_back(Dijkstra_Node{
					new_distance,
					(s64)(connected_road_segment_index),
					prev_road_node_index,
					next_road_node_index
					});
			}
		}
		return {};
	}
	std::vector<RS_Transition_For_Vehicle*> get_path_for_a_car(Building* start, Building* end)
	{
		auto& road_segments{ GameScene::ActiveGameScene->m_RoadManager.road_segments };
		auto& road_nodes{ GameScene::ActiveGameScene->m_RoadManager.road_nodes };
		auto& road_types{ GameScene::ActiveGameScene->MainApplication->road_types };

		RoadSegment& start_road_segment{ road_segments[start->connectedRoadSegment] };
		RoadSegment& end_road_segment{ road_segments[end->connectedRoadSegment] };

		RoadType& start_road_type{ road_types[start_road_segment.type] };

		if (start->connectedRoadSegment == end->connectedRoadSegment)
		{
			auto& start_road_type{ road_types[start_road_segment.type] };
			if (start_road_type.two_way == false)
			{
				if (end->snapped_t_index >= start->snapped_t_index)
				{
					RS_Transition_For_Vehicle* rs_transition{ new RS_Transition_For_Vehicle() };
					rs_transition->road_segment_index = start->connectedRoadSegment;
					if (end->snapped_to_right)
					{
						rs_transition->lane_index = start_road_type.lanes_backward.size();
						rs_transition->lane_index += start_road_type.lanes_forward.size() - 2;
					}
					else
					{
						rs_transition->lane_index = 1;
					}
					std::vector<RS_Transition_For_Vehicle*> the_path{ rs_transition };
					fill_points_stack(the_path, start, end);
					return the_path;
				}
			}
			else if (start_road_type.has_median == false)
			{
				RS_Transition_For_Vehicle* rs_transition{ new RS_Transition_For_Vehicle() };
				if (end->snapped_t_index >= start->snapped_t_index)
				{
					rs_transition->road_segment_index = start->connectedRoadSegment;
					if (end->snapped_to_right)
					{
						rs_transition->lane_index = start_road_type.lanes_forward.size() - 2;
						rs_transition->lane_index += start_road_type.lanes_backward.size();
					}
					else
					{
						rs_transition->lane_index = 0;
						rs_transition->lane_index += start_road_type.lanes_backward.size();
					}
				}
				else
				{
					rs_transition->road_segment_index = start->connectedRoadSegment;
					if (end->snapped_to_right)
					{
						rs_transition->lane_index = start_road_type.lanes_backward.size() - 1;
					}
					else
					{
						rs_transition->lane_index = 1;
					}
				}
				std::vector<RS_Transition_For_Vehicle*> the_path{ rs_transition };
				fill_points_stack(the_path, start, end);
				return the_path;
			}
			else
			{
				std::vector<RS_Transition_For_Vehicle*> the_path{};
				if (start->snapped_to_right)
				{
					if (end->snapped_to_right)
					{
						if (end->snapped_t_index >= start->snapped_t_index)
						{
							RS_Transition_For_Vehicle* rs_transition{ new RS_Transition_For_Vehicle() };
							rs_transition->road_segment_index = start->connectedRoadSegment;
							rs_transition->lane_index = start_road_type.lanes_forward.size() - 2;
							rs_transition->lane_index += start_road_type.lanes_backward.size();

							the_path.push_back(rs_transition);
						}
						else
						{
							RS_Transition_For_Vehicle* rs_transition_1{ new RS_Transition_For_Vehicle() };
							rs_transition_1->road_segment_index = start->connectedRoadSegment;
							rs_transition_1->lane_index = 0;
							rs_transition_1->lane_index += start_road_type.lanes_backward.size();

							RS_Transition_For_Vehicle* rs_transition_2{ new RS_Transition_For_Vehicle() };
							rs_transition_2->road_segment_index = start->connectedRoadSegment;
							rs_transition_2->lane_index = start_road_type.lanes_backward.size() - 1;

							RS_Transition_For_Vehicle* rs_transition_3{ new RS_Transition_For_Vehicle() };
							rs_transition_3->road_segment_index = start->connectedRoadSegment;
							rs_transition_3->lane_index = start_road_type.lanes_forward.size() - 2;
							rs_transition_3->lane_index += start_road_type.lanes_backward.size();

							the_path.push_back(rs_transition_1);
							the_path.push_back(rs_transition_2);
							the_path.push_back(rs_transition_3);
						}
					}
					else
					{
						RS_Transition_For_Vehicle* rs_transition_1{ new RS_Transition_For_Vehicle() };
						rs_transition_1->road_segment_index = start->connectedRoadSegment;
						rs_transition_1->lane_index = 0;
						rs_transition_1->lane_index += start_road_type.lanes_backward.size();

						RS_Transition_For_Vehicle* rs_transition_2{ new RS_Transition_For_Vehicle() };
						rs_transition_2->road_segment_index = start->connectedRoadSegment;
						rs_transition_2->lane_index = 1;

						the_path.push_back(rs_transition_1);
						the_path.push_back(rs_transition_2);
					}
				}
				else
				{
					if (end->snapped_to_right)
					{
						RS_Transition_For_Vehicle* rs_transition_1{ new RS_Transition_For_Vehicle() };
						rs_transition_1->road_segment_index = start->connectedRoadSegment;
						rs_transition_1->lane_index = start_road_type.lanes_backward.size() - 1;

						RS_Transition_For_Vehicle* rs_transition_2{ new RS_Transition_For_Vehicle() };
						rs_transition_2->road_segment_index = start->connectedRoadSegment;
						rs_transition_2->lane_index = start_road_type.lanes_forward.size() - 2;
						rs_transition_2->lane_index += start_road_type.lanes_backward.size();

						the_path.push_back(rs_transition_1);
						the_path.push_back(rs_transition_2);
					}
					else
					{
						if (end->snapped_t_index >= start->snapped_t_index)
						{
							RS_Transition_For_Vehicle* rs_transition_1{ new RS_Transition_For_Vehicle() };
							rs_transition_1->road_segment_index = start->connectedRoadSegment;
							rs_transition_1->lane_index = start_road_type.lanes_backward.size() - 1;

							RS_Transition_For_Vehicle* rs_transition_2{ new RS_Transition_For_Vehicle() };
							rs_transition_2->road_segment_index = start->connectedRoadSegment;
							rs_transition_2->lane_index = 0;
							rs_transition_2->lane_index += start_road_type.lanes_backward.size();

							RS_Transition_For_Vehicle* rs_transition_3{ new RS_Transition_For_Vehicle() };
							rs_transition_3->road_segment_index = start->connectedRoadSegment;
							rs_transition_3->lane_index = 1;

							the_path.push_back(rs_transition_1);
							the_path.push_back(rs_transition_2);
							the_path.push_back(rs_transition_3);
						}
						else
						{
							RS_Transition_For_Vehicle* rs_transition{ new RS_Transition_For_Vehicle() };
							rs_transition->road_segment_index = start->connectedRoadSegment;
							rs_transition->lane_index = 1;

							the_path.push_back(rs_transition);
						}
					}
				}
				fill_points_stack(the_path, start, end);
				return the_path;
			}

		}

		std::vector<Dijkstra_Node> linqs{};
		std::vector<Dijkstra_Node> fastest_road_to_these_nodes{};
		std::vector<std::tuple<s64, bool>> visited_road_segments{};
		if (start_road_type.has_median)
		{
			linqs.push_back(Dijkstra_Node{
				(s64)(start->snapped_to_right ? start_road_segment.curve_samples.size() - start->snapped_t_index : start->snapped_t_index),
				start->connectedRoadSegment,
				-1,
				(s64)(start->snapped_to_right ? start_road_segment.EndNode : start_road_segment.StartNode)
				});
			visited_road_segments.push_back({ start->connectedRoadSegment, start->snapped_to_right });
		}
		else if (start_road_type.two_way)
		{
			linqs.push_back(Dijkstra_Node{
					(s64)(start_road_segment.curve_samples.size() - start->snapped_t_index),
					start->connectedRoadSegment,
					-1,
					(s64)(start_road_segment.EndNode)
				});
			visited_road_segments.push_back({ start->connectedRoadSegment, true });
			linqs.push_back(Dijkstra_Node{
					(s64)(start->snapped_t_index),
					start->connectedRoadSegment,
					-1,
					(s64)(start_road_segment.StartNode)
				});
			visited_road_segments.push_back({ start->connectedRoadSegment, false });
		}
		else
		{
			linqs.push_back(Dijkstra_Node{
					(s64)(start_road_segment.curve_samples.size() - start->snapped_t_index),
					start->connectedRoadSegment,
					-1,
					(s64)(start_road_segment.EndNode)
				});
			visited_road_segments.push_back({ start->connectedRoadSegment, true });
		}


		while (linqs.empty() == false)
		{
			std::sort(linqs.begin(), linqs.end(), Helper::sort_by_distance());
			auto closest = linqs[linqs.size() - 1];
			s64 road_node_index = closest.next_road_node_index;
			linqs.pop_back();

			auto fastest_it = std::find_if(
				fastest_road_to_these_nodes.begin(),
				fastest_road_to_these_nodes.end(),
				[road_node_index](const Dijkstra_Node& el) {
					return el.next_road_node_index == road_node_index;
				});
			if (fastest_it == fastest_road_to_these_nodes.end())
			{
				fastest_road_to_these_nodes.push_back(closest);
			}

			RoadNode& road_node = road_nodes[road_node_index];
			auto& connected_road_segments = road_node.roadSegments;
			for (u64 i = 0; i < connected_road_segments.size(); i++)
			{
				u64 connected_road_segment_index = connected_road_segments[i];
				RoadSegment& connected_road_segment = road_segments[connected_road_segment_index];
				u64 curve_samples_count = connected_road_segment.curve_samples.size();
				bool from_start = road_node_index == connected_road_segment.StartNode;
				RoadType& type = road_types[connected_road_segment.type];
				if (type.two_way == false && connected_road_segment.EndNode == road_node_index) continue;

				if (connected_road_segment_index == end->connectedRoadSegment)
				{
					if (from_start == end->snapped_to_right || type.has_median == false)
					{
						u64 rs_index = connected_road_segment_index;
						std::vector<RS_Transition_For_Vehicle*> the_temp_path{};

						RS_Transition_For_Vehicle* temp_rs_transition{ new RS_Transition_For_Vehicle() };
						the_temp_path.push_back(temp_rs_transition);
						temp_rs_transition->road_segment_index = rs_index;

						while (road_node_index != -1)
						{
							auto linq_it = std::find_if(
								fastest_road_to_these_nodes.begin(),
								fastest_road_to_these_nodes.end(),
								[road_node_index](const Dijkstra_Node& el) {
									return el.next_road_node_index == road_node_index;
								});
							assert(linq_it != fastest_road_to_these_nodes.end());
							rs_index = linq_it->road_segment_index;

							temp_rs_transition = new RS_Transition_For_Vehicle();
							the_temp_path.push_back(temp_rs_transition);
							temp_rs_transition->road_segment_index = rs_index;
							temp_rs_transition->next_road_node_index = road_node_index;

							road_node_index = linq_it->prev_road_node_index;
						}

						u64 transition_count = the_temp_path.size();
						std::vector<RS_Transition_For_Vehicle*> the_path{};
						the_path.reserve(transition_count);
						while (the_temp_path.size() > 0)
						{
							the_path.push_back(the_temp_path[the_temp_path.size() - 1]);
							the_temp_path.pop_back();
						}

						fill_points_stack(the_path, start, end);
						return the_path;
					}
				}
				auto v_it = std::find_if(
					visited_road_segments.begin(),
					visited_road_segments.end(),
					[connected_road_segment_index, from_start](const std::tuple<s64, bool>& el) {
						return std::get<0>(el) == connected_road_segment_index && std::get<1>(el) == from_start;
					});
				if (v_it != visited_road_segments.end())
					continue;

				visited_road_segments.push_back({ connected_road_segment_index, from_start });


				s64 new_distance = closest.distance + curve_samples_count;
				s64 next_road_node_index = connected_road_segment.StartNode == road_node_index ? connected_road_segment.EndNode : connected_road_segment.StartNode;
				s64 prev_road_node_index = road_node_index;
				linqs.push_back(Dijkstra_Node{
					(s64)new_distance,
					(s64)connected_road_segment_index,
					(s64)prev_road_node_index,
					(s64)next_road_node_index
					});
			}
		}

		return {};
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