#ifndef VS_SCRIPT_PROCESSOR_DIALOG_H_INCLUDED
#define VS_SCRIPT_PROCESSOR_DIALOG_H_INCLUDED

#include <QDialog>

class SettingsManager;
class VapourSynthScriptProcessor;
struct VSAPI;
struct VSVideoInfo;

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

	protected slots:

		virtual void slotWriteLogMessage(int a_messageType,
			const QString & a_message);

		virtual void slotFrameQueueStateChanged(size_t a_inQueue,
			size_t a_inProcess, size_t a_maxThreads);

	signals:

		void signalWriteLogMessage(int a_messageType,
			const QString & a_message);

	protected:

		SettingsManager * m_pSettingsManager;

		VapourSynthScriptProcessor * m_pVapourSynthScriptProcessor;

		QString m_script;

		QString m_scriptName;

		const VSAPI * m_cpVSAPI;

		const VSVideoInfo * m_cpVideoInfo;

		size_t m_framesInQueue;
		size_t m_framesInProcess;
		size_t m_maxThreads;
};

#endif // VS_SCRIPT_PROCESSOR_DIALOG_H_INCLUDED
