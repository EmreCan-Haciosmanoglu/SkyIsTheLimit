#pragma once
#include <array>
#include <vector>

namespace Can
{
	class End;
	class Car;
	class Prefab;
	class Object;
	class Junction;
	class Building;
	class RoadSegment;

	struct ConnectedObject
	{
		End* end = nullptr;
		Junction* junction = nullptr;
		RoadSegment* roadSegment = nullptr;
	};

	class RoadSegment
	{
	public:
		RoadSegment(
			const std::array<Prefab*, 3>& type,
			const std::array<glm::vec3, 4>& curvePoints
		);
		~RoadSegment();

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
		std::array<Prefab*, 3> Type;
		std::vector<Building*> Buildings = {};
		std::vector<Car*> Cars = {};

		ConnectedObject ConnectedObjectAtStart;
		ConnectedObject ConnectedObjectAtEnd;

		Object* object = nullptr;

	private:
		std::array<glm::vec3, 4> CurvePoints;

		std::array<glm::vec3, 2> Directions;
		std::array<glm::vec2, 2> Rotations{
			glm::vec2(0.0f),
			glm::vec2(0.0f)
		};

		float length = 0.0f; // ????
	};
}