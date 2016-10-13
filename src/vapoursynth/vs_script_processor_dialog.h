#ifndef VS_SCRIPT_PROCESSOR_DIALOG_H_INCLUDED
#define VS_SCRIPT_PROCESSOR_DIALOG_H_INCLUDED

#include "vs_script_processor_structures.h"

#include <QDialog>
#include <QPixmap>
#include <list>

class QCloseEvent;
class QStatusBar;
class QLabel;
class SettingsManager;
class VSScriptLibrary;
class VapourSynthScriptProcessor;
struct VSAPI;
struct VSVideoInfo;
struct VSFrameRef;

class VSScriptProcessorDialog : public QDialog
{
	Q_OBJECT

	public:

		VSScriptProcessorDialog(SettingsManager * a_pSettingsManager,
			VSScriptLibrary * a_pVSScriptLibrary, QWidget * a_pParent = nullptr,
			Qt::WindowFlags a_flags =
			(Qt::WindowFlags)0
			| Qt::Window
			| Qt::CustomizeWindowHint
			| Qt::WindowMinimizeButtonHint
			| Qt::WindowMaximizeButtonHint
			| Qt::WindowCloseButtonHint);

		virtual ~VSScriptProcessorDialog();

		virtual bool initialize(const QString & a_script,
			const QString & a_scriptName);

		virtual bool busy() const;

	protected slots:

		virtual void slotWriteLogMessage(int a_messageType,
			const QString & a_message);

		virtual void slotFrameQueueStateChanged(size_t a_inQueue,
			size_t a_inProcess, size_t a_maxThreads);

		virtual void slotReceiveFrame(int a_frameNumber, int a_outputIndex,
			const VSFrameRef * a_cpOutputFrameRef,
			const VSFrameRef * a_cpPreviewFrameRef) = 0;

	signals:

		void signalWriteLogMessage(int a_messageType,
			const QString & a_message);

	protected:

		virtual void closeEvent(QCloseEvent * a_pEvent) override;

		virtual void stopAndCleanUp();

		virtual void clearFramesCache();

		/// Adds status bar to the dialog.
		/// Relies on dialog having a layout.
		/// Call in derived class after GUI is created.
		virtual void createStatusBar();

		SettingsManager * m_pSettingsManager;

		VSScriptLibrary * m_pVSScriptLibrary;

		VapourSynthScriptProcessor * m_pVapourSynthScriptProcessor;

		QString m_script;

		QString m_scriptName;

		const VSAPI * m_cpVSAPI;

		const VSVideoInfo * m_cpVideoInfo;

		size_t m_framesInQueue;
		size_t m_framesInProcess;
		size_t m_maxThreads;

		bool m_wantToFinalize;

		QStatusBar * m_pStatusBar;
		QLabel * m_pScriptProcessorStatusPixmapLabel;
		QLabel * m_pScriptProcessorStatusLabel;
		QLabel * m_pVideoInfoLabel;

		QPixmap m_readyPixmap;
		QPixmap m_busyPixmap;

		std::list<Frame> m_framesCache;
		size_t m_cachedFramesLimit;
};

#endif // VS_SCRIPT_PROCESSOR_DIALOG_H_INCLUDED
