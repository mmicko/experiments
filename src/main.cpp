#include <bx/uint32_t.h>
#include "common.h"
#include "bgfx_utils.h"
#include "entry/cmd.h"
#include "imgui/imgui.h" 
#include <math.h> 

struct Node
{
	int     ID;
	char    Name[32];
	ImVec2  Pos, Size;
	float   Value;
	int     InputsCount, OutputsCount;

	Node(int id, const char* name, const ImVec2& pos, float value, int inputs_count, int outputs_count) { ID = id; strncpy(Name, name, 31); Name[31] = 0; Pos = pos; Value = value; InputsCount = inputs_count; OutputsCount = outputs_count; }

	ImVec2 GetInputSlotPos(int slot_no) const { return ImVec2(Pos.x, Pos.y + Size.y * ((float)slot_no + 1) / ((float)InputsCount + 1)); }
	ImVec2 GetOutputSlotPos(int slot_no) const { return ImVec2(Pos.x + Size.x, Pos.y + Size.y * ((float)slot_no + 1) / ((float)OutputsCount + 1)); }
};

struct NodeLink
{
	int     InputIdx, InputSlot, OutputIdx, OutputSlot;

	NodeLink(int input_idx, int input_slot, int output_idx, int output_slot) { InputIdx = input_idx; InputSlot = input_slot; OutputIdx = output_idx; OutputSlot = output_slot; }
};

class mainapp : public entry::AppI
{
public:
	void init(int _argc, char** _argv) override;
	virtual int shutdown() override;
	bool update() override;
	void ShowExampleAppCustomNodeGraph();

	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_debug;
	uint32_t m_reset;
	entry::MouseState m_mouseState;

	ImVector<Node> m_nodes;
	ImVector<NodeLink> m_links;
	ImVec2 m_scrolling;
	int m_node_selected;

};

ENTRY_IMPLEMENT_MAIN(mainapp);



// NB: You can use math functions/operators on ImVec2 if you #define IMGUI_DEFINE_MATH_OPERATORS and #include "imgui_internal.h"
// Here we only declare simple +/- operators so others don't leak into the demo code.
static inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
static inline ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }


// Really dumb data structure provided for the example.
// Note that we storing links are INDICES (not ID) to make example code shorter, obviously a bad idea for any general purpose code.
void mainapp::ShowExampleAppCustomNodeGraph()
{
	ImGui::SetNextWindowPos(ImVec2(0, 24));
	ImGui::SetNextWindowSize(ImVec2(float(m_width), float(m_height - 24)));
	if (!ImGui::Begin("Window1", nullptr, ImVec2(float(m_width), float(m_height - 24)), 1.0f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar))
	{
		ImGui::End();
		return;
	}

	// Draw a list of nodes on the left side
	bool open_context_menu = false;
	int node_hovered_in_list = -1;
	int node_hovered_in_scene = -1;
	ImGui::BeginChild("node_list", ImVec2(100, 0));
	ImGui::Text("Nodes");
	ImGui::Separator();
	for (int node_idx = 0; node_idx < m_nodes.Size; node_idx++)
	{
		Node* node = &m_nodes[node_idx];
		ImGui::PushID(node->ID);
		if (ImGui::Selectable(node->Name, node->ID == m_node_selected))
			m_node_selected = node->ID;
		if (ImGui::IsItemHovered())
		{
			node_hovered_in_list = node->ID;
			open_context_menu |= ImGui::IsMouseClicked(1);
		}
		ImGui::PopID();
	}
	ImGui::EndChild();

	ImGui::SameLine();
	ImGui::BeginGroup();

	const float NODE_SLOT_RADIUS = 4.0f;
	const ImVec2 NODE_WINDOW_PADDING(8.0f, 8.0f);

	// Create our child canvas
	ImGui::Text("Hold middle mouse button to scroll (%.2f,%.2f)", m_scrolling.x, m_scrolling.y);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, ImColor(40, 40, 40, 200));
	ImGui::BeginChild("scrolling_region", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
	ImGui::PushItemWidth(120.0f);

	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	draw_list->ChannelsSplit(2);
	ImVec2 offset = ImGui::GetCursorScreenPos() - m_scrolling;

	// Display links
	draw_list->ChannelsSetCurrent(0); // Background

	ImVec2 canvasSize = ImGui::GetWindowSize();
	ImVec2 offset2 = ImGui::GetCursorPos();
	ImVec2 win_pos = ImGui::GetCursorScreenPos();
	// Display grid
	{
		const ImU32& GRID_COLOR = ImColor(200, 200, 200, 40);
		const float& GRID_SZ = 64.0f;
		const float grid_Line_width = 0.1f;
		for (float x = fmodf(offset2.x, GRID_SZ); x < canvasSize.x; x += GRID_SZ)
			draw_list->AddLine(ImVec2(x, 0.0f) + win_pos, ImVec2(x, canvasSize.y) + win_pos, GRID_COLOR, grid_Line_width);
		for (float y = fmodf(offset2.y, GRID_SZ); y < canvasSize.y; y += GRID_SZ)
			draw_list->AddLine(ImVec2(0.0f, y) + win_pos, ImVec2(canvasSize.x, y) + win_pos, GRID_COLOR, grid_Line_width);
	}

	for (int link_idx = 0; link_idx < m_links.Size; link_idx++)
	{
		NodeLink* link = &m_links[link_idx];
		Node* node_inp = &m_nodes[link->InputIdx];
		Node* node_out = &m_nodes[link->OutputIdx];

#if 1
		// Hermite spline
		// TODO: move to ImDrawList path API
		ImVec2 p1 = offset + node_inp->GetOutputSlotPos(link->InputSlot);
		ImVec2 t1 = ImVec2(+80.0f, 0.0f);
		ImVec2 p2 = offset + node_out->GetInputSlotPos(link->OutputSlot);
		ImVec2 t2 = ImVec2(+80.0f, 0.0f);
		const int STEPS = 12;
		for (int step = 0; step <= STEPS; step++)
		{
			float t = (float)step / (float)STEPS;
			float h1 = +2 * t*t*t - 3 * t*t + 1.0f;
			float h2 = -2 * t*t*t + 3 * t*t;
			float h3 = t*t*t - 2 * t*t + t;
			float h4 = t*t*t - t*t;
			draw_list->PathLineTo(ImVec2(h1*p1.x + h2*p2.x + h3*t1.x + h4*t2.x, h1*p1.y + h2*p2.y + h3*t1.y + h4*t2.y));
		}
		draw_list->PathStroke(ImColor(200, 200, 100), false, 3.0f);
#else
		draw_list->AddLine(offset + node_inp->GetOutputSlotPos(link->InputSlot), offset + node_out->GetInputSlotPos(link->OutputSlot), ImColor(200, 200, 100), 3.0f);
#endif
	}

	// Display nodes
	for (int node_idx = 0; node_idx < m_nodes.Size; node_idx++)
	{
		Node* node = &m_nodes[node_idx];
		ImGui::PushID(node->ID);
		ImVec2 node_rect_min = offset + node->Pos;

		// Display node contents first
		draw_list->ChannelsSetCurrent(1); // Foreground
		bool old_any_active = ImGui::IsAnyItemActive();
		ImGui::SetCursorScreenPos(node_rect_min + NODE_WINDOW_PADDING);
		ImGui::BeginGroup(); // Lock horizontal position
		ImGui::Text("%s", node->Name);
		ImGui::SliderFloat("##value", &node->Value, 0.0f, 1.0f, "Alpha %.2f");
		float dummy_color[3] = { node->Pos.x / ImGui::GetWindowWidth(), node->Pos.y / ImGui::GetWindowHeight(), fmodf((float)node->ID * 0.5f, 1.0f) };
		ImGui::ColorEdit3("##color", &dummy_color[0]);
		ImGui::EndGroup();

		// Save the size of what we have emitted and weither any of the widgets are being used
		bool node_widgets_active = (!old_any_active && ImGui::IsAnyItemActive());
		node->Size = ImGui::GetItemRectSize() + NODE_WINDOW_PADDING + NODE_WINDOW_PADDING;
		ImVec2 node_rect_max = node_rect_min + node->Size;

		// Display node box
		draw_list->ChannelsSetCurrent(0); // Background
		ImGui::SetCursorScreenPos(node_rect_min);
		ImGui::InvisibleButton("node", node->Size);
		if (ImGui::IsItemHovered())
		{
			node_hovered_in_scene = node->ID;
			open_context_menu |= ImGui::IsMouseClicked(1);
		}
		bool node_moving_active = ImGui::IsItemActive();
		if (node_widgets_active || node_moving_active)
			m_node_selected = node->ID;
		if (node_moving_active && ImGui::IsMouseDragging(0))
			node->Pos = node->Pos + ImGui::GetIO().MouseDelta;

		ImU32 node_bg_color = (node_hovered_in_list == node->ID || node_hovered_in_scene == node->ID || (node_hovered_in_list == -1 && m_node_selected == node->ID)) ? ImColor(75, 75, 75) : ImColor(60, 60, 60);
		draw_list->AddRectFilled(node_rect_min, node_rect_max, node_bg_color, 4.0f);
		draw_list->AddRect(node_rect_min, node_rect_max, ImColor(100, 100, 100), 4.0f);
		for (int slot_idx = 0; slot_idx < node->InputsCount; slot_idx++)
			draw_list->AddCircleFilled(offset + node->GetInputSlotPos(slot_idx), NODE_SLOT_RADIUS, ImColor(150, 150, 150, 150));
		for (int slot_idx = 0; slot_idx < node->OutputsCount; slot_idx++)
			draw_list->AddCircleFilled(offset + node->GetOutputSlotPos(slot_idx), NODE_SLOT_RADIUS, ImColor(150, 150, 150, 150));

		ImGui::PopID();
	}
	draw_list->ChannelsMerge();

	// Open context menu
	if (!ImGui::IsAnyItemHovered() && ImGui::IsMouseHoveringWindow() && ImGui::IsMouseClicked(1))
	{
		m_node_selected = node_hovered_in_list = node_hovered_in_scene = -1;
		open_context_menu = true;
	}
	if (open_context_menu)
	{
		ImGui::OpenPopup("context_menu");
		if (node_hovered_in_list != -1)
			m_node_selected = node_hovered_in_list;
		if (node_hovered_in_scene != -1)
			m_node_selected = node_hovered_in_scene;
	}

	// Draw context menu
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
	if (ImGui::BeginPopup("context_menu"))
	{
		Node* node = m_node_selected != -1 ? &m_nodes[m_node_selected] : NULL;
		ImVec2 scene_pos = ImGui::GetMousePosOnOpeningCurrentPopup() - offset;
		if (node)
		{
			ImGui::Text("Node '%s'", node->Name);
			ImGui::Separator();
			if (ImGui::MenuItem("Rename..", NULL, false, false)) {}
			if (ImGui::MenuItem("Delete", NULL, false, false)) {}
			if (ImGui::MenuItem("Copy", NULL, false, false)) {}
		}
		else
		{
			if (ImGui::MenuItem("Add")) { m_nodes.push_back(Node(m_nodes.Size, "New node", scene_pos, 0.5f, 2, 2)); }
			if (ImGui::MenuItem("Paste", NULL, false, false)) {}
		}
		ImGui::EndPopup();
	}
	ImGui::PopStyleVar();

	// Scrolling
	if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() && ImGui::IsMouseDragging(2, 0.0f))
		m_scrolling = m_scrolling - ImGui::GetIO().MouseDelta;

	ImGui::PopItemWidth();
	ImGui::EndChild();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);
	ImGui::EndGroup();

	ImGui::End();
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
		if (ImGui::BeginMenu(ICON_FA_FILE " Files"))
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
		if (ImGui::BeginMenu(ICON_KI_GITHUB " Commands"))
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

	m_scrolling = ImVec2(0.0f, 0.0f);
	m_node_selected = -1;
	m_nodes.push_back(Node(0, "MainTex", ImVec2(40, 50), 0.5f, 1, 1));
	m_nodes.push_back(Node(1, "BumpMap", ImVec2(40, 150), 0.42f, 1, 1));
	m_nodes.push_back(Node(2, "Combine", ImVec2(270, 80), 1.0f, 2, 2));
	m_links.push_back(NodeLink(0, 0, 2, 0));
	m_links.push_back(NodeLink(1, 0, 2, 1));
}

int mainapp::shutdown()
{
	m_links.clear();
	m_nodes.clear();

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
			| (m_mouseState.m_buttons[entry::MouseButton::Middle] ? IMGUI_MBUT_MIDDLE : 0)
			, m_mouseState.m_mz
			, m_width
			, m_height
		);

		displayMainMenu();
		ShowExampleAppCustomNodeGraph();
		imguiEndFrame();
		bgfx::frame();
		return true;
	}

	return false;
}
