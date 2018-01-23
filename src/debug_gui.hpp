#ifndef __DEBUG_GUI_HPP__
#define __DEBUG_GUI_HPP__

#include <vector>
#include <utility>
#include "lt_core.hpp"
#include "lt_math.hpp"

struct GLFWwindow;

struct DebugGuiState
{
	bool enable_normal_mapping = true;
	bool enable_multisampling = true;
	bool draw_shadow_map = false;
	f32  frame_time;
	f32  fps;
	Vec3f camera_pos;
	Vec3f camera_front;
	f32 pcf_texel_offset = 1.0f;
	i32 pcf_window_side = 3;
	std::vector<std::pair<std::string, i64>> entity_names_and_ids;

	static DebugGuiState &instance()
	{
		lt_local_persist DebugGuiState state;
		return state;
	}

private:
	DebugGuiState() {};
};

void debug_gui_draw(GLFWwindow *window);
void debug_gui_init(GLFWwindow *window);

#endif // __DEBUG_GUI_HPP__
