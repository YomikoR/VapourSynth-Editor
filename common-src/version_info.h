#ifndef VERSION_INFO_H_INCLUDED
#define VERSION_INFO_H_INCLUDED

#define VSE_VERSION_STR "R19-mod-5.4"

#include <iostream>

void print_version()
{
    std::cout << "VapourSynth Editor " << VSE_VERSION_STR << std::endl;
}

#endif
