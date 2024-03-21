#include "vs_set_matrix.h"

#include <vapoursynth/VSHelper4.h>
#include <vapoursynth/VSConstants4.h>

const VSFrame * VS_CC setMatrixGetFrame(int n, int activationReason, void *instanceData, [[maybe_unused]] void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi)
{
    VSNode *node = reinterpret_cast<VSNode *>(instanceData);
    if (activationReason == arInitial)
    {
        vsapi->requestFrameFilter(n, node, frameCtx);
    }
    else if (activationReason == arAllFramesReady)
    {
        const VSFrame *src_frame = vsapi->getFrameFilter(n, node, frameCtx);
        const VSVideoFormat *vff = vsapi->getVideoFrameFormat(src_frame);
        if (vff->colorFamily == cfGray)
        {
            const VSMap *src_props = vsapi->getFramePropertiesRO(src_frame);
            int err;
            int64_t matrix = vsapi->mapGetInt(src_props, "_Matrix", 0, &err);
            if (!err && matrix == VSC_MATRIX_RGB)
            {
                VSFrame *dst_frame = vsapi->copyFrame(src_frame, core);
                vsapi->freeFrame(src_frame);
                vsapi->mapDeleteKey(vsapi->getFramePropertiesRW(dst_frame), "_Matrix");
                return dst_frame;
            }
        }
        return src_frame;
    }
    return nullptr;
}

void VS_CC setMatrixFree(void *instanceData, [[maybe_unused]] VSCore *core, const VSAPI *vsapi)
{
    vsapi->freeNode(reinterpret_cast<VSNode *>(instanceData));
}

void VS_CC setMatrixFilter(const VSMap *in, VSMap *out, VSCore *core, const VSAPI *vsapi)
{
    VSNode *node = vsapi->mapGetNode(in, "clip", 0, nullptr);
    VSFilterDependency deps[] = {{node, rpStrictSpatial}};
    vsapi->createVideoFilter(out, "SetMatrix", vsapi->getVideoInfo(node), setMatrixGetFrame, setMatrixFree, fmParallel, deps, 1, node, core);
}
