#include "debug_gui.hpp"
#include <iostream>
#include "lt_core.hpp"

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "imgui/imgui.h"
#include "imgui_impl_glfw.hpp"
#include "lt_utils.hpp"
#include <stdio.h>

using std::cout;
using std::endl;

lt_global_variable lt::Logger logger("debug_gui");

void
dgui::init(GLFWwindow *window)
{
	 ImGui_ImplGlfwGL3_Init(window, true);
	 // ImGui::StyleColorsDark();
	 ImGui::StyleColorsClassic();
}

void
dgui::draw(GLFWwindow *window, Entities &entities)
{
	auto &state = State::instance();

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
			ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize()*3);
			EntityHandle node_clicked = -1;
			for (const auto &it : state.entities_map)
			{
				const EntityHandle curr_handle = it.first;

				const ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow
					| ImGuiTreeNodeFlags_OpenOnDoubleClick
					| ((curr_handle == state.selected_entity_handle) ? ImGuiTreeNodeFlags_Selected : 0);

				// ImGui::SetNextTreeNodeOpen(curr_handle == state.selected_entity_handle);
				const bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)curr_handle, node_flags,
												   "%s", it.second.c_str());

				if (ImGui::IsItemClicked())
					node_clicked = curr_handle;
				if (node_open)
				{
					if (entities.has(curr_handle, ComponentKind_Transform))
					{
						// HACK: this code should be here just for testing.
						// directly changing the transform matrix is kinda sneaky.
						Mat4f &transform = entities.transform[curr_handle].mat;

						ImGui::Text("Position:");
						// ImGui::SameLine();
						ImGui::PushItemWidth(65);
						ImGui::DragFloat("x", &transform(0, 3), 0.05f, 0, 0, "%.3f");
						ImGui::SameLine();
						ImGui::PushItemWidth(65);
						ImGui::DragFloat("y", &transform(1, 3), 0.05f, 0, 0, "%.3f");
						ImGui::SameLine();
						ImGui::PushItemWidth(65);
						ImGui::DragFloat("z", &transform(2, 3), 0.05f, 0, 0, "%.3f");

						ImGui::Text("Scale:");
						// ImGui::SameLine();
						ImGui::PushItemWidth(65);
						ImGui::DragFloat("x##2", &transform(0, 0), 0.05f, 0, 0, "%.3f");
						ImGui::SameLine();
						ImGui::PushItemWidth(65);
						ImGui::DragFloat("y##2", &transform(1, 1), 0.05f, 0, 0, "%.3f");
						ImGui::SameLine();
						ImGui::PushItemWidth(65);
						ImGui::DragFloat("z##2", &transform(2, 2), 0.05f, 0, 0, "%.3f");

						if (entities.has(curr_handle, ComponentKind_LightEmmiter))
						{
							LightEmmiter &le = entities.light_emmiter[curr_handle];
							le.position = Vec3f(transform.col(3));
						}
					}
					ImGui::TreePop();
				}
			}
			if (node_clicked != -1)
			{
				state.selected_entity_handle = node_clicked;
			}
			ImGui::PopStyleVar();
		}
		ImGui::End();
	}

	i32 display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	ImGui::Render();
}
