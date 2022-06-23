#include "frame_header_writer_null.h"

//==============================================================================

FrameHeaderWriterNull::FrameHeaderWriterNull(const VSAPI * a_cpVSAPI,
	const VSVideoInfo * a_cpVideoInfo, QObject * a_pParent) :
	FrameHeaderWriter(a_cpVSAPI, a_cpVideoInfo, a_pParent)
{
}

// END OF FrameHeaderWriterNull::FrameHeaderWriterNull(const VSAPI * a_cpVSAPI,
//		const VSVideoInfo * a_cpVideoInfo, QObject * a_pParent)
//==============================================================================

bool FrameHeaderWriterNull::isCompatible()
{
    Q_ASSERT(m_cpVideoInfo);
    if(!m_cpVideoInfo)
		return false;
	return true;
}

// END OF bool FrameHeaderWriterNull::isCompatible()
//==============================================================================

bool FrameHeaderWriterNull::needVideoHeader()
{
	return false;
}

// END OF bool FrameHeaderWriterNull::needVideoHeader()
//==============================================================================

QByteArray FrameHeaderWriterNull::videoHeader(int a_totalFrames)
{
	(void)a_totalFrames;
	return QByteArray();
}

// END OF QByteArray FrameHeaderWriterNull::videoHeader(int a_totalFrames)
//==============================================================================

bool FrameHeaderWriterNull::needFramePrefix()
{
	return false;
}

// END OF bool FrameHeaderWriterNull::needFramePrefix()
//==============================================================================

QByteArray FrameHeaderWriterNull::framePrefix(const VSFrame * a_cpFrame)
{
	(void)a_cpFrame;
	return QByteArray();
}

// END OF QByteArray FrameHeaderWriterNull::framePrefix(
//		const VSFrame * a_cpFrame)
//==============================================================================

bool FrameHeaderWriterNull::needFramePostfix()
{
	return false;
}

// END OF bool FrameHeaderWriterNull::needFramePostfix()
//==============================================================================

QByteArray FrameHeaderWriterNull::framePostfix(const VSFrame * a_cpFrame)
{
	(void)a_cpFrame;
	return QByteArray();
}

// END OF QByteArray FrameHeaderWriterNull::framePostfix(
//		const VSFrame * a_cpFrame)
//==============================================================================
