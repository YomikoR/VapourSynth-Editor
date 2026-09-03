#ifndef VERSION_INFO_H_INCLUDED
#define VERSION_INFO_H_INCLUDED

#define VSE_VERSION_STR "R19-mod-6.10"

#define VS_USE_LATEST_API
#define VSE_VS_API_VER_MAJOR 4
#define VSE_VS_API_VER_MINOR 3

#define VSSCRIPT_USE_LATEST_API
#define VSE_VSS_API_VER_MAJOR 4
#define VSE_VSS_API_VER_MINOR 4

void print_version();

int version_compare(int major1, int minor1, int major2, int minor2);

#endif
