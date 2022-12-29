#pragma comment (lib, "user32.lib")
#pragma comment (lib, "kernel32.lib")

#include "version_info.h"

#include <iostream>

void print_version()
{
	std::cout << "VapourSynth Editor " << VSE_VERSION_STR << std::endl;
}

#if defined(_WIN32)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
void hide_tty()
{
	HWND console = GetConsoleWindow();
	DWORD pid;
	GetWindowThreadProcessId(console, &pid);
	if(GetCurrentProcessId() == pid)
		ShowWindow(console, SW_HIDE);
	else
		print_version();
}
#endif
