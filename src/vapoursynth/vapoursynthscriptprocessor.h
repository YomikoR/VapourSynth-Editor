#ifndef VAPOURSYNTHSCRIPTPROCESSOR_H
#define VAPOURSYNTHSCRIPTPROCESSOR_H

#include <QObject>
#include <QPixmap>
#include <vapoursynth/VSScript.h>

#include "pixmapfromframe.h"

class VapourSynthScriptProcessor : public QObject
{
	Q_OBJECT

	public:

		VapourSynthScriptProcessor(QObject * a_pParent = nullptr);

		virtual ~VapourSynthScriptProcessor();

		bool initialize(const QString& a_script, const QString& a_scriptName);

		void finalize();

		bool isInitialized() const;

		QString error() const;

		const VSVideoInfo * videoInfo();

		bool requestFrame(int a_frameNumber);

		void freeFrame();

		QPixmap pixmap(int a_frameNumber);

	signals:

		void signalWriteLogMessage(int a_messageType,
			const QString & a_message);

	private:

		void handleVSMessage(int a_messageType, const QString & a_message);

		void setFrameConverter();

		friend void VS_CC vsMessageHandler(int a_msgType,
			const char * a_message, void * a_pUserData);

		QString m_script;

		QString m_scriptName;

		QString m_error;

		bool m_vsScriptInitialized;

		bool m_initialized;

		const VSAPI * m_cpVSAPI;

		VSScript * m_pVSScript;

		VSNodeRef * m_pOutputNode;

		const VSVideoInfo * m_cpVideoInfo;

		int m_currentFrame;

		const VSFrameRef * m_cpCurrentFrameRef;

		PixmapFromFrameFunc m_pixmapFromFrame;

};

#endif // VAPOURSYNTHSCRIPTPROCESSOR_H
