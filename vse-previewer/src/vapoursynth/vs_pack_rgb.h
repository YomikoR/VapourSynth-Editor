#ifndef VS_PACK_RGB_H_INCLUDED
#define VS_PACK_RGB_H_INCLUDED

#include <vapoursynth/VapourSynth4.h>

VSNode *packRGBFilter(VSNode *rgbNode, VSNode *outputNode, bool use10bit, VSCore *core, const VSAPI *vsapi);

#endif
