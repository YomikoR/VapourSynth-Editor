#ifndef FRAME_HEADER_WRITER_Y4M_H_INCLUDED
#define FRAME_HEADER_WRITER_Y4M_H_INCLUDED

#include "frame_header_writer.h"

class FrameHeaderWriterY4M : public FrameHeaderWriter
{
	Q_OBJECT

public:

	FrameHeaderWriterY4M(const VSAPI * a_cpVSAPI = nullptr,
		const VSVideoInfo * a_cpVideoInfo = nullptr,
		QObject * a_pParent = nullptr);

	virtual bool isCompatible() override;

	virtual bool needVideoHeader() override;
	virtual QByteArray videoHeader(int a_totalFrames = -1) override;

	virtual bool needFramePrefix() override;
	virtual QByteArray framePrefix(const VSFrame * a_cpFrame) override;

	virtual bool needFramePostfix() override;
	virtual QByteArray framePostfix(const VSFrame * a_cpFrame) override;
};

#endif // FRAME_HEADER_WRITER_Y4M_H_INCLUDED
