#include "vs_gray_frame_prop.h"

#include <vapoursynth/VSHelper4.h>

#include <memory>

struct grayData
{
    VSNode *node;
    VSVideoInfo vi;
};

const VSFrame * VS_CC grayFramePropGetFrame(int n, int activationReason, void *instanceData, void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi)
{
    grayData *d = reinterpret_cast<grayData *>(instanceData);
    if (activationReason == arInitial)
    {
        vsapi->requestFrameFilter(n, d->node, frameCtx);
    }
    else if (activationReason == arAllFramesReady)
    {
        const VSFrame *src_frame = vsapi->getFrameFilter(n, d->node, frameCtx);
        VSFrame *dst_frame = vsapi->copyFrame(src_frame, core);
        vsapi->freeFrame(src_frame);
        VSMap *props = vsapi->getFramePropertiesRW(dst_frame);
        int err;
        int matrix = vsh::int64ToIntS(vsapi->mapGetInt(props, "_Matrix", 0, &err));
        if (!err && matrix == 0) // RGB matrix
        {
            vsapi->mapDeleteKey(props, "_Matrix");
        }
        return dst_frame;
    }
    return nullptr;
}

void VS_CC grayFramePropFree(void *instanceData, VSCore *core, const VSAPI *vsapi)
{
    grayData *d = reinterpret_cast<grayData *>(instanceData);
    vsapi->freeNode(d->node);
    delete d;
}

void VS_CC grayFramePropCreate(const VSMap *in, VSMap *out, VSCore *core, const VSAPI *vsapi)
{
    std::unique_ptr<grayData> d(new grayData());

    d->node = vsapi->mapGetNode(in, "clip", 0, nullptr);
    const VSVideoInfo *vi = vsapi->getVideoInfo(d->node);
    d->vi = *vi;

    VSFilterDependency deps[] = {{d->node, rpStrictSpatial}};

    vsapi->createVideoFilter(out, "GrayFrameProp", &d->vi, grayFramePropGetFrame, grayFramePropFree, fmParallel, deps, 1, d.get(), core);

    d.release();
}
