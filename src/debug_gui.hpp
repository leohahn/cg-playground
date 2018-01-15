#ifndef __DEBUG_GUI_HPP__
#define __DEBUG_GUI_HPP__

struct GLFWwindow;

struct DebugGuiState
{
	bool enable_normal_mapping = true;
};

void debug_gui_draw(GLFWwindow *window, DebugGuiState *state);
void debug_gui_init(GLFWwindow *window);

#endif // __DEBUG_GUI_HPP__
