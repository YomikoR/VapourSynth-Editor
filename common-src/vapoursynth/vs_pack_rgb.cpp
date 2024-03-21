#include "vs_pack_rgb.h"
#include "../libp2p/p2p_api.h"

#include <vapoursynth/VSHelper4.h>

#include <vector>

struct PackData
{
    VSNode *rgbNode;
    VSNode *outputNode;
};

void packFree(void *instanceData, [[maybe_unused]] VSCore *core, const VSAPI *vsapi)
{
    PackData *d = reinterpret_cast<PackData *>(instanceData);
    vsapi->freeNode(d->rgbNode);
    // The outputNode is freed somewhere else
    // vsapi->freeNode(d->outputNode);
    delete d;
}

template <p2p_packing packing_fmt>
const VSFrame *packGetFrame(int n, int activationReason, void *instanceData, [[maybe_unused]] void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi)
{
    PackData *d = reinterpret_cast<PackData *>(instanceData);
    if (activationReason == arInitial)
    {
        vsapi->requestFrameFilter(n, d->outputNode, frameCtx);
        vsapi->requestFrameFilter(n, d->rgbNode, frameCtx);
    }
    else if (activationReason == arAllFramesReady)
    {
        const VSFrame *srcFrame = vsapi->getFrameFilter(n, d->rgbNode, frameCtx);
        VSVideoFormat frameFormat;
        vsapi->getVideoFormatByID(&frameFormat, pfGray8, core);
        int width = vsapi->getFrameWidth(srcFrame, 0);
        int height = vsapi->getFrameHeight(srcFrame, 0);
        VSFrame *dstFrame = vsapi->newVideoFrame(&frameFormat, width * 4, height, nullptr, core);

        p2p_buffer_param p = {};
        p.width = width;
        p.height = height;
        p.packing = packing_fmt;
        for (int plane = 0; plane < 3; ++plane)
        {
            p.src[plane] = vsapi->getReadPtr(srcFrame, plane);
            p.src_stride[plane] = vsapi->getStride(srcFrame, plane);
        }
        p.dst[0] = vsapi->getWritePtr(dstFrame, 0);
        p.dst_stride[0] = vsapi->getStride(dstFrame, 0);
        p2p_pack_frame(&p, P2P_ALPHA_SET_ONE);

        VSMap *props = vsapi->getFramePropertiesRW(dstFrame);
        vsapi->mapSetInt(props, "PackingFormat", static_cast<int64_t>(packing_fmt), maReplace);

        const VSFrame *outputFrame = vsapi->getFrameFilter(n, d->outputNode, frameCtx);
        vsapi->mapConsumeFrame(props, "OutputFrame", outputFrame, maReplace);
        vsapi->freeFrame(srcFrame);
        return dstFrame;
    }
    return nullptr;
}

VSNode *packRGBFilter(VSNode *rgbNode, VSNode *outputNode, bool use10bit, VSCore *core, const VSAPI *vsapi)
{
    VSVideoInfo vi = *vsapi->getVideoInfo(rgbNode);
    vi.width *= 4;
    vsapi->getVideoFormatByID(&vi.format, pfGray8, core);

    PackData *d = new PackData{rgbNode, outputNode};

    std::vector<VSFilterDependency> deps = {
        {rgbNode, rpStrictSpatial},
        {outputNode, rpStrictSpatial}};

    if (use10bit)
        return vsapi->createVideoFilter2("PackRGB30", &vi, packGetFrame<p2p_rgb30>, packFree, fmParallel, deps.data(), deps.size(), d, core);
    else
        return vsapi->createVideoFilter2("PackRGB24", &vi, packGetFrame<p2p_argb32>, packFree, fmParallel, deps.data(), deps.size(), d, core);
}
