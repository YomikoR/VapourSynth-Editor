#include "vs_icm.h"
#include "../libp2p/p2p_api.h"

/* THE FORMATS
 * libp2p   p2p_abgr64
 * lcms2    TYPE_RGBA_16
 * Qt       Format_RGBA64 (Since 5.12)
 * * Dithering in lcms2 is slowwwww
*/

#define CMS_NO_REGISTER_KEYWORD 1
#include <lcms2.h>

struct icmData
{
    VSNodeRef *node = nullptr;
    const VSVideoInfo *vi = nullptr;
    cmsHTRANSFORM transform = nullptr;
};

void VS_CC icmInit(VSMap *in, VSMap *out, void **instanceData, VSNode *node, VSCore *core, const VSAPI *vsapi)
{
    icmData *d = reinterpret_cast<icmData *>(*instanceData);
    VSVideoInfo new_vi = (VSVideoInfo) * (d->vi);
    new_vi.format = vsapi->registerFormat(cmGray, stInteger, 8, 0, 0, core);
    new_vi.width = d->vi->width * 8;
    new_vi.height = d->vi->height;
    vsapi->setVideoInfo(&new_vi, 1, node);
}

const VSFrameRef *VS_CC icmGetFrame(int n, int activationReason, void **instanceData, void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi)
{
    icmData *d = reinterpret_cast<icmData *>(*instanceData);

    if (activationReason == arInitial)
    {
        vsapi->requestFrameFilter(n, d->node, frameCtx);
    }
    else if (activationReason == arAllFramesReady)
    {
        const VSFrameRef *src_f = vsapi->getFrameFilter(n, d->node, frameCtx);
        int width = vsapi->getFrameWidth(src_f, 0);
        int height = vsapi->getFrameHeight(src_f, 0);
        int stride = vsapi->getStride(src_f, 0);
        const VSFormat *dstfmt = vsapi->registerFormat(cmGray, stInteger, 8, 0, 0, core);
        VSFrameRef *dst_f = vsapi->newVideoFrame(dstfmt, width * 8, height, nullptr, core);
        uint8_t *dst_buffer = vsapi->getWritePtr(dst_f, 0);
        int dst_stride = vsapi->getStride(dst_f, 0);

        // Pack to ABGR64
        p2p_buffer_param p = {};
        p.width = width;
        p.height = height;
        p.packing = p2p_abgr64;
        for (int plane = 0; plane < 3; ++plane)
        {
            p.src[plane] = vsapi->getReadPtr(src_f, plane);
            p.src_stride[plane] = vsapi->getStride(src_f, plane);
        }
        p.dst[0] = dst_buffer;
        p.dst_stride[0] = dst_stride;
        p2p_pack_frame(&p, P2P_ALPHA_SET_ONE);

        vsapi->freeFrame(src_f);

        // Do in-place CM
        if (d->transform)
            cmsDoTransformLineStride(d->transform, dst_buffer, dst_buffer, width, height, dst_stride, dst_stride, 0, 0);

        // Will dither in Qt
        return dst_f;
    }
    return nullptr;
}

void VS_CC icmFree(void *instanceData, VSCore *core, const VSAPI *vsapi)
{
    icmData *d = static_cast<icmData *>(instanceData);
    vsapi->freeNode(d->node);
    if (d->transform) cmsDeleteTransform(d->transform);
    delete d;
}

void VS_CC icmCreate(const VSMap *in, VSMap *out, VSCore *core, const VSAPI *vsapi)
{
    icmData d;

    d.node = vsapi->propGetNode(in, "clip", 0, nullptr);
    d.vi = vsapi->getVideoInfo(d.node);

    if (d.vi->format->id != pfRGB48)
    {
        vsapi->freeNode(d.node);
        vsapi->setError(out, "icm: Input clip must have RGB48 format.");
        return;
    }

    cmsUInt32Number icm_datatype = TYPE_RGBA_16;

    int err;
    cmsHPROFILE icm;
    const char *icm_profile = vsapi->propGetData(in, "icm", 0, &err);
    if (err || !(icm = cmsOpenProfileFromFile(icm_profile, "r")))
    {
        vsapi->freeNode(d.node);
        vsapi->setError(out, "icm: Unable to open ICM.");
        return;
    }

    cmsHPROFILE icm_dst = cmsCreate_sRGBProfile();

    cmsUInt32Number intent = cmsGetHeaderRenderingIntent(icm);

    cmsUInt32Number flags = cmsFLAGS_HIGHRESPRECALC;

    d.transform = cmsCreateTransform(icm_dst, icm_datatype, icm, icm_datatype, intent, flags);
    if (!d.transform)
    {
        vsapi->freeNode(d.node);
        cmsCloseProfile(icm);
        cmsCloseProfile(icm_dst);
        vsapi->setError(out, "icm: Unable to create transform.");
        return;
    }

    icmData *data = new icmData(d);

    vsapi->createFilter(in, out, "CM", icmInit, icmGetFrame, icmFree, fmParallel, 0, data, core);
}
