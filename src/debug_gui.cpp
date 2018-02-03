#include "debug_gui.hpp"
#include <iostream>
#include "lt_core.hpp"

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "imgui/imgui.h"
#include "imgui_impl_glfw.hpp"
#include "lt_utils.hpp"
#include <cstdio>
#include <map>
#include <clocale>

using std::cout;
using std::endl;

lt_global_variable lt::Logger logger("debug_gui");

lt_internal void
draw_vec3(f32 &x, f32 &y, f32 &z, u32 id, i32 width = 65)
{
	lt_local_persist char x_buf[10] = {};
	lt_local_persist char y_buf[10] = {};
	lt_local_persist char z_buf[10] = {};

	std::snprintf(x_buf, 10, "x##%u", id);
	std::snprintf(y_buf, 10, "y##%u", id);
	std::snprintf(z_buf, 10, "z##%u", id);

	ImGui::PushItemWidth(width);
	ImGui::DragFloat(x_buf, &x, 0.05f, 0, 0, "%.2f");
	ImGui::SameLine();
	ImGui::PushItemWidth(width);
	ImGui::DragFloat(y_buf, &y, 0.05f, 0, 0, "%.2f");
	ImGui::SameLine();
	ImGui::PushItemWidth(width);
	ImGui::DragFloat(z_buf, &z, 0.05f, 0, 0, "%.2f");
}

void
dgui::init(GLFWwindow *window)
{
	 ImGui_ImplGlfwGL3_Init(window, true);
	 ImGui::StyleColorsDark();
	 setlocale(LC_NUMERIC, "en_US.UTF-8");
	 // ImGui::StyleColorsClassic();
}

void
dgui::draw(GLFWwindow *window, Entities &entities)
{
	auto &state = State::instance();
	i32 id = 0;

	ImGui_ImplGlfwGL3_NewFrame();

	if (ImGui::Begin("Rendering Options", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse))
	{
		ImGui::Checkbox("Normal mapping", &state.enable_normal_mapping);
		ImGui::Checkbox("Multisampling", &state.enable_multisampling);
		ImGui::Checkbox("Gamma correction", &state.enable_gamma_correction);
		ImGui::Checkbox("Tone mapping", &state.enable_tone_mapping);
		ImGui::Checkbox("Bloom", &state.enable_bloom);
		ImGui::Checkbox("Bloom filter", &state.display_bloom_filter);

		ImGui::PushItemWidth(65);
		ImGui::DragFloat("Bloom threshold", &state.bloom_threshold, 0.05f, 0, 100);

		ImGui::PushItemWidth(65);
		ImGui::DragFloat("Exposure", &state.exposure, 0.05f, 0.05f, 100);

		ImGui::Checkbox("Interpolation", &state.enable_interpolation);
		ImGui::Checkbox("Draw shadow map", &state.draw_shadow_map);
		ImGui::Text("Position: x = %.2f, y = %.2f, z = %.2f",
					state.camera_pos.x, state.camera_pos.y, state.camera_pos.z);
		ImGui::Text("Front: x = %.2f, y = %.2f, z = %.2f",
					state.camera_front.x, state.camera_front.y, state.camera_front.z);
		ImGui::Text("Frame time: %.2f ms/frame", state.frame_time);
		ImGui::Text("FPS: %.2f", state.fps);
		ImGui::Text("UPS: %.2f", state.ups);
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
						draw_vec3(transform(0, 3), transform(1, 3), transform(2, 3), id++);

						ImGui::Text("Scale:");
						draw_vec3(transform(0, 0), transform(1, 1), transform(2, 2), id++);

						if (entities.has(curr_handle, ComponentKind_LightEmmiter))
						{
							LightEmmiter &le = entities.light_emmiter[curr_handle];
							le.position = Vec3f(transform.col(3));

							ImGui::Text("Light variables:");
							ImGui::Text("Ambient:");
							draw_vec3(le.ambient.x, le.ambient.y, le.ambient.z, id++);
							ImGui::Text("Diffuse:");
							draw_vec3(le.diffuse.x, le.diffuse.y, le.diffuse.z, id++);
							ImGui::Text("Specular:");
							draw_vec3(le.specular.x, le.specular.y, le.specular.z, id++);
						}
					}
					ImGui::TreePop();
				}
			}
			if (node_clicked != -1)
			{
				state.selected_entity_handle = (node_clicked == state.selected_entity_handle) ? -1 : node_clicked;
			}
			ImGui::PopStyleVar();
		}
		ImGui::End();
	}

	if (ImGui::Begin("Performance", nullptr))
	{
		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize()*3);

		i32 node_clicked = -1;
		lt_local_persist i32 selected_node = -1;

		for (i32 i = 0; i < PerformanceRegion_Count; i++)
		{
			const u64 region_val = state.performance_regions[i];

			const ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow
				| ImGuiTreeNodeFlags_OpenOnDoubleClick
				| ((i == selected_node) ? ImGuiTreeNodeFlags_Selected : 0);

			// ImGui::SetNextTreeNodeOpen(curr_handle == state.selected_entity_handle);
			const bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags,
													 "%s: %'lu cycles", PerformanceRegionNames[i], region_val);

			if (ImGui::IsItemClicked())
				node_clicked = i;

			if (node_open)
				ImGui::TreePop();
		}
		if (node_clicked != -1)
			selected_node = (node_clicked == selected_node) ? -1 : node_clicked;

		ImGui::PopStyleVar();
		ImGui::End();
	}
	// ImGui::ShowDemoWindow();

	i32 display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	ImGui::Render();
}
