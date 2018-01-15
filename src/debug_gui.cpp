#include "debug_gui.hpp"
#include <iostream>
#include "lt_core.hpp"

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "imgui/imgui.h"
#include "imgui_impl_glfw.hpp"

using std::cout;
using std::endl;


void
debug_gui_init(GLFWwindow *window)
{
	 ImGui_ImplGlfwGL3_Init(window, true);
	 ImGui::StyleColorsDark();
}

void
debug_gui_draw(GLFWwindow *window)
{
	ImGui_ImplGlfwGL3_NewFrame();

	ImGui::Begin("Rendering Options");
	ImGui::Text("my window maaaaann!");
	ImGui::End();

	i32 display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	ImGui::Render();
}
