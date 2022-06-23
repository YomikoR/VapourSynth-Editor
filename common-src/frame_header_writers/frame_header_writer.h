#ifndef FRAME_HEADER_WRITER_H_INCLUDED
#define FRAME_HEADER_WRITER_H_INCLUDED

#include <vapoursynth/VapourSynth4.h>

#include <QObject>

class FrameHeaderWriter : public QObject
{
	Q_OBJECT

public:

	FrameHeaderWriter(const VSAPI * a_cpVSAPI = nullptr,
		const VSVideoInfo * a_cpVideoInfo = nullptr,
		QObject * a_pParent = nullptr);
	virtual ~FrameHeaderWriter();

	virtual void setVSAPI(const VSAPI * a_cpVSAPI);
	virtual void setVideoInfo(const VSVideoInfo * a_cpVideoInfo);

	virtual bool isCompatible() = 0;

	virtual bool needVideoHeader() = 0;
	virtual QByteArray videoHeader(int a_totalFrames = -1) = 0;

	virtual bool needFramePrefix() = 0;
	virtual QByteArray framePrefix(const VSFrame * a_cpFrame) = 0;

	virtual bool needFramePostfix() = 0;
	virtual QByteArray framePostfix(const VSFrame * a_cpFrame) = 0;

protected:

	const VSAPI * m_cpVSAPI;
	const VSVideoInfo * m_cpVideoInfo;
};

#endif // FRAME_HEADER_WRITER_H_INCLUDED
