#include <bx/uint32_t.h>
#include "common.h"
#include "bgfx_utils.h"
#include "entry/cmd.h"
#include <experimental/filesystem>
#include <string>       // std::string
#include <iostream>     // std::cout
#include <sstream>      // std::stringstream

namespace fs = std::experimental::filesystem;

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

#include <iostream>
void mainapp::init(int _argc, char** _argv)
{
	Args args(_argc, _argv);

	m_width  = 1280;
	m_height = 720;
	m_debug  = BGFX_DEBUG_TEXT;
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
}

int mainapp::shutdown()
{
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
		bgfx::dbgTextClear();
		int i = 0;
		for (auto& dirEntry : fs::directory_iterator(".."))
		{
			std::stringstream  ss;
			ss << dirEntry.path().filename();			
			
			if (is_directory(dirEntry.path())) {
				bgfx::dbgTextPrintf(0, i, 0x0f, "%-40s", ss.str().c_str());
				i++;
			}
		}
		i = 0;
		for (auto& dirEntry : fs::directory_iterator("."))
		{
			std::stringstream  ss;
			ss << dirEntry.path().filename();
			
			bgfx::dbgTextPrintf(40, i, 0x0f, "%-40s", ss.str().c_str());
			if (is_directory(dirEntry.path())) {
				bgfx::dbgTextPrintf(80, i, 0x03, "%10s", "<dir>");
			}
			else {
				bgfx::dbgTextPrintf(80, i, 0x0f, "%10d", fs::file_size(dirEntry.path()));
			}
			i++;
		}

		bgfx::frame();
		return true;
	}

	return false;
}
