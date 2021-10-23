#include "vs_gray_frame_prop.h"

struct grayData
{
    VSNodeRef *node;
    const VSVideoInfo *vi;
};

void VS_CC grayFramePropInit(VSMap *in, VSMap *out, void **instanceData, VSNode *node, VSCore *core, const VSAPI *vsapi)
{
    grayData *d = reinterpret_cast<grayData *>(*instanceData);
    vsapi->setVideoInfo(d->vi, 1, node);
}

const VSFrameRef * VS_CC grayFramePropGetFrame(int n, int activationReason, void **instanceData, void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi)
{
    grayData *d = reinterpret_cast<grayData *>(*instanceData);
    if (activationReason == arInitial)
    {
        vsapi->requestFrameFilter(n, d->node, frameCtx);
    }
    else if (activationReason == arAllFramesReady)
    {
        const VSFrameRef *src_frame = vsapi->getFrameFilter(n, d->node, frameCtx);
        VSFrameRef *dst_frame = vsapi->copyFrame(src_frame, core);
        vsapi->freeFrame(src_frame);
        VSMap *props = vsapi->getFramePropsRW(dst_frame);
        int err;
        int matrix = int64ToIntS(vsapi->propGetInt(props, "_Matrix", 0, &err));
        if (!err && matrix == 0) // RGB matrix
        {
            vsapi->propDeleteKey(props, "_Matrix");
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
    grayData d;
    d.node = vsapi->propGetNode(in, "clip", 0, nullptr);
    d.vi = vsapi->getVideoInfo(d.node);
    grayData *data = new grayData(d);
    vsapi->createFilter(in, out, "GrayFrameProp", grayFramePropInit, grayFramePropGetFrame, grayFramePropFree, fmParallel, nfNoCache, data, core);
}
