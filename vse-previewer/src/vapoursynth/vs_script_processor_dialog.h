#ifndef VS_SCRIPT_PROCESSOR_DIALOG_H_INCLUDED
#define VS_SCRIPT_PROCESSOR_DIALOG_H_INCLUDED

#include "../helpers.h"
#include "vs_script_processor_structures.h"
#include "vapoursynth_script_processor.h"
#include "../script_status_bar_widget/script_status_bar_widget.h"

#include <QDialog>
#include <QPixmap>
#include <QString>
#include <list>
#include <map>

class QCloseEvent;
class QStatusBar;
class QLabel;
class SettingsManager;
class VSScriptLibrary;
class VapourSynthScriptProcessor;
struct VSAPI;
struct VSVideoInfo;
struct VSFrame;

class VSScriptProcessorDialog : public QDialog
{
	Q_OBJECT

public:

	VSScriptProcessorDialog(SettingsManager * a_pSettingsManager,
		VSScriptLibrary * a_pVSScriptLibrary, QWidget * a_pParent = nullptr,
		Qt::WindowFlags a_flags =
		  Qt::Window
		| Qt::CustomizeWindowHint
		| Qt::WindowMinimizeButtonHint
		| Qt::WindowMaximizeButtonHint
		| Qt::WindowCloseButtonHint);

	virtual ~VSScriptProcessorDialog();

	virtual bool initialize(const QString & a_script,
		const QString & a_scriptName);

	virtual bool busy(int a_outputIndex = 0) const;

	virtual const QString & script() const;

	virtual const QString & scriptName() const;

	virtual void setScriptName(const QString & a_scriptName);

	virtual void setVSScriptLibraryLogs();

protected slots:

	virtual void slotWriteLogMessage(int a_messageType,
		const QString & a_message);

	virtual void slotFrameQueueStateChanged(size_t a_inQueue,
		size_t a_inProcess, size_t a_maxThreads, double a_usedCacheRatio);

	virtual void slotScriptProcessorFinalized();

	virtual void slotReceiveFrame(int a_frameNumber, int a_outputIndex,
		const VSFrame * a_cpOutputFrame,
		const VSFrame * a_cpPreviewFrame) = 0;

	virtual void slotFrameRequestDiscarded(int a_frameNumber,
		int a_outputIndex, const QString & a_reason) = 0;

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

	const VSAPI * m_cpVSAPI;

	std::map<int, VSNodeInfo> m_nodeInfo;

	std::map<int, size_t> m_framesInQueue;
	std::map<int, size_t> m_framesInProcess;
	size_t m_maxThreads;
	double m_usedCacheRatio;

	int m_outputIndex;

	bool m_wantToFinalize;
	bool m_wantToClose;

	QStatusBar * m_pStatusBar;
	ScriptStatusBarWidget * m_pStatusBarWidget;

	QPixmap m_readyPixmap;
	QPixmap m_busyPixmap;
	QPixmap m_errorPixmap;

	std::map<int, std::list<Frame>> m_framesCache;
	size_t m_cachedFramesLimit;

	QString m_clipName;
	QString m_sceneName;
	QString m_absoluteTime;
};

#endif // VS_SCRIPT_PROCESSOR_DIALOG_H_INCLUDED
