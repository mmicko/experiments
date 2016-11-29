#include <bx/uint32_t.h>
#include "common.h"
#include "bgfx_utils.h"
#include "entry/cmd.h"
#include "entry/input.h"
#include <dirent.h> 
#include <vector>
#include <string>

#if defined(__GNUC__)
#define ATTR_PRINTF(x,y)        __attribute__((format(printf, x, y)))
#else
#define ATTR_PRINTF(x,y)
#endif

class mainapp : public entry::AppI
{
public:
	void init(int _argc, char** _argv) override;
	virtual int shutdown() override;
	bool update() override;	

	void updateFolder();
	void clearBuffer();
	void bufferPrintf(uint16_t x, uint16_t y, uint8_t attrib, char const *format, ...) ATTR_PRINTF(2, 3);
	void bufferAttrib(uint16_t x, uint16_t y, uint8_t attrib, uint16_t w);
	void checkKeyPress();
	void keypressed(entry::Key::Enum key);
private:
	std::vector<std::string> m_filelist;
	uint16_t m_selected;
	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_debug;
	uint32_t m_reset;
	entry::MouseState m_mouseState;

	uint16_t m_text_width;
	uint16_t m_text_height;
	std::vector<uint8_t> m_buffer;
	char m_temp_buffer[8192];

	bool m_keyState[entry::Key::Count];
	uint8_t m_keyNumPress[entry::Key::Count];
};

ENTRY_IMPLEMENT_MAIN(mainapp);

void mainapp::init(int _argc, char** _argv)
{
	Args args(_argc, _argv);

	m_width  = 1280;
	m_height = 720;
	m_debug  = BGFX_DEBUG_TEXT;
	m_reset  = BGFX_RESET_NONE;
	m_selected = 0;

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

	for (auto& item : m_keyState)
		item = false;
	for (auto& item : m_keyNumPress)
		item = 0;
	updateFolder();
}

int mainapp::shutdown()
{
	// Shutdown bgfx.
	bgfx::shutdown();
	return 0;
}


void mainapp::bufferPrintf(uint16_t x, uint16_t y, uint8_t attrib, const char *format, ...)
{
	va_list args;
	/* format the message */
	va_start(args, format);
	uint32_t num = vsnprintf(m_temp_buffer, sizeof(m_temp_buffer), format, args);
	va_end(args);

	if (x < m_text_width && y < m_text_height)
	{
		uint32_t pos = (y*m_text_width + x) * 2;
		for (uint32_t ii = 0, xx = x; ii < num && xx < m_text_width; ++ii, ++xx)
		{
			m_buffer[pos++] = m_temp_buffer[ii];
			m_buffer[pos++] = attrib;
		}
	}
}


void mainapp::bufferAttrib(uint16_t x, uint16_t y, uint8_t attrib, uint16_t w)
{
	if (x < m_text_width && y < m_text_height)
	{
		uint32_t pos = (y*m_text_width + x) * 2  + 1;
		for (uint32_t ii = 0, xx = x; ii < w && xx < m_text_width; ++ii, ++xx)
		{
			m_buffer[pos++] = attrib;
			pos++;
		}
	}
}

void mainapp::updateFolder()
{
	std::string path = ".";
	m_filelist.clear();
	DIR* dir = opendir(".");

	if (NULL == dir)
	{
		path = ".";
	}
	int i = 0;
	dir = opendir(path.c_str());
	if (NULL != dir)
	{
		for (dirent* item = readdir(dir); NULL != item; item = readdir(dir))
		{
			//if (0 == (item->d_type & DT_DIR))
			{
				m_filelist.push_back(std::string(item->d_name));
				i++;
			}
		}

		closedir(dir);
	}
}

void mainapp::clearBuffer()
{
	const bgfx::Stats* stats = bgfx::getStats();
	if (m_text_width != stats->textWidth || m_text_height != stats->textHeight)
	{
		m_text_width = stats->textWidth;
		m_text_height = stats->textHeight;
		m_buffer.resize(m_text_width*m_text_height * 2);		
	}
	std::fill(m_buffer.begin(), m_buffer.end(), 0x00);
}

void mainapp::keypressed(entry::Key::Enum key)
{
	if (key == entry::Key::Up) {
		if (m_selected > 0) m_selected--;
	}
	if (key == entry::Key::Down) {
		if (m_selected < m_filelist.size()-1) m_selected++;
	}
}

void mainapp::checkKeyPress()
{
	for (int32_t ii = 0; ii < (int32_t)entry::Key::Count; ++ii)
	{
		bool oldpressed = m_keyState[ii];
		bool pressed = inputGetKeyState(entry::Key::Enum(ii));
		if (oldpressed == false && pressed == true) { // Key pressed
			m_keyNumPress[ii] = 0;
			m_keyState[ii] = true;
		}
		if (oldpressed == true && pressed == true) { // Key being pressed
			m_keyNumPress[ii]++;
			if (m_keyNumPress[ii] == 0) { // 256 means holding too long
				m_keyState[ii] = false;
				keypressed(entry::Key::Enum(ii));
			}
		}
		if (oldpressed == true && pressed == false) { // Key released
			m_keyNumPress[ii] = 0;
			m_keyState[ii] = false;
			keypressed(entry::Key::Enum(ii));
		}
	}
}

bool mainapp::update()
{
	checkKeyPress();
	if (!entry::processEvents(m_width, m_height, m_debug, m_reset, &m_mouseState) )
	{
		bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height) );
		bgfx::touch(0);
		clearBuffer();
		
		int pos = 0;
		for (const auto entry : m_filelist)
		{
			bufferPrintf(0, pos, 0x0f, "%-40s", entry.c_str());
			pos++;
		}
		bufferAttrib(0, m_selected, 0x40, 80);
		bgfx::dbgTextImage(0, 0, m_text_width, m_text_height, m_buffer.data(), m_text_width*2);

		bgfx::frame();
		return true;
	}

	return false;
}
