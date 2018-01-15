#ifndef __DEBUG_GUI_HPP__
#define __DEBUG_GUI_HPP__

#include "lt_core.hpp"

struct GLFWwindow;

struct DebugGuiState
{
	bool enable_normal_mapping = true;
	f32  frame_time;
	f32  fps;
};

void debug_gui_draw(GLFWwindow *window, DebugGuiState *state);
void debug_gui_init(GLFWwindow *window);

#endif // __DEBUG_GUI_HPP__
