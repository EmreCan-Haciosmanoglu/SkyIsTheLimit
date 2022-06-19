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
		Prefab* road_junction = nullptr;
		Prefab* road_junction_mirror = nullptr;
		Prefab* road_end = nullptr;
		Prefab* road_end_mirror = nullptr;

		Prefab* tunnel = nullptr;
		Prefab* tunnel_junction = nullptr;
		Prefab* tunnel_junction_mirror = nullptr;
		Prefab* tunnel_end = nullptr;
		Prefab* tunnel_end_mirror = nullptr;

		Prefab* tunnel_entrance = nullptr;
		Prefab* tunnel_entrance_mirror = nullptr;

		Ref<Texture2D> thumbnail;

		f32 road_width = 0.01f;
		f32 road_length = 0.01f;
		f32 road_height = 0.01f;
		f32 road_junction_length = 0.01f;

		f32 tunnel_width = 0.01f;
		f32 tunnel_length = 0.01f;
		f32 tunnel_height = 0.01f;
		f32 tunnel_junction_length = 0.01f;

		f32 bridge_height = 0.01f; // for later

		bool asymmetric = false;
		bool left_hand_drive = true; // for later
		bool zoneable = true;
		bool has_sidewalk = true; // change later

		std::vector<Lane> left;
		std::vector<Lane> rigth;
	};
}