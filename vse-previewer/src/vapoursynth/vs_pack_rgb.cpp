#include "vs_pack_rgb.h"
#include "../libp2p/p2p_api.h"

#include <vapoursynth/VSHelper4.h>

#include <memory>

struct packData
{
    VSNode *node;
    VSVideoInfo vi;
    enum p2p_packing packing_fmt;
};

const VSFrame * VS_CC packGetFrame(int n, int activationReason, void *instanceData, void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi)
{
    packData *d = reinterpret_cast<packData *>(instanceData);

    if (activationReason == arInitial)
    {
        vsapi->requestFrameFilter(n, d->node, frameCtx);
    }
    else if (activationReason == arAllFramesReady)
    {
        const VSFrame *src_frame = vsapi->getFrameFilter(n, d->node, frameCtx);
        int width = vsapi->getFrameWidth(src_frame, 0);
        int height = vsapi->getFrameHeight(src_frame, 0);
        VSFrame *dst_frame = vsapi->newVideoFrame(&d->vi.format, width * 4, height, nullptr, core);

        p2p_buffer_param p = {};
        p.width = width;
        p.height = height;
        p.packing = d->packing_fmt;
        for (int plane = 0; plane < 3; ++plane)
        {
            p.src[plane] = vsapi->getReadPtr(src_frame, plane);
            p.src_stride[plane] = vsapi->getStride(src_frame, plane);
        }
        p.dst[0] = vsapi->getWritePtr(dst_frame, 0);
        p.dst_stride[0] = vsapi->getStride(dst_frame, 0);
        p2p_pack_frame(&p, P2P_ALPHA_SET_ONE);

        VSMap *props = vsapi->getFramePropertiesRW(dst_frame);
        vsapi->mapSetInt(props, "PackingFormat", static_cast<int64_t>(d->packing_fmt), maReplace);
        vsapi->freeFrame(src_frame);
        return dst_frame;
    }
    return nullptr;
}

void VS_CC packFree(void *instanceData, VSCore *core, const VSAPI *vsapi)
{
    packData *d = reinterpret_cast<packData *>(instanceData);
    vsapi->freeNode(d->node);
    delete d;
}

void VS_CC packCreateRGB24(const VSMap *in, VSMap *out, VSCore *core, const VSAPI *vsapi)
{
    std::unique_ptr<packData> d(new packData());

    d->node = vsapi->mapGetNode(in, "clip", 0, nullptr);

    const VSVideoInfo *vi = vsapi->getVideoInfo(d->node);
    d->vi = *vi;
    d->vi.width *= 4;
    vsapi->getVideoFormatByID(&d->vi.format, pfGray8, core);

    d->packing_fmt = p2p_argb32;

    VSFilterDependency deps[] = {{d->node, rpStrictSpatial}};

    vsapi->createVideoFilter(out, "PackRGB24", &d->vi, packGetFrame, packFree, fmParallel, deps, 1, d.get(), core);

    d.release();
}

void VS_CC packCreateRGB30(const VSMap *in, VSMap *out, VSCore *core, const VSAPI *vsapi)
{
    std::unique_ptr<packData> d(new packData());

    d->node = vsapi->mapGetNode(in, "clip", 0, nullptr);
    vsapi->getVideoFormatByID(&d->vi.format, pfGray8, core);
    d->vi.width *= 4;
    d->packing_fmt = p2p_rgb30;

    VSFilterDependency deps[] = {{d->node, rpStrictSpatial}};

    vsapi->createVideoFilter(out, "PackRGB30", &d->vi, packGetFrame, packFree, fmParallel, deps, 1, d.get(), core);

    d.release();
}
