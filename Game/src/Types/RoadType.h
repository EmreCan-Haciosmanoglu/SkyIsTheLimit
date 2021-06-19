#pragma once

#include "Can/Renderer/Prefab.h"

namespace Can
{
	struct Lane
	{
		float distance_from_center;
		float speed_limit;
		float width; // maybe?
		// And other stuff??
		// lane type? => 
		//       for car, for tram for * 
		//    or road, railway, route for ship|plane|helicopter|*, *
		//    or car lane, bike lane, side walk, median, trees, parking, *
	};
	class RoadType
	{
	public:
		std::string name = "Unnamed Road";
		
		Prefab* road = nullptr;
		Prefab* junction = nullptr;
		Prefab* junction_mirror = nullptr;
		Prefab* end = nullptr;
		Prefab* end_mirror = nullptr;
		Ref<Texture2D> thumbnail;

		float road_width;
		float road_length;
		float junction_length;

		bool asymmetric = false;
		bool left_hand_drive = true; // for later
		bool zoneable = true;

		std::vector<Lane> left;
		std::vector<Lane> rigth;
	};
}