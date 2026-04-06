#include "version_info.h"

#include <iostream>

void print_version()
{
	std::cerr << "VapourSynth Editor " << VSE_VERSION_STR << std::endl;
}

int version_compare(int major1, int minor1, int major2, int minor2)
{
	if(major1 > major2)
		return 1;
	else if(major1 < major2)
		return -1;
	else if(minor1 > minor2)
		return 1;
	else if(minor1 < minor2)
		return -1;
	else
		return 0;
}
