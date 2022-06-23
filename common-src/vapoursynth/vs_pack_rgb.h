#ifndef VS_PACK_RGB_H_INCLUDED
#define VS_PACK_RGB_H_INCLUDED

#include <vapoursynth/VapourSynth4.h>

void VS_CC packCreateRGB24(const VSMap *in, VSMap *out, VSCore *core, const VSAPI *vsapi);
void VS_CC packCreateRGB30(const VSMap *in, VSMap *out, VSCore *core, const VSAPI *vsapi);

#endif
