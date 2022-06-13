#include "vs_icm.h"
#include "../libp2p/p2p_api.h"

#include <cmath>

/* THE FORMATS
 * libp2p   p2p_abgr64
 * lcms2    TYPE_RGBA_16
 * Qt       Format_RGBA64 (Since 5.12)
 * * Dithering in lcms2 is slowwwww
*/

#define CMS_NO_REGISTER_KEYWORD 1
#include <lcms2.h>

struct cspData
{
    cmsFloat64Number xw, yw, xr, yr, xg, yg, xb, yb;
};

static constexpr cspData csp_709 = {0.3127, 0.3290, 0.64, 0.33, 0.3, 0.6, 0.15, 0.06};

// 170m
static constexpr cspData csp_601_525 = {0.3127, 0.3290, 0.63, 0.34, 0.31, 0.595, 0.155, 0.07};

// 2020-10
static constexpr cspData csp_2020 = {0.3127, 0.3290, 0.708, 0.292, 0.17, 0.797, 0.131, 0.046};

// NTSC
static constexpr cspData csp_1953 = {0.31, 0.316, 0.67, 0.33, 0.21, 0.71, 0.14, 0.08};

cmsHPROFILE gen_bt1886_profile(const cmsHPROFILE profile, const cspData &csp)
{
    cmsContext ctx = cmsCreateContext(nullptr, nullptr);

    // Find black pt
    cmsCIEXYZ bp;
    if(!cmsDetectDestinationBlackPoint(&bp, profile, INTENT_RELATIVE_COLORIMETRIC, 0))
        return nullptr;
    double min_luma = bp.Y; // Should be sufficient for sane usage

    // Build tone curve
    double binv = std::pow(min_luma, 1.0 / 2.4);
    cmsFloat64Number params[4] = {2.4, 1.0 - binv, binv, 0.0};
    cmsToneCurve *tone_curve[3];
    if(!(tone_curve[0] = cmsBuildParametricToneCurve(ctx, 6, params)))
        return nullptr;
    tone_curve[1] = tone_curve[0];
    tone_curve[2] = tone_curve[0];

    // Create profile
    cmsCIExyY wp_xyY = {csp.xw, csp.yw, 1.0};
    cmsCIExyYTRIPLE prim_xyY = {
        {csp.xr, csp.yr, 1.0},
        {csp.xg, csp.yg, 1.0},
        {csp.xb, csp.yb, 1.0}
    };
    cmsHPROFILE profile_dst = cmsCreateRGBProfile(&wp_xyY, &prim_xyY, tone_curve);
    cmsFreeToneCurve(tone_curve[0]);

    cmsDeleteContext(ctx);

    return profile_dst;
}

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

    cmsHPROFILE icm_dst = gen_bt1886_profile(icm, csp_709);
    if(!icm_dst)
    {
        vsapi->freeNode(d.node);
        vsapi->setError(out, "icm: Unable to create BT.1886 profile.");
        cmsCloseProfile(icm);
        return;
    }

    cmsUInt32Number intent = INTENT_RELATIVE_COLORIMETRIC;

    cmsUInt32Number flags = cmsFLAGS_HIGHRESPRECALC | cmsFLAGS_BLACKPOINTCOMPENSATION;

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
