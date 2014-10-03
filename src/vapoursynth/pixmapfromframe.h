#ifndef PIXMAPFROMFRAME_H_INCLUDED
#define PIXMAPFROMFRAME_H_INCLUDED

#include <QPixmap>
#include <vapoursynth/VapourSynth.h>

typedef QPixmap (*PixmapFromFrameFunc)(const VSAPI * a_cpVSAPI,
	const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef);

namespace vsedit
{
	QPixmap pixmapFromGray1B(const VSAPI * a_cpVSAPI,
		const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef);
	QPixmap pixmapFromGray2B(const VSAPI * a_cpVSAPI,
		const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef);
	QPixmap pixmapFromGrayH(const VSAPI * a_cpVSAPI,
		const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef);
	QPixmap pixmapFromGrayS(const VSAPI * a_cpVSAPI,
		const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef);
	QPixmap pixmapFromYUV1B(const VSAPI * a_cpVSAPI,
		const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef);
	QPixmap pixmapFromYUV2B(const VSAPI * a_cpVSAPI,
		const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef);
	QPixmap pixmapFromYUVH(const VSAPI * a_cpVSAPI,
		const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef);
	QPixmap pixmapFromYUVS(const VSAPI * a_cpVSAPI,
		const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef);
	QPixmap pixmapFromRGB1B(const VSAPI * a_cpVSAPI,
		const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef);
	QPixmap pixmapFromRGB2B(const VSAPI * a_cpVSAPI,
		const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef);
	QPixmap pixmapFromRGBH(const VSAPI * a_cpVSAPI,
		const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef);
	QPixmap pixmapFromRGBS(const VSAPI * a_cpVSAPI,
		const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef);
	QPixmap pixmapFromCompatBGR32(const VSAPI * a_cpVSAPI,
		const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef);
	QPixmap pixmapFromCompatYUY2(const VSAPI * a_cpVSAPI,
		const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef);
}

#endif // PIXMAPFROMFRAME_H_INCLUDED
