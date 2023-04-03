#pragma comment (lib, "kernel32.lib")
#pragma comment (lib, "user32.lib")
#if defined(_WIN32)

#include "win32_console.h"

void AttachedConsole::init()
{
    if (console)
        return;

    if (!AllocConsole())
        return;

    console = GetConsoleWindow();
    if (!console)
        return;

    freopen_s(&file, "CONOUT$", "w", stderr);
    freopen_s(&file, "CONOUT$", "w", stdout);

    HANDLE handle = CreateFile(L"CONOUT$", GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, NULL);

    SetStdHandle(STD_OUTPUT_HANDLE, handle);
    SetStdHandle(STD_ERROR_HANDLE, handle);
}

AttachedConsole::AttachedConsole() : console(NULL), file(nullptr)
{
    init();
    if (console)
        ShowWindow(console, SW_HIDE);
}

bool AttachedConsole::visible()
{
    if (!console)
        return false;

    return IsWindowVisible(console);
}

void AttachedConsole::show()
{
    if (!console)
    {
        init();
        if (!console)
            return;
    }

    ShowWindow(console, SW_SHOWNOACTIVATE);
}

void AttachedConsole::hide()
{
    if (!console)
        return;

    clear();
    ShowWindow(console, SW_HIDE);
}

void AttachedConsole::destroy()
{
    if (!console)
        return;

    clear();

    FreeConsole();
}

void AttachedConsole::clear()
{
    std::wcerr.clear();
    std::wcout.clear();
    std::wclog.clear();
    std::cerr.clear();
    std::cout.clear();
    std::clog.clear();

    if (console)
    {
        SetConsoleTextAttribute(console, 0);
    }
}

#endif
