#ifndef VAPOURSYNTHSCRIPTPROCESSOR_H
#define VAPOURSYNTHSCRIPTPROCESSOR_H

#include <QObject>
#include <QPixmap>
#include <vapoursynth/VSScript.h>

#include "../settings/settingsmanager.h"

namespace vsedit
{
	class Resampler;
	class AbstractYuvToRgbConverter;
}

class VapourSynthScriptProcessor : public QObject
{
	Q_OBJECT

	public:

		VapourSynthScriptProcessor(SettingsManager * a_pSettingsManager,
			QObject * a_pParent = nullptr);

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

	private slots:

		void slotSettingsChanged();

	private:

		void handleVSMessage(int a_messageType, const QString & a_message);

		QPixmap pixmapFromFrame(const VSFrameRef * a_cpFrameRef);

		// pixmapfromframe.cpp
		QPixmap pixmapFromGray1B(const VSFrameRef * a_cpFrameRef);
		QPixmap pixmapFromGray2B(const VSFrameRef * a_cpFrameRef);
		QPixmap pixmapFromGrayH(const VSFrameRef * a_cpFrameRef);
		QPixmap pixmapFromGrayS(const VSFrameRef * a_cpFrameRef);
		QPixmap pixmapFromYUV1B(const VSFrameRef * a_cpFrameRef);
		QPixmap pixmapFromYUV2B(const VSFrameRef * a_cpFrameRef);
		QPixmap pixmapFromYUVH(const VSFrameRef * a_cpFrameRef);
		QPixmap pixmapFromYUVS(const VSFrameRef * a_cpFrameRef);
		QPixmap pixmapFromRGB1B(const VSFrameRef * a_cpFrameRef);
		QPixmap pixmapFromRGB2B(const VSFrameRef * a_cpFrameRef);
		QPixmap pixmapFromRGBH(const VSFrameRef * a_cpFrameRef);
		QPixmap pixmapFromRGBS(const VSFrameRef * a_cpFrameRef);
		QPixmap pixmapFromCompatBGR32(const VSFrameRef * a_cpFrameRef);
		QPixmap pixmapFromCompatYUY2(const VSFrameRef * a_cpFrameRef);

		friend void VS_CC vsMessageHandler(int a_msgType,
			const char * a_message, void * a_pUserData);

		SettingsManager * m_pSettingsManager;

		QString m_script;

		QString m_scriptName;

		QString m_error;

		bool m_vsScriptInitialized;

		bool m_initialized;
		const VSAPI * m_cpVSAPI;

		VSScript * m_pVSScript;

		VSNodeRef * m_pOutputNode;
		VSNodeRef * m_pPreviewNode;

		const VSVideoInfo * m_cpVideoInfo;

		int m_currentFrame;

		const VSFrameRef * m_cpCurrentFrameRef;

		ResamplingFilter m_chromaResamplingFilter;

		ChromaPlacement m_chromaPlacement;

		double m_resamplingFilterParameterA;

		double m_resamplingFilterParameterB;

		vsedit::AbstractYuvToRgbConverter * m_pYuvToRgbConverter;

		vsedit::Resampler * m_pResampler;

};

#endif // VAPOURSYNTHSCRIPTPROCESSOR_H
