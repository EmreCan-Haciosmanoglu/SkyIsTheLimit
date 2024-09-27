#pragma once

#include "Can/Renderer/Prefab.h"

namespace Can
{
	struct Lane
	{
		float distance_from_center = 0.0f;
		float speed_limit = 0.0f;
		float width = 0.0f; // maybe?
		// And other stuff??
		// lane type? => 
		//       for car, for tram for * 
		//    or road, railway, route for ship|plane|helicopter|*, *
		//    or car lane, bike lane, side walk, median, trees, parking, *
	};
	struct Road_Type
	{
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
		bool has_median = false;
		bool two_way = true;
		bool left_hand_drive = true; // for later
		bool zoneable = true;

		std::vector<Lane> lanes_backward{ Lane{-0.5f, 5.0f, 0.05f}, Lane{-0.25f, 10.0f, 0.05f} };
		std::vector<Lane> lanes_forward{ Lane{0.25f, 10.0f, 0.05f},  Lane{0.5f, 5.0f, 0.05f} };
	};
}