#ifndef VS_SCRIPT_PROCESSOR_DIALOG_H_INCLUDED
#define VS_SCRIPT_PROCESSOR_DIALOG_H_INCLUDED

#include <QDialog>
#include <QPixmap>
#include <list>

class QCloseEvent;
class QStatusBar;
class QLabel;
class SettingsManager;
class VapourSynthScriptProcessor;
struct VSAPI;
struct VSVideoInfo;
struct VSFrameRef;

namespace vsedit
{
	struct Frame
	{
		int number;
		int outputIndex;
		const VSFrameRef * cpFrameRef;

		Frame(int a_number, int a_outputIndex,
			const VSFrameRef * a_cpFrameRef);
		bool operator==(const Frame & a_other) const;
	};
}

class VSScriptProcessorDialog : public QDialog
{
	Q_OBJECT

	public:

		VSScriptProcessorDialog(SettingsManager * a_pSettingsManager,
			QWidget * a_pParent = nullptr, Qt::WindowFlags a_flags =
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
			const VSFrameRef * a_cpFrameRef) = 0;

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

		std::list<vsedit::Frame> m_framesCache;
		size_t m_cachedFramesLimit;
};

#endif // VS_SCRIPT_PROCESSOR_DIALOG_H_INCLUDED
