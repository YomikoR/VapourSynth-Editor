#include "frame_header_writer.h"

//==============================================================================

FrameHeaderWriter::FrameHeaderWriter(const VSAPI * a_cpVSAPI,
	const VSVideoInfo * a_cpVideoInfo, QObject * a_pParent) :
	  QObject(a_pParent)
	, m_cpVSAPI(a_cpVSAPI)
	, m_cpVideoInfo(a_cpVideoInfo)
{
}

// END OF FrameHeaderWriter::FrameHeaderWriter(const VSAPI * a_cpVSAPI,
//		const VSVideoInfo * a_cpVideoInfo, QObject * a_pParent)
//==============================================================================

FrameHeaderWriter::~FrameHeaderWriter()
{
}

// END OF FrameHeaderWriter::~FrameHeaderWriter()
//==============================================================================

void FrameHeaderWriter::setVSAPI(const VSAPI * a_cpVSAPI)
{
	m_cpVSAPI = a_cpVSAPI;
}

// END OF void FrameHeaderWriter::setVSAPI(const VSAPI * a_cpVSAPI)
//==============================================================================

void FrameHeaderWriter::setVideoInfo(const VSVideoInfo * a_cpVideoInfo)
{
	m_cpVideoInfo = a_cpVideoInfo;
}

// END OF void FrameHeaderWriter::setVideoInfo(
//		const VSVideoInfo * a_cpVideoInfo)
//==============================================================================
