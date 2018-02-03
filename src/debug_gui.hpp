#ifndef __DEBUG_GUI_HPP__
#define __DEBUG_GUI_HPP__

// #include <vector>
// #include <utility>
#include <unordered_map>
#include "lt_core.hpp"
#include "lt_math.hpp"
#include "entities.hpp"

#define PERFORMANCE_KINDS \
	PERFORMANCE_KIND(PerformanceRegion_RenderLoop = 0, "Render loop"), \
	PERFORMANCE_KIND(PerformanceRegion_DrawEntities, "Draw entities"), \
	PERFORMANCE_KIND(PerformanceRegion_UpdateLoop, "Update loop"),

struct GLFWwindow;

enum PerformanceRegion
{
#define PERFORMANCE_KIND(e, s) e
	PERFORMANCE_KINDS
#undef PERFORMANCE_KIND
	PerformanceRegion_Count,
};

lt_internal const char *PerformanceRegionNames[] =
{
#define PERFORMANCE_KIND(e, s) s
	PERFORMANCE_KINDS
#undef PERFORMANCE_KIND
};


namespace dgui
{

static_assert(LT_Count(PerformanceRegionNames) == PerformanceRegion_Count,
			  "Both the char array and the enum should match");

struct State
{
	bool enable_normal_mapping = true;
	bool enable_multisampling = true;
	bool draw_shadow_map = false;
	bool enable_interpolation = true;
	bool enable_tone_mapping = true;
	bool enable_gamma_correction = true;
	bool enable_bloom = false;
	bool display_bloom_filter = false;
	f32  bloom_threshold = 1.0f;
	f32  exposure = 1.0f;
	f32  frame_time;
	f32  fps;
	f32  ups;
	Vec3f camera_pos;
	Vec3f camera_front;
	f32 pcf_texel_offset = 1.0f;
	i32 pcf_window_side = 3;
	std::unordered_map<EntityHandle, std::string> entities_map;
	EntityHandle selected_entity_handle = -1;

	u64 performance_regions[PerformanceRegion_Count];

	static State &instance()
	{
		lt_local_persist State state;
		return state;
	}

private:
	State() {};
};

void draw(GLFWwindow *window, Entities &entities);
void init(GLFWwindow *window);

};

#endif // __DEBUG_GUI_HPP__
