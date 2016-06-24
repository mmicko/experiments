/* 	Refactoring from https://github.com/ocornut/imgui/issues/306
	It's basically the same exact code with a few modifications (and tons of additions)
*/

#ifndef IMGUINODEGRAPHEDITOR_H_
#define IMGUINODEGRAPHEDITOR_H_

namespace ImGui {
	enum FieldType {
		FT_INT = 0,
		FT_UNSIGNED,
		FT_FLOAT,
		FT_DOUBLE,
		//--------------- End types that support 1 to 4 array components ----------
		FT_STRING,      // an arbitrary-length string (or a char blob that can be used as custom type)
		FT_ENUM,        // serialized/deserialized as FT_INT
		FT_BOOL,
		FT_COLOR,       // serialized/deserialized as FT_FLOAT (with 3 or 4 components)
		FT_TEXTLINE,    // a (series of) text line(s) (separated by '\n') that are fed one at a time in the Deserializer callback
		FT_CUSTOM,      // a custom type that is served like FT_TEXTLINE (=one line at a time).
		FT_COUNT
	};
}

namespace ImGui {

	class FieldInfo {
	protected:
#       ifndef IMGUIFIELDINFO_MAX_LABEL_LENGTH
#       define IMGUIFIELDINFO_MAX_LABEL_LENGTH 32
#       endif //IMGUIFIELDINFO_MAX_LABEL_LENGTH
#       ifndef IMGUIFIELDINFO_MAX_TOOLTIP_LENGTH
#       define IMGUIFIELDINFO_MAX_TOOLTIP_LENGTH 64
#       endif //IMGUIFIELDINFO_MAX_TOOLTIP_LENGTH

	public:
		int type;                   // usually one of the ImGui::FieldTypes in imguihelper.h
		void* pdata;                // ptr to a variable of type "type" (or to an array of "types")
		char label[IMGUIFIELDINFO_MAX_LABEL_LENGTH];
		char tooltip[IMGUIFIELDINFO_MAX_TOOLTIP_LENGTH];
		// in case of FT_STRING max number of characters, in case of FT_FLOAT or FT_DOUBLE the number of decimals to be displayed (experiment for other types and see)
		int precision;
		// used only for FT_INT, FT_UNSIGNED, FT_FLOAT, FT_DOUBLE
		int numArrayElements;       // up to 4
		double minValue, maxValue;
		bool needsRadiansToDegs;    // optional for FT_FLOAT and FT_DOUBLE only
		// used only for FT_ENUM (internally it uses FT_INT, pdata must point to an int):
		int numEnumElements;
		typedef bool(*TextFromEnumDelegate)(void*, int, const char**); // userData is the first param
		TextFromEnumDelegate  textFromEnumFunctionPointer;  // used only when type==FT_ENUM, otherwise set it to NULL. The method is used to convert an int to a char*.
		void* userData;          // passed to textFromEnumFunctionPointer when type==FT_ENUM (useful if you want to share the same TextFromEnumDelegate for multiple enums). Otherwise set it to NULL or use it as you like.
		typedef void(*EditedFieldDelegate)(FieldInfo& field, int widgetIndex);  // widgetIndex is always zero
		EditedFieldDelegate editedFieldDelegate;
		// used only for FT_CUSTOM
		typedef bool(*RenderFieldDelegate)(FieldInfo& field);
		RenderFieldDelegate renderFieldDelegate;
		typedef bool(*CopyFieldDelegate)(FieldInfo& fdst, const FieldInfo& fsrc);
		CopyFieldDelegate copyFieldDelegate;

		bool render(int nodeWidth);

	protected:
		FieldInfo() {}
		void init(int _type = FT_INT, void* _pdata = NULL, const char* _label = NULL, const char* _tooltip = NULL,
			int _precision = 0, int _numArrayElements = 0, double _lowerLimit = 0, double _upperLimit = 1, bool _needsRadiansToDegs = false,
			int _numEnumElements = 0, TextFromEnumDelegate _textFromEnumFunctionPointer = NULL, void* _userData = NULL,
			RenderFieldDelegate _renderFieldDelegate = NULL, EditedFieldDelegate _editedFieldDelegate = NULL)
		{
			label[0] = '\0'; if (_label) { strncpy(label, _label, IMGUIFIELDINFO_MAX_LABEL_LENGTH); label[IMGUIFIELDINFO_MAX_LABEL_LENGTH - 1] = '\0'; }
			tooltip[0] = '\0'; if (_tooltip) { strncpy(tooltip, _tooltip, IMGUIFIELDINFO_MAX_TOOLTIP_LENGTH); tooltip[IMGUIFIELDINFO_MAX_TOOLTIP_LENGTH - 1] = '\0'; }
			type = _type;
			pdata = _pdata;
			precision = _precision;
			numArrayElements = _numArrayElements;
			minValue = _lowerLimit;
			maxValue = _upperLimit;
			needsRadiansToDegs = _needsRadiansToDegs;
			numEnumElements = _numEnumElements;
			textFromEnumFunctionPointer = _textFromEnumFunctionPointer;
			userData = _userData;
			renderFieldDelegate = _renderFieldDelegate;
			editedFieldDelegate = _editedFieldDelegate;
		}

		inline bool isCompatibleWith(const FieldInfo& f) const {
			return (type == f.type &&
				numArrayElements == f.numArrayElements);   // Warning: we can't use numArrayElements for other purposes when it's not used....
		}
		//bool copyFrom(const FieldInfo& f);
		bool copyPDataValueFrom(const FieldInfo& f);
		friend class FieldInfoVector;
		friend class Node;
	};
	class FieldInfoVector : public ImVector < FieldInfo > {
	public:
		// Warning: returned reference might not stay valid for long in these methods
		FieldInfo& addField(int* pdata, int numArrayElements = 1, const char* label = NULL, const char* tooltip = NULL, int precision = 0, int lowerLimit = 0, int upperLimit = 100, void* userData = NULL);
		FieldInfo& addField(unsigned* pdata, int numArrayElements = 1, const char* label = NULL, const char* tooltip = NULL, int precision = 0, unsigned lowerLimit = 0, unsigned upperLimit = 100, void* userData = NULL);
		FieldInfo& addField(float* pdata, int numArrayElements = 1, const char* label = NULL, const char* tooltip = NULL, int precision = 3, float lowerLimit = 0, float upperLimit = 1, void* userData = NULL, bool needsRadiansToDegs = false);
		FieldInfo& addField(double* pdata, int numArrayElements = 1, const char* label = NULL, const char* tooltip = NULL, int precision = 3, double lowerLimit = 0, double upperLimit = 100, void* userData = NULL, bool needsRadiansToDegs = false);

		FieldInfo& addFieldEnum(int* pdata, int numEnumElements, FieldInfo::TextFromEnumDelegate textFromEnumFunctionPtr, const char* label = NULL, const char* tooltip = NULL, void* userData = NULL);
		FieldInfo& addField(bool* pdata, const char* label = NULL, const char* tooltip = NULL, void* userData = NULL);
		FieldInfo& addFieldColor(float* pdata, bool useAlpha = true, const char* label = NULL, const char* tooltip = NULL, int precision = 3, void* userData = NULL);
		FieldInfo& addFieldTextEdit(char* pdata, int textLength = 0, const char* label = NULL, const char* tooltip = NULL, int flags = ImGuiInputTextFlags_EnterReturnsTrue, void* userData = NULL) {
			return addField(pdata, textLength, label, tooltip, flags, false, -1.f, userData);
		}
		FieldInfo& addFieldTextEditMultiline(char* pdata, int textLength = 0, const char* label = NULL, const char* tooltip = NULL, int flags = ImGuiInputTextFlags_EnterReturnsTrue, float optionalHeight = -1.f, void* userData = NULL) {
			return addField(pdata, textLength, label, tooltip, flags, true, optionalHeight, userData);
		}
		FieldInfo& addFieldTextWrapped(char* pdata, int textLength = 0, const char* label = NULL, const char* tooltip = NULL, void* userData = NULL) {
			return addField(pdata, textLength, label, tooltip, -1, true, -1.f, userData);
		}
		FieldInfo& addFieldTextEditAndBrowseButton(char* pdata, int textLength = 0, const char* label = NULL, const char* tooltip = NULL, int flags = ImGuiInputTextFlags_EnterReturnsTrue, void* userData = NULL) {
			return addField(pdata, textLength, label, tooltip, flags, false, -1.f, userData, true);
		}

		FieldInfo& addFieldCustom(FieldInfo::RenderFieldDelegate renderFieldDelegate, FieldInfo::CopyFieldDelegate copyFieldDelegate, void* userData
		);

		void copyPDataValuesFrom(const FieldInfoVector& o) {
			for (int i = 0, isz = o.size() < size() ? o.size() : size(); i < isz; i++) {
				const FieldInfo& of = o[i];
				FieldInfo& f = (*this)[i];
				f.copyPDataValueFrom(of);
			}
		}

	protected:
		FieldInfo& addField(char* pdata, int textLength = 0, const char* label = NULL, const char* tooltip = NULL, int flags = ImGuiInputTextFlags_EnterReturnsTrue, bool multiline = false, float optionalHeight = -1.f, void* userData = NULL, bool isSingleEditWithBrowseButton = false);
		friend class Node;
	};
	//--------------------------------------------------------------------------------------------

	class Node
	{
	public:
		virtual ~Node() {}
		mutable void* user_ptr;
		mutable int userID;
		inline const char* getName() const { return Name; }
		inline int getType() const { return typeID; }
		inline int getNumInputSlots() const { return InputsCount; }
		inline int getNumOutputSlots() const { return OutputsCount; }
		inline void setOpen(bool flag) { isOpen = flag; }

	protected:
		FieldInfoVector fields; // I guess you can just skip these at all and implement virtual methods... but it was supposed to be useful...
		// virtual methods
		virtual bool render(float nodeWidth) // should return "true" if the node has been edited and its values modified (to fire "edited callbacks")
		{
			bool nodeEdited = false;
			for (int i = 0, isz = fields.size(); i < isz; i++) {
				FieldInfo& f = fields[i];
				nodeEdited |= f.render((int)nodeWidth);
			}
			return nodeEdited;
		}
		virtual const char* getTooltip() const { return NULL; }
		virtual const char* getInfo() const { return NULL; }
		virtual void onEdited() {}  // called (a few seconds) after the node has been edited
		virtual void onCopied() {}  // called after the node fileds has been copied from another node
		virtual void onLoaded() {}  // called after the node has been loaded (=deserialized from file)
		virtual bool canBeCopied() const { return true; }

		// some constants
#   ifndef IMGUINODE_MAX_NAME_LENGTH
#   define IMGUINODE_MAX_NAME_LENGTH 32
#   endif //IMGUINODE_MAX_NAME_LENGTH
#   ifndef IMGUINODE_MAX_INPUT_SLOTS
#   define IMGUINODE_MAX_INPUT_SLOTS 8
#   endif //IMGUINODE_MAX_INPUT_SLOTS
#   ifndef IMGUINODE_MAX_OUTPUT_SLOTS
#   define IMGUINODE_MAX_OUTPUT_SLOTS 8
#   endif //IMGUINODE_MAX_OUTPUT_SLOTS
#   ifndef IMGUINODE_MAX_SLOT_NAME_LENGTH
#   define IMGUINODE_MAX_SLOT_NAME_LENGTH 12
#   endif //IMGUINODE_MAX_SLOT_NAME_LENGTH
	// ---------------

		ImVec2  Pos, Size;
		char    Name[IMGUINODE_MAX_NAME_LENGTH];
		int     InputsCount, OutputsCount;
		char    InputNames[IMGUINODE_MAX_INPUT_SLOTS][IMGUINODE_MAX_SLOT_NAME_LENGTH];
		char    OutputNames[IMGUINODE_MAX_OUTPUT_SLOTS][IMGUINODE_MAX_SLOT_NAME_LENGTH];
		mutable float startEditingTime; // used for Node Editing Callbacks
		mutable bool isOpen;
		int typeID;
		float baseWidthOverride;
		bool mustOverrideName, mustOverrideInputSlots, mustOverrideOutputSlots;

		Node() : Pos(0, 0), Size(0, 0), baseWidthOverride(-1), mustOverrideName(false), mustOverrideInputSlots(false), mustOverrideOutputSlots(false) {}
		void init(const char* name, const ImVec2& pos, const char* inputSlotNamesSeparatedBySemicolons = NULL, const char* outputSlotNamesSeparatedBySemicolons = NULL, int _nodeTypeID = 0/*,float currentWindowFontScale=-1.f*/);

		inline ImVec2 GetInputSlotPos(int slot_no, float currentFontWindowScale = 1.f) const { return ImVec2(Pos.x*currentFontWindowScale, Pos.y*currentFontWindowScale + Size.y * ((float)slot_no + 1) / ((float)InputsCount + 1)); }
		inline ImVec2 GetOutputSlotPos(int slot_no, float currentFontWindowScale = 1.f) const { return ImVec2(Pos.x*currentFontWindowScale + Size.x, Pos.y*currentFontWindowScale + Size.y * ((float)slot_no + 1) / ((float)OutputsCount + 1)); }
		inline const ImVec2 GetPos(float currentFontWindowScale = 1.f) const { return ImVec2(Pos.x*currentFontWindowScale, Pos.y*currentFontWindowScale); }

		friend struct NodeLink;
		friend struct NodeGraphEditor;

		// Helper static methods to simplify code of the derived classes
		// casts:
		template <typename T> inline static T* Cast(Node* n, int TYPE) { return ((n && n->getType() == TYPE) ? static_cast<T*>(n) : NULL); }
		template <typename T> inline static const T* Cast(const Node* n, int TYPE) { return ((n && n->getType() == TYPE) ? static_cast<const T*>(n) : NULL); }


	};

	struct NodeLink
	{
		Node*  InputNode;   int InputSlot;
		Node*  OutputNode;  int OutputSlot;

		NodeLink(Node* input_node, int input_slot, Node* output_node, int output_slot) {
			InputNode = input_node; InputSlot = input_slot;
			OutputNode = output_node; OutputSlot = output_slot;
		}

		friend struct NodeGraphEditor;
	};

	struct NodeGraphEditor {
	public:
		typedef Node* (*NodeFactoryDelegate)(int nodeType, const ImVec2& pos);

	protected:
		ImVector<Node*> nodes;          // used as a garbage collector too
		ImVector<NodeLink> links;
		ImVec2 scrolling;
		Node *selectedNode;
		Node *sourceCopyNode;           // this is owned by the NodeGraphEditor
		bool inited;
		bool allowOnlyOneLinkPerInputSlot;  // multiple links can still be connected to single output slots
		bool avoidCircularLinkLoopsInOut;   // however multiple paths from a node to another are still allowed (only in-out circuits are prevented)
		//bool isAContextMenuOpen;            // to fix a bug
		float oldFontWindowScale;           // to fix zooming (CTRL+mouseWheel)    
		float maxConnectorNameWidth;        //used to enlarge node culling space to include connector names

		// Node types here are supposed to be zero-based and contiguous
		const char** pNodeTypeNames; // NOT OWNED! -> Must point to a static reference
		int numNodeTypeNames;
		NodeFactoryDelegate nodeFactoryFunctionPtr;
		ImVector<int> availableNodeTypes;   // These will appear in the "add node menu"

		enum NodeState { NS_ADDED, NS_DELETED, NS_EDITED };
		typedef void(*NodeCallback)(Node*& node, NodeState state, NodeGraphEditor& editor);
		enum LinkState { LS_ADDED, LS_DELETED };
		typedef void(*LinkCallback)(const NodeLink& link, LinkState state, NodeGraphEditor& editor);
		LinkCallback linkCallback;// called after a link is added and before it's deleted
		NodeCallback nodeCallback;// called after a node is added, after it's edited and before it's deleted
		float nodeEditedTimeThreshold; // time in seconds that must elapse after the last "editing touch" before the NS_EDITED callback is called

	public:
		struct Style {
			ImVec4 color_background;
			ImU32 color_grid;
			float grid_line_width, grid_size;
			ImU32 color_node;
			ImU32 color_node_frame;
			ImU32 color_node_selected;
			ImU32 color_node_hovered;
			float node_rounding;
			ImVec2 node_window_padding;
			ImU32 color_node_input_slots;
			ImU32 color_node_output_slots;
			float node_slots_radius;
			ImU32 color_link;
			float link_line_width;
			float link_control_point_distance;
			int link_num_segments;  // in AddBezierCurve(...)
			ImVec4 color_node_title;
			ImVec4 color_node_input_slots_names;
			ImVec4 color_node_output_slots_names;
			Style() {
				color_background = ImColor(60, 60, 70, 200);
				color_grid = ImColor(200, 200, 200, 40);
				grid_line_width = 1.f;
				grid_size = 64.f;

				color_node = ImColor(60, 60, 60);
				color_node_frame = ImColor(100, 100, 100);
				color_node_selected = ImColor(75, 75, 85);
				color_node_hovered = ImColor(85, 85, 85);
				node_rounding = 4.f;
				node_window_padding = ImVec2(8.f, 8.f);

				color_node_input_slots = ImColor(150, 150, 150, 150);
				color_node_output_slots = ImColor(150, 150, 150, 150);
				node_slots_radius = 5.f;

				color_link = ImColor(200, 200, 100);
				link_line_width = 3.f;
				link_control_point_distance = 50.f;
				link_num_segments = 0;

				color_node_title = ImGui::GetStyle().Colors[ImGuiCol_Text];
				color_node_input_slots_names = ImGui::GetStyle().Colors[ImGuiCol_Text]; color_node_input_slots_names.w = 0.75f;
				color_node_output_slots_names = ImGui::GetStyle().Colors[ImGuiCol_Text]; color_node_output_slots_names.w = 0.75f;
			}

			static bool Edit(Style& style);
			static void Reset(Style& style) { style = Style(); }
		};
		bool show_grid;
		bool show_connection_names;
		bool show_left_pane;
		bool show_top_pane;
		bool show_node_copy_paste_buttons;
		static bool UseSlidersInsteadOfDragControls;
		mutable void* user_ptr;
		static Style& GetStyle() { return style; }
		mutable ImGuiColorEditMode colorEditMode;
		float nodesBaseWidth;

		NodeGraphEditor(bool show_grid_ = true, bool show_connection_names_ = true, bool _allowOnlyOneLinkPerInputSlot = true, bool _avoidCircularLinkLoopsInOut = true, bool init_in_ctr = false) {
			scrolling = ImVec2(0.0f, 0.0f);
			show_grid = show_grid_;
			show_connection_names = show_connection_names_;
			selectedNode = dragNode.node = sourceCopyNode = NULL;
			allowOnlyOneLinkPerInputSlot = _allowOnlyOneLinkPerInputSlot;
			avoidCircularLinkLoopsInOut = _avoidCircularLinkLoopsInOut;
			nodeCallback = NULL; linkCallback = NULL; nodeEditedTimeThreshold = 1.5f;
			user_ptr = NULL;
			show_left_pane = true;
			show_top_pane = true;
			show_node_copy_paste_buttons = true;
			pNodeTypeNames = NULL;
			numNodeTypeNames = 0;
			nodeFactoryFunctionPtr = NULL;
			inited = init_in_ctr;
			colorEditMode = ImGuiColorEditMode_RGB;
			oldFontWindowScale = 0.f;
			nodesBaseWidth = 120.f;
			maxConnectorNameWidth = 0;
		}
		virtual ~NodeGraphEditor() {
			clear();
		}
		void clear() {
			if (linkCallback) {
				for (int i = links.size() - 1; i >= 0; i--) {
					const NodeLink& link = links[i];
					linkCallback(link, LS_DELETED, *this);
				}
			}
			links.clear();
			for (int i = nodes.size() - 1; i >= 0; i--) {
				Node*& node = nodes[i];
				if (node) {
					if (nodeCallback) nodeCallback(node, NS_DELETED, *this);
					node->~Node();              // ImVector does not call it
					ImGui::MemFree(node);       // items MUST be allocated by the user using ImGui::MemAlloc(...)
					node = NULL;
				}
			}
			nodes.clear();
			scrolling = ImVec2(0, 0);
			if (sourceCopyNode) {
				sourceCopyNode->~Node();              // ImVector does not call it
				ImGui::MemFree(sourceCopyNode);       // items MUST be allocated by the user using ImGui::MemAlloc(...)
				sourceCopyNode = NULL;
			}
			selectedNode = dragNode.node = NULL;
			oldFontWindowScale = 0.f;
		}

		bool isInited() const { return !inited; }

		bool isEmpty() const { return nodes.size() == 0; }

		// nodeTypeNames must point to a block of static memory: it's not owned, nor copied. pOptionalNodeTypesToUse is copied.
		void registerNodeTypes(const char* nodeTypeNames[], int numNodeTypeNames, NodeFactoryDelegate _nodeFactoryFunctionPtr, const int* pOptionalNodeTypesToUse = NULL, int numNodeTypesToUse = -1);
		inline int getNumAvailableNodeTypes() const { return availableNodeTypes.size(); }

		Node* addNode(int nodeType, const ImVec2& Pos = ImVec2(0, 0)) {
			if (!nodeFactoryFunctionPtr) return NULL;
			//const float fontWindowScale = oldFontWindowScale==0 ? 1.f : oldFontWindowScale;
			// Just to say that I've tried all the combinations of Pos, fontWindowScale and scrolling, at it just does not work to adjust Pos to zooming/scrolling.
			return addNode(nodeFactoryFunctionPtr(nodeType, Pos));
		}
		bool deleteNode(Node* node) {
			if (node == selectedNode)  selectedNode = NULL;
			if (node == dragNode.node) dragNode.node = NULL;
			for (int i = 0; i < nodes.size(); i++) {
				Node*& n = nodes[i];
				if (n == node) {
					removeAnyLinkFromNode(n);
					if (nodeCallback) nodeCallback(n, NS_DELETED, *this);
					n->~Node();              // ImVector does not call it
					ImGui::MemFree(n);       // items MUST be allocated by the user using ImGui::MemAlloc(...)
					if (i + 1 < nodes.size()) n = nodes[nodes.size() - 1];    // swap with the last node
					nodes.resize(nodes.size() - 1);
					return true;
				}
			}
			return false;
		}
		bool addLink(Node* inputNode, int input_slot, Node* outputNode, int output_slot, bool checkIfAlreadyPresent = false) {
			bool insert = true;
			if (checkIfAlreadyPresent) insert = !isLinkPresent(inputNode, input_slot, outputNode, output_slot);
			if (insert) {
				links.push_back(NodeLink(inputNode, input_slot, outputNode, output_slot));
				if (linkCallback) linkCallback(links[links.size() - 1], LS_ADDED, *this);
			}
			return insert;
		}
		bool removeLink(Node* inputNode, int input_slot, Node* outputNode, int output_slot) {
			int link_idx = -1;
			bool ok = isLinkPresent(inputNode, input_slot, outputNode, output_slot, &link_idx);
			if (ok) ok = removeLinkAt(link_idx);
			return ok;
		}
		void removeAnyLinkFromNode(Node* node, bool removeInputLinks = true, bool removeOutputLinks = true);
		bool isLinkPresent(Node* inputNode, int input_slot, Node* outputNode, int output_slot, int* pOptionalIndexInLinkArrayOut = NULL) const;

		// To be called INSIDE a window
		void render();

		// Optional helper methods:
		Node* getSelectedNode() { return selectedNode; }
		const Node* getSelectedNode() const { return selectedNode; }
		const char* getSelectedNodeInfo() const { return selectedNode->getInfo(); }

		void getOutputNodesForNodeAndSlot(const Node* node, int output_slot, ImVector<Node*>& returnValueOut, ImVector<int>* pOptionalReturnValueInputSlotOut = NULL) const;
		void getInputNodesForNodeAndSlot(const Node* node, int input_slot, ImVector<Node*>& returnValueOut, ImVector<int>* pOptionalReturnValueOutputSlotOut = NULL) const;
		// if allowOnlyOneLinkPerInputSlot == true:
		Node* getInputNodeForNodeAndSlot(const Node* node, int input_slot, int* pOptionalReturnValueOutputSlotOut = NULL) const;
		bool isNodeReachableFrom(const Node *node1, int slot1, bool goBackward, const Node* nodeToFind, int* pOptionalNodeToFindSlotOut = NULL) const;
		bool isNodeReachableFrom(const Node *node1, bool goBackward, const Node* nodeToFind, int* pOptionalNode1SlotOut = NULL, int* pOptionalNodeToFindSlotOut = NULL) const;
		bool hasLinks(Node* node) const;
		int getAllNodesOfType(int typeID, ImVector<Node*>* pNodesOut = NULL, bool clearNodesOutBeforeUsage = true);
		int getAllNodesOfType(int typeID, ImVector<const Node*>* pNodesOut = NULL, bool clearNodesOutBeforeUsage = true) const;

		// It should be better not to add/delete node/links in the callbacks... (but all is untested here)
		void setNodeCallback(NodeCallback cb) { nodeCallback = cb; }
		void setLinkCallback(LinkCallback cb) { linkCallback = cb; }
		void setNodeEditedCallbackTimeThreshold(int seconds) { nodeEditedTimeThreshold = (float)seconds; }
		// I suggest we don not use these 3; however we can:
		bool overrideNodeName(Node* node, const char* newName);
		bool overrideNodeInputSlots(Node* node, const char* slotNamesSeparatedBySemicolons);
		bool overrideNodeOutputSlots(Node* node, const char* slotNamesSeparatedBySemicolons);


	protected:

		struct DragNode {
			ImVec2 pos;
			Node* node; int inputSlotIdx, outputSlotIdx;
			DragNode() : node(NULL), inputSlotIdx(-1), outputSlotIdx(-1) {}
			bool isValid() const { return node && (inputSlotIdx >= 0 || outputSlotIdx >= 0); }
			void reset() { *this = DragNode(); }
		};
		DragNode dragNode;

		// BEST PRACTICE: always call this method like: Node* node = addNode(ExampleNode::Create(...));
		Node* addNode(Node* justCreatedNode) {
			if (justCreatedNode) {
				nodes.push_back(justCreatedNode);
				if (nodeCallback) nodeCallback(nodes[nodes.size() - 1], NS_ADDED, *this);
			}
			return justCreatedNode;
		}
		void copyNode(Node* n);
		bool removeLinkAt(int link_idx);
		inline int getNodeIndex(const Node* node) {
			for (int i = 0; i < nodes.size(); i++) {
				const Node* n = nodes[i];
				if (n == node) return i;
			}
			return -1;
		}
		static Style style;
	};


	void TestNodeGraphEditor();


}	// namespace ImGui



#endif //IMGUINODEGRAPHEDITOR_H_

