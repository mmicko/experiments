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
