#ifndef VAPOURSYNTHSCRIPTPROCESSOR_H
#define VAPOURSYNTHSCRIPTPROCESSOR_H

#include <QObject>
#include <QPixmap>
#include <QLibrary>
#include <vapoursynth/VSScript.h>

#include "../settings/settingsmanager.h"

typedef int (VS_CC *FNP_vssInit)(void);
typedef const VSAPI * (VS_CC *FNP_vssGetVSApi)(void);
typedef int (VS_CC *FNP_vssEvaluateScript)(VSScript ** a_handle,
	const char * a_script, const char * a_scriptFilename, int a_flags);
typedef const char * (VS_CC *FNP_vssGetError)(VSScript * a_handle);
typedef VSCore * (VS_CC *FNP_vssGetCore)(VSScript * a_handle);
typedef VSNodeRef * (VS_CC *FNP_vssGetOutput)(VSScript * a_handle,
	int a_index);
typedef void (VS_CC *FNP_vssFreeScript)(VSScript * a_handle);
typedef int (VS_CC *FNP_vssFinalize)(void);

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

		void colorAtPoint(size_t a_x, size_t a_y, double & a_rValue1,
			double & a_rValue2, double & a_rValue3);

	signals:

		void signalWriteLogMessage(int a_messageType,
			const QString & a_message);

	private slots:

		void slotSettingsChanged();

	private:

		void handleVSMessage(int a_messageType, const QString & a_message);

		QPixmap pixmapFromFrame(const VSFrameRef * a_cpFrameRef);

		bool initLibrary();

		void freeLibrary();

		double valueAtPoint(size_t a_x, size_t a_y, int a_plane);

		void initPreviewNode();

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

		YuvToRgbConversionMatrix m_yuvMatrix;

		QLibrary m_vsScriptLibrary;

		FNP_vssInit vssInit;
		FNP_vssGetVSApi vssGetVSApi;
		FNP_vssEvaluateScript vssEvaluateScript;
		FNP_vssGetError vssGetError;
		FNP_vssGetCore vssGetCore;
		FNP_vssGetOutput vssGetOutput;
		FNP_vssFreeScript vssFreeScript;
		FNP_vssFinalize vssFinalize;
};

#endif // VAPOURSYNTHSCRIPTPROCESSOR_H
