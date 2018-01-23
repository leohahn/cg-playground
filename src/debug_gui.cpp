#include "debug_gui.hpp"
#include <iostream>
#include "lt_core.hpp"

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "imgui/imgui.h"
#include "imgui_impl_glfw.hpp"
#include <stdio.h>

using std::cout;
using std::endl;

void
debug_gui_init(GLFWwindow *window)
{
	 ImGui_ImplGlfwGL3_Init(window, true);
	 // ImGui::StyleColorsDark();
	 ImGui::StyleColorsClassic();
}

void
debug_gui_draw(GLFWwindow *window)
{
	auto &state = DebugGuiState::instance();

	ImGui_ImplGlfwGL3_NewFrame();

	if (ImGui::Begin("Rendering Options", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse))
	{
		ImGui::Checkbox("Normal mapping", &state.enable_normal_mapping);
		ImGui::Checkbox("Multisampling", &state.enable_multisampling);
		ImGui::Checkbox("Draw shadow map", &state.draw_shadow_map);
		ImGui::Text("Position: x = %.2f, y = %.2f, z = %.2f",
					state.camera_pos.x, state.camera_pos.y, state.camera_pos.z);
		ImGui::Text("Front: x = %.2f, y = %.2f, z = %.2f",
					state.camera_front.x, state.camera_front.y, state.camera_front.z);
		ImGui::Text("Frame time: %.2f ms/frame", state.frame_time * 1000);
		ImGui::Text("FPS: %.2f", state.fps);
		ImGui::End();
	}

	ImGui::SetNextWindowSizeConstraints(ImVec2(400, 100), ImVec2(400, 500));
	if (ImGui::Begin("Options", nullptr, ImGuiWindowFlags_NoTitleBar))
	{
		if (ImGui::CollapsingHeader("Shadows"))
		{
			ImGui::DragFloat("PCF texel offset", &state.pcf_texel_offset, 0.05f, 0.0f, 30.f, "%.2f");
			ImGui::DragInt("PCF window side", &state.pcf_window_side, 2, 1, 21);
		}
		if (ImGui::CollapsingHeader("Entities"))
		{
			for (const auto &it : state.entity_names_and_ids)
			{
				// TODO: Make the entities selectable
				ImGui::Selectable(it.first.c_str());
			}
		}
		ImGui::End();
	}

	// ImGui::ShowDemoWindow();

	i32 display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	ImGui::Render();
}
