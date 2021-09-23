#include "vs_pack_rgb.h"

struct packData
{
    VSNodeRef *node = nullptr;
    const VSVideoInfo *vi = nullptr;
    enum p2p_packing packing_fmt;
};

void VS_CC packInit(VSMap *in, VSMap *out, void **instanceData, VSNode *node, VSCore *core, const VSAPI *vsapi)
{
    packData *d = reinterpret_cast<packData *>(*instanceData);
    VSVideoInfo new_vi = (VSVideoInfo) * (d->vi);
    new_vi.format = vsapi->registerFormat(cmGray, stInteger, 8, 0, 0, core);
    new_vi.width = d->vi->width * 4;
    new_vi.height = d->vi->height;
    vsapi->setVideoInfo(&new_vi, 1, node);
}

const VSFrameRef * VS_CC packGetFrame(int n, int activationReason, void **instanceData, void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi)
{
    packData *d = reinterpret_cast<packData *>(*instanceData);

    if (activationReason == arInitial)
    {
        vsapi->requestFrameFilter(n, d->node, frameCtx);
    }
    else if (activationReason == arAllFramesReady)
    {
        const VSFrameRef *src_frame = vsapi->getFrameFilter(n, d->node, frameCtx);
        int width = vsapi->getFrameWidth(src_frame, 0);
        int height = vsapi->getFrameHeight(src_frame, 0);
        const VSFormat *dstfmt = vsapi->registerFormat(cmGray, stInteger, 8, 0, 0, core);
        VSFrameRef *dst_frame = vsapi->newVideoFrame(dstfmt, width * 4, height, nullptr, core);

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

        VSMap *props = vsapi->getFramePropsRW(dst_frame);
        vsapi->propSetInt(props, "_packingFormat", static_cast<int64_t>(d->packing_fmt), paReplace);
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
    packData d;
    d.node = vsapi->propGetNode(in, "clip", 0, nullptr);
    d.vi = vsapi->getVideoInfo(d.node);
    d.packing_fmt = p2p_argb32;
    packData *data = new packData(d);
    vsapi->createFilter(in, out, "PackRGB24", packInit, packGetFrame, packFree, fmParallel, 0, data, core);
}

void VS_CC packCreateRGB30(const VSMap *in, VSMap *out, VSCore *core, const VSAPI *vsapi)
{
    packData d;
    d.node = vsapi->propGetNode(in, "clip", 0, nullptr);
    d.vi = vsapi->getVideoInfo(d.node);
    d.packing_fmt = p2p_rgb30;
    packData *data = new packData(d);
    vsapi->createFilter(in, out, "PackRGB30", packInit, packGetFrame, packFree, fmParallel, 0, data, core);
}
