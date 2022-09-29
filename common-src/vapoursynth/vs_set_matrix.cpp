#include "vs_set_matrix.h"

#include <vapoursynth/VSHelper4.h>
#include <vapoursynth/VSConstants4.h>

#include <memory>
#include <cstring>

struct setMatrixData
{
    VSNode *node;
    VSVideoInfo vi;

    // Default values

    int64_t matrix_in;
    int64_t transfer_in;
    bool is_transfer_in_given = false;
    int64_t chromaloc;
};

const VSFrame * VS_CC setMatrixGetFrame(int n, int activationReason, void *instanceData, void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi)
{
    setMatrixData *d = reinterpret_cast<setMatrixData *>(instanceData);
    if (activationReason == arInitial)
    {
        vsapi->requestFrameFilter(n, d->node, frameCtx);
    }
    else if (activationReason == arAllFramesReady)
    {
        const VSFrame *src_frame = vsapi->getFrameFilter(n, d->node, frameCtx);

        /* RGB: tag RGB matrix
           YUV: if missing matrix, add it
                if transfer is given, add it with matrix
           GRAY: if having RGB matrix, drop it
         */

        const VSVideoFormat *vff = vsapi->getVideoFrameFormat(src_frame);

        VSFrame *dst_frame = vsapi->copyFrame(src_frame, core);
        vsapi->freeFrame(src_frame);
        VSMap *props = vsapi->getFramePropertiesRW(dst_frame);

        if (vff->colorFamily == cfRGB)
        {
            vsapi->mapSetInt(props, "_Matrix", VSC_MATRIX_RGB, maReplace);
        }
        else if (vff->colorFamily == cfYUV)
        {
            int err;

            int64_t matrix = vsapi->mapGetInt(props, "_Matrix", 0, &err);
            if (err)
            {
                vsapi->mapSetInt(props, "_Matrix", d->matrix_in, maReplace);
                if (d->is_transfer_in_given)
                {
                    vsapi->mapSetInt(props, "_Transfer", d->transfer_in, maReplace);
                }
            }

            int64_t chromaloc = vsapi->mapGetInt(props, "_ChromaLocation", 0, &err);
            if (err)
            {
                vsapi->mapSetInt(props, "_ChromaLocation", d->chromaloc, maReplace);
            }
        }
        else
        {
            int err;

            int64_t matrix = vsapi->mapGetInt(props, "_Matrix", 0, &err);
            if (!err && matrix == 0)
            {
                vsapi->mapDeleteKey(props, "_Matrix");
            }
        }
        return dst_frame;
    }
    return nullptr;
}

void VS_CC setMatrixFree(void *instanceData, VSCore *core, const VSAPI *vsapi)
{
    setMatrixData *d = reinterpret_cast<setMatrixData *>(instanceData);
    vsapi->freeNode(d->node);
    delete d;
}

void VS_CC setMatrixFilter(const VSMap *in, VSMap *out, VSCore *core, const VSAPI *vsapi)
{
    std::unique_ptr<setMatrixData> d(new setMatrixData());

    d->node = vsapi->mapGetNode(in, "clip", 0, nullptr);
    const VSVideoInfo *vi = vsapi->getVideoInfo(d->node);
    d->vi = *vi;

    int err;
    d->matrix_in = vsapi->mapGetInt(in, "matrix_in", 0, &err);
    d->transfer_in = vsapi->mapGetInt(in, "transfer_in", 0, &err);
    if (!err) d->is_transfer_in_given = true;
    d->chromaloc = vsapi->mapGetInt(in, "chromaloc", 0, &err);

    VSFilterDependency deps[] = {{d->node, rpStrictSpatial}};

    vsapi->createVideoFilter(out, "SetMatrix", &d->vi, setMatrixGetFrame, setMatrixFree, fmParallel, deps, 1, d.get(), core);

    d.release();
}
