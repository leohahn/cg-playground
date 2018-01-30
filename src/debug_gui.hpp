#ifndef __DEBUG_GUI_HPP__
#define __DEBUG_GUI_HPP__

// #include <vector>
// #include <utility>
#include <unordered_map>
#include "lt_core.hpp"
#include "lt_math.hpp"
#include "entities.hpp"

struct GLFWwindow;

namespace dgui
{

struct State
{
	bool enable_normal_mapping = true;
	bool enable_multisampling = true;
	bool draw_shadow_map = false;
	bool enable_interpolation = true;
	bool enable_tone_mapping = true;
	f32  frame_time;
	f32  fps;
	f32  ups;
	Vec3f camera_pos;
	Vec3f camera_front;
	f32 pcf_texel_offset = 1.0f;
	i32 pcf_window_side = 3;
	std::unordered_map<EntityHandle, std::string> entities_map;
	EntityHandle selected_entity_handle = -1;

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
