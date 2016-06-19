/*
 * Copyright 2011-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include <bx/uint32_t.h>
#include "common.h"
#include "bgfx_utils.h"
#include "entry/cmd.h"
#include "ocornut-imgui/imgui.h"
#include "imgui/imgui.h" 

inline void SetupImGuiStyle(bool bStyleDark_, float alpha_)
{
	ImGuiStyle& style = ImGui::GetStyle();

	// light style from Pacôme Danhiez (user itamago) https://github.com/ocornut/imgui/pull/511#issuecomment-175719267
	style.Alpha = 1.0f;
	style.FrameRounding = 3.0f;
	style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 0.94f);
	style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
	style.Colors[ImGuiCol_ComboBg] = ImVec4(0.86f, 0.86f, 0.86f, 0.99f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_Column] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
	style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	style.Colors[ImGuiCol_CloseButton] = ImVec4(0.59f, 0.59f, 0.59f, 0.50f);
	style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
	style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

	if (bStyleDark_)
	{
		for (int i = 0; i <= ImGuiCol_COUNT; i++)
		{
			ImVec4& col = style.Colors[i];
			float H, S, V;
			ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, H, S, V);

			if (S < 0.1f)
			{
				V = 1.0f - V;
			}
			ImGui::ColorConvertHSVtoRGB(H, S, V, col.x, col.y, col.z);
			if (col.w < 1.00f)
			{
				col.w *= alpha_;
			}
		}
	}
	else
	{
		for (int i = 0; i <= ImGuiCol_COUNT; i++)
		{
			ImVec4& col = style.Colors[i];
			if (col.w < 1.00f)
			{
				col.x *= alpha_;
				col.y *= alpha_;
				col.z *= alpha_;
				col.w *= alpha_;
			}
		}
	}
}

void displayMainMenu()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Left"))
		{
			if (ImGui::MenuItem("Brief", "CTRL+1")) {}
			if (ImGui::MenuItem("Medium", "CTRL+2")) {}
			if (ImGui::MenuItem("Two columns", "CTRL+3")) {}
			if (ImGui::MenuItem("Full (name)", "CTRL+4")) {}
			if (ImGui::MenuItem("Full (size, time)", "CTRL+5")) {}
			if (ImGui::MenuItem("Full (access)", "CTRL+6")) {}
			ImGui::Separator();
			if (ImGui::BeginMenu("Sort mode"))
			{
				ImGui::MenuItem("Name");
				ImGui::MenuItem("Extension");
				ImGui::MenuItem("Modif. Time");
				ImGui::MenuItem("Size");
				ImGui::MenuItem("Unsorted");
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Change source")) {}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Files"))
		{
			if (ImGui::MenuItem("User menu", "F2")) {}
			if (ImGui::MenuItem("View", "F3")) {}
			if (ImGui::MenuItem("Edit", "F4")) {}
			if (ImGui::MenuItem("Copy", "F5")) {}
			if (ImGui::MenuItem("Rename or move", "F6")) {}
			if (ImGui::MenuItem("Make directory", "F7")) {}
			if (ImGui::MenuItem("Delete", "F8")) {}
			ImGui::Separator();
			if (ImGui::MenuItem("File attributes", "CTRL+A")) {}
			if (ImGui::MenuItem("Apply command", "CTRL+G")) {}
			ImGui::Separator();
			if (ImGui::MenuItem("Select group")) {}
			if (ImGui::MenuItem("Unselect group")) {}
			if (ImGui::MenuItem("Invert selection")) {}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Commands"))
		{
			if (ImGui::MenuItem("Find file", "ALT+F7")) {}
			if (ImGui::MenuItem("History", "ALT+F8")) {}
			if (ImGui::MenuItem("Maximize window", "ALT+F9")) {}
			ImGui::Separator();
			if (ImGui::MenuItem("Panel on/off", "CTRL+O")) {}
			if (ImGui::MenuItem("Equal panels", "CTRL+=")) {}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Options"))
		{
			if (ImGui::MenuItem("Settings")) {}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Right"))
		{
			if (ImGui::MenuItem("Brief", "CTRL+1")) {}
			if (ImGui::MenuItem("Medium", "CTRL+2")) {}
			if (ImGui::MenuItem("Two columns", "CTRL+3")) {}
			if (ImGui::MenuItem("Full (name)", "CTRL+4")) {}
			if (ImGui::MenuItem("Full (size, time)", "CTRL+5")) {}
			if (ImGui::MenuItem("Full (access)", "CTRL+6")) {}
			ImGui::Separator();
			if (ImGui::BeginMenu("Sort mode"))
			{
				ImGui::MenuItem("Name");
				ImGui::MenuItem("Extension");
				ImGui::MenuItem("Modif. Time");
				ImGui::MenuItem("Size");
				ImGui::MenuItem("Unsorted");
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Change source")) {}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

class mainapp : public entry::AppI
{
public:
	void init(int _argc, char** _argv) override;
	virtual int shutdown() override;
	bool update() override;

	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_debug;
	uint32_t m_reset;
	entry::MouseState m_mouseState;
};

ENTRY_IMPLEMENT_MAIN(mainapp);

void mainapp::init(int _argc, char** _argv)
{
	Args args(_argc, _argv);

	m_width  = 1280;
	m_height = 720;
	m_debug  = BGFX_DEBUG_NONE;
	m_reset  = BGFX_RESET_NONE;

	// Disable commands from BGFX
	cmdShutdown();
	cmdInit();

	bgfx::init(args.m_type, args.m_pciId);
	bgfx::reset(m_width, m_height, m_reset);

	// Enable debug text.
	bgfx::setDebug(m_debug);

	// Set view 0 clear state.
	bgfx::setViewClear(0
			, BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH
			, 0x000000ff
			, 1.0f
			, 0
			);
	imguiCreate();
}

int mainapp::shutdown()
{
	// Cleanup.
	imguiDestroy();

	// Shutdown bgfx.
	bgfx::shutdown();

	return 0;
}

bool mainapp::update()
{
	if (!entry::processEvents(m_width, m_height, m_debug, m_reset, &m_mouseState) )
	{
		bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height) );
		bgfx::touch(0);

		imguiBeginFrame(m_mouseState.m_mx
			, m_mouseState.m_my
			, (m_mouseState.m_buttons[entry::MouseButton::Left] ? IMGUI_MBUT_LEFT : 0)
			| (m_mouseState.m_buttons[entry::MouseButton::Right] ? IMGUI_MBUT_RIGHT : 0)
			, m_mouseState.m_mz
			, m_width
			, m_height
		);

		displayMainMenu();

		imguiEndFrame();
		bgfx::frame();
		return true;
	}

	return false;
}
