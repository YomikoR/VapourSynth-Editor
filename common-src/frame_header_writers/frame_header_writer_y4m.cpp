#include "frame_header_writer_y4m.h"

#include "../../../common-src/helpers.h"

#include <string>
#include <map>

//==============================================================================

FrameHeaderWriterY4M::FrameHeaderWriterY4M(const VSAPI * a_cpVSAPI,
	const VSVideoInfo * a_cpVideoInfo, QObject * a_pParent) :
	FrameHeaderWriter(a_cpVSAPI, a_cpVideoInfo, a_pParent)
{
}

// END OF FrameHeaderWriterY4M::FrameHeaderWriterY4M(const VSAPI * a_cpVSAPI,
//		const VSVideoInfo * a_cpVideoInfo, QObject * a_pParent)
//==============================================================================

bool FrameHeaderWriterY4M::isCompatible()
{
    Q_ASSERT(m_cpVideoInfo);
    if(!m_cpVideoInfo)
		return false;

	const VSVideoFormat * cpFormat = &m_cpVideoInfo->format;

	int compatibleColorFamily[] = {cfGray, cfYUV};
	if(!vsedit::contains(compatibleColorFamily, cpFormat->colorFamily))
		return false;

	if(cpFormat->sampleType == stFloat)
	{
		int acceptableBitDepths[] = {16, 32, 64};
		if(!vsedit::contains(acceptableBitDepths, cpFormat->bitsPerSample))
			return false;
	}

	std::pair<int, int> compatibleSubsampling[] =
		{{0, 0}, {0, 1}, {1, 0}, {1, 1}, {2, 0}, {2, 2}};
	std::pair<int, int> subsampling(cpFormat->subSamplingW,
		cpFormat->subSamplingH);
	if(!vsedit::contains(compatibleSubsampling, subsampling))
		return false;

	return true;
}

// END OF bool FrameHeaderWriterY4M::isCompatible()
//==============================================================================

bool FrameHeaderWriterY4M::needVideoHeader()
{
	return true;
}

// END OF bool FrameHeaderWriterY4M::needVideoHeader()
//==============================================================================

QByteArray FrameHeaderWriterY4M::videoHeader(int a_totalFrames)
{
	Q_ASSERT(m_cpVideoInfo);
	Q_ASSERT(isCompatible());
	const VSVideoFormat * cpFormat = &m_cpVideoInfo->format;

	std::string header;

	header += "YUV4MPEG2 C";

	if(cpFormat->colorFamily == cfGray)
	{
		header += "mono";
		if(cpFormat->bitsPerSample > 8)
			header += std::to_string(cpFormat->bitsPerSample);
	}
	else if(cpFormat->colorFamily == cfYUV)
	{
		std::map<std::pair<int, int>, std::string> subsamplingStringMap =
		{
			{{0, 0}, "444"},
			{{0, 1}, "440"},
			{{1, 0}, "422"},
			{{1, 1}, "420"},
			{{2, 0}, "411"},
			{{2, 2}, "410"},
		};
		std::pair<int, int> subsampling(cpFormat->subSamplingW,
			cpFormat->subSamplingH);
		header += subsamplingStringMap[subsampling];

		if((cpFormat->bitsPerSample > 8) && (cpFormat->sampleType == stInteger))
			header += "p" + std::to_string(cpFormat->bitsPerSample);
		else if(cpFormat->sampleType == stFloat)
		{
			header += "p";
			if(cpFormat->bitsPerSample == 16)
				header += "h";
			else if(cpFormat->bitsPerSample == 32)
				header += "s";
			else if(cpFormat->bitsPerSample == 64)
				header += "d";
			else
			{
				Q_ASSERT(false);
				header += "u";
			}
		}
	}
	else
	{
		Q_ASSERT(false);
	}

	int totalFrames = (a_totalFrames < 0) ? m_cpVideoInfo->numFrames :
		a_totalFrames;

	header = header
		+ " W" + std::to_string(m_cpVideoInfo->width)
		+ " H" + std::to_string(m_cpVideoInfo->height)
		+ " F" + std::to_string(m_cpVideoInfo->fpsNum)
		+ ":" + std::to_string(m_cpVideoInfo->fpsDen)
        + " Ip A0:0"
        + " XLENGTH=" + std::to_string(totalFrames)
        + "\n";

	QByteArray headerData(header.c_str());
	return headerData;
}

// END OF QByteArray FrameHeaderWriterY4M::videoHeader(int a_totalFrames)
//==============================================================================

bool FrameHeaderWriterY4M::needFramePrefix()
{
	return true;
}

// END OF bool FrameHeaderWriterY4M::needFramePrefix()
//==============================================================================

QByteArray FrameHeaderWriterY4M::framePrefix(const VSFrame * a_cpFrame)
{
	(void)a_cpFrame;
	std::string prefix = "FRAME\n";
	QByteArray prefixData(prefix.c_str());
	return prefixData;
}

// END OF QByteArray FrameHeaderWriterY4M::framePrefix(
//		const VSFrame * a_cpFrame)
//==============================================================================

bool FrameHeaderWriterY4M::needFramePostfix()
{
	return false;
}

// END OF bool FrameHeaderWriterY4M::needFramePostfix()
//==============================================================================

QByteArray FrameHeaderWriterY4M::framePostfix(const VSFrame * a_cpFrame)
{
	(void)a_cpFrame;
	return QByteArray();
}

// END OF QByteArray FrameHeaderWriterY4M::framePostfix(
//		const VSFrame * a_cpFrame)
//==============================================================================
