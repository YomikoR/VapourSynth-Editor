#ifndef WIN32_CONSOLE_H_INCLUDED
#define WIN32_CONSOLE_H_INCLUDED

#if defined(_WIN32)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"

#include <iostream>

class AttachedConsole
{
private:
    HWND console;
    FILE *file;

    void init();
    void clear();

public:
    AttachedConsole();

    bool visible();
    void show();
    void hide();
    void destroy();
};

#endif

#endif
