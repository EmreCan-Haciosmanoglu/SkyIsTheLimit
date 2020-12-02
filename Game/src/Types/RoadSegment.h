#pragma once
#include <array>
#include <vector>

namespace Can
{
	class End;
	class Prefab;
	class Object;
	class Junction;
	class Building;
	class RoadSegment;

	class ConnectedObject
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
		void ChangeType(Prefab* type);

		inline const std::array<glm::vec3, 4>& GetCurvePoints() const { return CurvePoints; }

		inline const glm::vec3& GetStartPosition() const { return CurvePoints[0]; }
		inline const glm::vec3& GetEndPosition() const { return CurvePoints[3]; }

		void SetStartPosition(const glm::vec3& position);
		void SetEndPosition(const glm::vec3& position);

	private:
		void Construct();

	private:
		std::array<Prefab*, 3> Type;
		std::array<glm::vec3, 4> CurvePoints;
		std::vector<Building*> Buildings = {};

		std::array<glm::vec3, 2> Directions;
		std::array<glm::vec2, 2> Rotations{
			glm::vec2(0.0f),
			glm::vec2(0.0f)
		};

		float length = 0.0f; // ????

		Object* object = nullptr;

		ConnectedObject ConnectedObjectAtStart;
		ConnectedObject ConnectedObjectAtEnd;
	};
}