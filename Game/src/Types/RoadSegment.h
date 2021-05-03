#pragma once
#include <array>
#include <vector>

#include "RoadType.h"

namespace Can
{
	class Car;
	class Prefab;
	class Object;
	class Building;
	class RoadSegment;
	class RoadNode;

	class RoadSegment
	{
	public:
		RoadSegment(
			const std::array<Prefab*, 3>& type,
			const std::array<glm::vec3, 4>& curvePoints
		);
		RoadSegment(RoadSegment&& other);
		~RoadSegment();

		RoadSegment& operator=(RoadSegment&& other);
		friend bool operator==(const RoadSegment& left, const RoadSegment& right) { return &left == &right; }

		void ReConstruct();
		void ChangeType(const std::array<Prefab*, 3>& type);

		inline const std::array<glm::vec3, 4>& GetCurvePoints() const { return CurvePoints; }
		inline const glm::vec3& GetCurvePoint(size_t index) const { return CurvePoints[index]; }
		inline glm::vec3 GetCurvePoint(size_t index) { return CurvePoints[index]; }

		inline const glm::vec3& GetStartPosition() const { return CurvePoints[0]; }
		inline const glm::vec3& GetEndPosition() const { return CurvePoints[3]; }

		void SetStartPosition(const glm::vec3& position);
		void SetEndPosition(const glm::vec3& position);
		void SetCurvePoints(const std::array<glm::vec3, 4>& curvePoints);
		void SetCurvePoint(size_t index, const glm::vec3& curvePoint);

		inline const glm::vec2& GetStartRotation() const { return Rotations[0]; }
		inline const glm::vec2& GetEndRotation() const { return Rotations[1]; }

		inline const glm::vec3& GetStartDirection() const { return Directions[0]; }
		inline const glm::vec3& GetEndDirection() const { return Directions[1]; }

	private:
		void Construct();
		void CalcRotsAndDirs();

	public:
		RoadType road_type{};
		std::vector<Building*> Buildings = {};
		std::vector<Car*> Cars = {};

		u64 StartNode = (u64)-1;
		u64 EndNode = (u64)-1;

		std::vector<glm::vec3> curve_samples{};
		std::vector<float> curve_t_samples{};

		Object* object = nullptr;

		std::array<glm::vec3, 4> CurvePoints;
		std::array<std::array<glm::vec2, 3>, 2> bounding_box{};
	private:

		std::array<glm::vec3, 2> Directions;
		std::array<glm::vec2, 2> Rotations{
			glm::vec2(0.0f),
			glm::vec2(0.0f)
		};
};
}