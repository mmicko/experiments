#include <bx/uint32_t.h>
#include "common.h"
#include "bgfx_utils.h"
#include "entry/cmd.h"
#include "entry/input.h"
#include <dirent.h> 
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>

#if defined(WIN32)
constexpr char PATHSEPCH = '\\';
#else
constexpr char PATHSEPCH = '/';
#include <memory>
#include <unistd.h>
#endif

#if defined(__GNUC__)
#define ATTR_PRINTF(x,y)        __attribute__((format(printf, x, y)))
#else
#define ATTR_PRINTF(x,y)
#endif

bool get_full_path(std::string &dst, std::string const &path)
{
	try
	{
#if defined(WIN32)
		std::vector<char> path_buffer(MAX_PATH);
		if (::_fullpath(&path_buffer[0], path.c_str(), MAX_PATH))
		{
			dst = &path_buffer[0];
			return true;
		}
		else
		{
			return false;
		}
#else
		std::unique_ptr<char, void(*)(void *)> canonical(::realpath(path.c_str(), nullptr), &std::free);
		if (canonical)
		{
			dst = canonical.get();
			return true;
		}

		std::vector<char> path_buffer(PATH_MAX);
		if (::realpath(path.c_str(), &path_buffer[0]))
		{
			dst = &path_buffer[0];
			return true;
		}
		else if (path[0] == PATHSEPCH)
		{
			dst = path;
			return true;
		}
		else
		{
			while (!::getcwd(&path_buffer[0], path_buffer.size()))
			{
				if (errno != ERANGE)
					return false;
				else
					path_buffer.resize(path_buffer.size() * 2);
			}
			dst.assign(&path_buffer[0]).push_back(PATHSEPCH);
			dst.append(path);
			return true;
		}
#endif
	}
	catch (...)
	{
		return false;
	}
}

typedef std::chrono::time_point<std::chrono::high_resolution_clock> timepoint_type;
typedef std::chrono::high_resolution_clock clock_type;
struct fileData
{
	std::string name;
	std::string fullpath;
	bool isDirectory;
};
class mainapp : public entry::AppI
{
public:
	mainapp(): m_selected(0), m_width(0), m_height(0), m_debug(0), m_reset(0), m_text_width(0), m_text_height(0)
	{
	}

	void init(int _argc, char** _argv) override;
	virtual int shutdown() override;
	bool update() override;	

	void updateFolder(std::string path);
	void clearBuffer();
	void bufferPrintf(uint16_t x, uint16_t y, uint8_t attrib, char const *format, ...) ATTR_PRINTF(5, 6);
	void bufferAttrib(uint16_t x, uint16_t y, uint8_t attrib, uint16_t w);
	void checkKeyPress();
	void keypressed(entry::Key::Enum key);
private:
	std::vector<fileData> m_filelist;
	std::string m_path;
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
	timepoint_type m_keyTimePress[entry::Key::Count];
	clock_type m_clock;
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

	get_full_path(m_path, ".");
	updateFolder(m_path);
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

void mainapp::updateFolder(std::string path)
{
	m_filelist.clear();
	DIR* dir = opendir(path.c_str());
	if (NULL != dir)
	{
		for (dirent* item = readdir(dir); NULL != item; item = readdir(dir))
		{
			fileData fd;
			fd.name = std::string(item->d_name);
			fd.fullpath = path + PATHSEPCH + fd.name;
			fd.isDirectory = (item->d_type & DT_DIR)!=0;
			if (fd.name!=".") m_filelist.push_back(fd);
		}
		closedir(dir);
	}
	std::sort(m_filelist.begin(), m_filelist.end(), [](fileData a, fileData b) {
        return a.name < b.name;   
    });	
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
		if (m_selected < m_filelist.size()-1 && m_filelist.size()> 0) m_selected++;
	}
	if (key == entry::Key::Home) {
		m_selected = 0;
	}
	if (key == entry::Key::End) {
		if (m_filelist.size() > 0) m_selected = uint16_t(m_filelist.size() - 1); else m_selected = 0;
	}
	if (key == entry::Key::PageUp) {
		if (m_selected > 10) m_selected -= 10; else m_selected = 0;
	}
	if (key == entry::Key::PageDown) {
		m_selected+= 10;
		if (m_filelist.empty()) m_selected = 0;
		if (m_selected > m_filelist.size()-1) m_selected = uint16_t(m_filelist.size() - 1);
	}
	if (key == entry::Key::Return) {
		if (m_filelist.size()> 0 && m_filelist[m_selected].isDirectory)
		{
			get_full_path(m_path, m_filelist[m_selected].fullpath);
			updateFolder(m_path);
			
			m_selected = 0;
		}
	}
	if (key == entry::Key::Esc) {
	}
}

void mainapp::checkKeyPress()
{
	for (int32_t ii = 0; ii < int32_t(entry::Key::Count); ++ii)
	{
		bool oldpressed = m_keyState[ii];
		bool pressed = inputGetKeyState(entry::Key::Enum(ii));
		if (oldpressed == false && pressed == true) { // Key pressed
			m_keyTimePress[ii] = m_clock.now();
			m_keyState[ii] = true;
		}
		if (oldpressed == true && pressed == true) { // Key being pressed
			auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(m_clock.now() - m_keyTimePress[ii]);
			if (elapsed.count() > 150) { // 150ms
				m_keyState[ii] = false;
				keypressed(entry::Key::Enum(ii));
			}
		}
		if (oldpressed == true && pressed == false) { // Key released
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
		
		bufferPrintf(0, 0, 0x0f, "%s", m_path.c_str());
		int pos = 1;
		for (const auto entry : m_filelist)
		{
			bufferPrintf(0, pos, 0x0f, "%-40s", entry.name.c_str());
			if (entry.isDirectory)
				bufferPrintf(40, pos, 0x03, "<dir>");
			pos++;
		}
		bufferAttrib(0, m_selected+1, 0x40, 80);
		bgfx::dbgTextImage(0, 0, m_text_width, m_text_height, m_buffer.data(), m_text_width*2);

		bgfx::frame();
		return true;
	}

	return false;
}
