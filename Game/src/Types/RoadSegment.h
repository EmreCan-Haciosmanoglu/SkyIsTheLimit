#pragma once
#include <array>
#include <vector>

#include "RoadType.h"
#include "Can/Unordered_Array.h"

namespace Can
{
	class Car;
	class Object;
	class Building;
	class Person;
	class RoadNode;

	class RoadSegment
	{
	public:
		RoadSegment() {}
		RoadSegment(
			u64 type,
			const std::array<v3, 4>& curvePoints,
			s8 elevation_type
		);
		RoadSegment(RoadSegment&& other);
		~RoadSegment();

		RoadSegment& operator=(RoadSegment&& other);
		friend bool operator==(const RoadSegment& left, const RoadSegment& right) { return &left == &right; }

		void ReConstruct();
		void SetType(u64 type) { this->type = type; ReConstruct(); }

		inline const std::array<v3, 4>& GetCurvePoints() const { return CurvePoints; }
		inline const v3& GetCurvePoint(size_t index) const { return CurvePoints[index]; }
		inline v3 GetCurvePoint(size_t index) { return CurvePoints[index]; }

		inline const v3& GetStartPosition() const { return CurvePoints[0]; }
		inline void SetStartPosition(const v3& position) { SetCurvePoint(0, position); }

		inline const v3& GetEndPosition() const { return CurvePoints[3]; }
		inline void SetEndPosition(const v3& position) { SetCurvePoint(3, position); }

		void SetCurvePoints(const std::array<v3, 4>& curvePoints);
		void SetCurvePoint(u64 index, const v3& curvePoint);

		inline const v2& GetStartRotation() const { return Rotations[0]; }
		inline const v2& GetEndRotation() const { return Rotations[1]; }

		inline const v3& GetStartDirection() const { return Directions[0]; }
		inline const v3& GetEndDirection() const { return Directions[1]; }

		void Construct();
		void CalcRotsAndDirs();

		static void move(RoadSegment* dest, RoadSegment* src);
		static void remove(RoadSegment* obj);

		u64 type = 0;
		std::vector<Building*> Buildings = {};
		std::vector<Car*> Cars = {};
		std::vector<Person*> peoples = {};

		u64 StartNode = (u64)-1;
		u64 EndNode = (u64)-1;

		std::vector<v3> curve_samples{};
		std::vector<float> curve_t_samples{};

		Object* object = nullptr;

		std::array<v3, 4> CurvePoints{};
		std::array<std::array<v2, 3>, 2> bounding_rect{};
		std::vector<std::array<v3, 3>> bounding_polygon{};

		// better name
		s8 elevation_type = 0;
		// -1 tunnel
		// +0 ground
		// +1 bridge

	private:

		std::array<v3, 2> Directions{};
		std::array<v2, 2> Rotations{
			v2(0.0f),
			v2(0.0f)
		};
	};
}