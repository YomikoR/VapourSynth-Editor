#include "vs_script_processor_dialog.h"

#include "../helpers.h"
#include "../settings/settings_manager.h"
#include "vapoursynth_script_processor.h"
#include "vs_script_library.h"

#include <vapoursynth/VapourSynth4.h>

#include <QCloseEvent>
#include <QStatusBar>
#include <QLabel>
#include <QLayout>

//==============================================================================

VSScriptProcessorDialog::VSScriptProcessorDialog(
	SettingsManager * a_pSettingsManager, VSScriptLibrary * a_pVSScriptLibrary,
	QWidget * a_pParent, Qt::WindowFlags a_flags):
	  QDialog(a_pParent, a_flags)
	, m_pSettingsManager(a_pSettingsManager)
	, m_pVSScriptLibrary(a_pVSScriptLibrary)
	, m_pVapourSynthScriptProcessor(nullptr)
	, m_cpVSAPI(nullptr)
	, m_framesInQueue()
	, m_framesInProcess()
	, m_maxThreads(0)
	, m_usedCacheRatio(0.0)
	, m_outputIndex(0)
	, m_wantToFinalize(false)
	, m_wantToClose(false)
	, m_pStatusBar(nullptr)
	, m_pStatusBarWidget(nullptr)
	, m_readyPixmap(":tick.png")
	, m_busyPixmap(":busy.png")
	, m_errorPixmap(":cross.png")
	, m_cachedFramesLimit(100)
	, m_clipName("")
	, m_sceneName("")
	, m_absoluteTime("")
{
	Q_ASSERT(m_pSettingsManager);
	Q_ASSERT(m_pVSScriptLibrary);

	m_outputIndex = a_pVSScriptLibrary->getDefaultOutputIndex();

	//connect(m_pVSScriptLibrary,
	//	SIGNAL(signalWriteLogMessage(int, const QString &)),
	//	this, SLOT(slotWriteLogMessage(int, const QString &)));

	m_pVapourSynthScriptProcessor = new VapourSynthScriptProcessor(
		m_pSettingsManager, m_pVSScriptLibrary, this);

	m_pStatusBarWidget = new ScriptStatusBarWidget();

	connect(m_pVapourSynthScriptProcessor,
		SIGNAL(signalWriteLogMessage(int, const QString &)),
		this, SLOT(slotWriteLogMessage(int, const QString &)));
	connect(m_pVapourSynthScriptProcessor,
		SIGNAL(signalFrameQueueStateChanged(size_t, size_t, size_t, double)),
		this, SLOT(slotFrameQueueStateChanged(size_t, size_t, size_t, double)));
	connect(m_pVapourSynthScriptProcessor, SIGNAL(signalFinalized()),
		this, SLOT(slotScriptProcessorFinalized()));
	connect(m_pVapourSynthScriptProcessor,
		SIGNAL(signalDistributeFrame(int, int, const VSFrame *,
			const VSFrame *)),
		this, SLOT(slotReceiveFrame(int, int, const VSFrame *,
			const VSFrame *)));
	connect(m_pVapourSynthScriptProcessor,
		SIGNAL(signalFrameRequestDiscarded(int, int, const QString &)),
		this, SLOT(slotFrameRequestDiscarded(int, int, const QString &)));
}

// END OF VSScriptProcessorDialog::VSScriptProcessorDialog(
//		SettingsManager * a_pSettingsManager,
//		VSScriptLibrary * a_pVSScriptLibrary,
//		QWidget * a_pParent, Qt::WindowFlags a_flags)
//==============================================================================

VSScriptProcessorDialog::~VSScriptProcessorDialog()
{
	stopAndCleanUp();
	m_pVapourSynthScriptProcessor->finalize();
}

// END OF VSScriptProcessorDialog::~VSScriptProcessorDialog()
//==============================================================================

bool VSScriptProcessorDialog::initialize(const QString & a_script,
	const QString & a_scriptName)
{
	Q_ASSERT(m_pVapourSynthScriptProcessor);

	m_cpVSAPI = m_pVSScriptLibrary->getVSAPI();
	if(!m_cpVSAPI)
		return false;

	bool initialized = m_pVapourSynthScriptProcessor->initialize(a_script,
		a_scriptName, m_outputIndex);
	if(!initialized)
	{
		if(isVisible())
			hide();
		return false;
	}

	m_nodeInfo[m_outputIndex] =
		m_pVapourSynthScriptProcessor->nodeInfo(m_outputIndex);
	Q_ASSERT(!m_nodeInfo[m_outputIndex].isInvalid());

	m_pStatusBarWidget->setNodeInfo(m_nodeInfo[m_outputIndex], m_cpVSAPI);

	return true;
}

// END OF bool VSScriptProcessorDialog::initialize(const QString & a_script,
//		const QString & a_scriptName)
//==============================================================================

bool VSScriptProcessorDialog::busy(int a_outputIndex) const
{
	return (m_framesInProcess.at(a_outputIndex)
		+ m_framesInQueue.at(a_outputIndex) != 0);
}

// END OF bool VSScriptProcessorDialog::busy(int a_outputIndex)
//==============================================================================

const QString & VSScriptProcessorDialog::script() const
{
	return m_pVapourSynthScriptProcessor->script();
}

// END OF const QString & VSScriptProcessorDialog::script() const
//==============================================================================

const QString & VSScriptProcessorDialog::scriptName() const
{
	return m_pVapourSynthScriptProcessor->scriptName();
}

// END OF const QString & VSScriptProcessorDialog::scriptName() const
//==============================================================================

void VSScriptProcessorDialog::setScriptName(const QString & a_scriptName)
{
	m_pVapourSynthScriptProcessor->setScriptName(a_scriptName);
}

// END OF void VSScriptProcessorDialog::setScriptName(
//		const QString & a_scriptName)
//==============================================================================

void VSScriptProcessorDialog::setVSScriptLibraryLogs()
{
	connect(m_pVSScriptLibrary,
		SIGNAL(signalWriteLogMessage(int, const QString &)),
		this, SLOT(slotWriteLogMessage(int, const QString &)));
}

void VSScriptProcessorDialog::slotWriteLogMessage(int a_messageType,
	const QString & a_message)
{
	emit signalWriteLogMessage(a_messageType, a_message);
}

// END OF void VSScriptProcessorDialog::slotWriteLogMessage(int a_messageType,
//		const QString & a_message)
//==============================================================================

void VSScriptProcessorDialog::slotFrameQueueStateChanged(size_t a_inQueue,
	size_t a_inProcess, size_t a_maxThreads, double a_usedCacheRatio)
{
	m_framesInQueue[m_outputIndex] = a_inQueue;
	m_framesInProcess[m_outputIndex] = a_inProcess;
	m_maxThreads = a_maxThreads;
	m_usedCacheRatio = a_usedCacheRatio;

	m_pStatusBarWidget->setQueueState(m_framesInQueue[m_outputIndex],
		m_framesInProcess[m_outputIndex], m_maxThreads, m_usedCacheRatio);
}

// END OF void VSScriptProcessorDialog::slotFrameQueueStateChanged(
//		size_t a_inQueue, size_t a_inProcess, size_t a_maxThreads,
//		double a_usedCacheRatio)
//==============================================================================

void VSScriptProcessorDialog::slotScriptProcessorFinalized()
{
	m_wantToFinalize = false;
	if(m_wantToClose)
	{
		m_wantToClose = false;
		close();
	}
}

// END OF void VSScriptProcessorDialog::slotScriptProcessofFinalized()
//==============================================================================


void VSScriptProcessorDialog::closeEvent(QCloseEvent * a_pEvent)
{
	if(m_wantToClose)
		return;

	m_wantToClose = true;
	stopAndCleanUp();

	bool finalized = m_pVapourSynthScriptProcessor->finalize();
	if(!finalized)
	{
		m_wantToFinalize = true;
		emit signalWriteLogMessage(mtWarning, tr("Script processor "
			"is busy. Dialog will close when it is finalized."));
		a_pEvent->ignore();
		return;
	}

	QDialog::closeEvent(a_pEvent);
	m_wantToClose = false;
}

// END OF void VSScriptProcessorDialog::closeEvent(QCloseEvent * a_pEvent)
//==============================================================================

void VSScriptProcessorDialog::stopAndCleanUp()
{
	clearFramesCache();
	for(auto & pair : m_nodeInfo)
		pair.second.setNull();

	m_outputIndex = m_pVSScriptLibrary->getDefaultOutputIndex();
}

// END OF void VSScriptProcessorDialog::stopAndCleanUp()
//==============================================================================

void VSScriptProcessorDialog::clearFramesCache()
{
	for(auto & pair : m_framesCache)
	{
		if(!pair.second.empty())
		{
			for(Frame & frame : pair.second)
			{
				m_cpVSAPI->freeFrame(frame.cpOutputFrame);
				m_cpVSAPI->freeFrame(frame.cpPreviewFrame);
			}
			pair.second.clear();
		}
	}
}

// END OF void VSScriptProcessorDialog::stopAndCleanUp()
//==============================================================================

void VSScriptProcessorDialog::createStatusBar()
{
	QLayout * pLayout = layout();
	Q_ASSERT(pLayout);
	if(!pLayout)
		return;

	m_pStatusBar = new QStatusBar(this);
	pLayout->addWidget(m_pStatusBar);

	m_pStatusBar->addPermanentWidget(m_pStatusBarWidget, 1);
}

// END OF void VSScriptProcessorDialog::createStatusBar()
//==============================================================================
