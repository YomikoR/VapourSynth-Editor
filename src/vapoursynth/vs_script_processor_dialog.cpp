#include "vs_script_processor_dialog.h"

#include "../settings/settingsmanager.h"
#include "vapoursynthscriptprocessor.h"

#include <vapoursynth/VapourSynth.h>

#include <cassert>

//==============================================================================

VSScriptProcessorDialog::VSScriptProcessorDialog(
	SettingsManager * a_pSettingsManager, QWidget * a_pParent,
	Qt::WindowFlags a_flags):
	  QDialog(a_pParent, a_flags)
	, m_pSettingsManager(a_pSettingsManager)
	, m_pVapourSynthScriptProcessor(nullptr)
	, m_cpVSAPI(nullptr)
	, m_cpVideoInfo(nullptr)
	, m_framesInQueue(0)
	, m_framesInProcess(0)
	, m_maxThreads(0)
{
	assert(a_pSettingsManager);
	m_pVapourSynthScriptProcessor =
		new VapourSynthScriptProcessor(a_pSettingsManager, this);

	connect(m_pVapourSynthScriptProcessor,
		SIGNAL(signalWriteLogMessage(int, const QString &)),
		this, SLOT(slotWriteLogMessage(int, const QString &)));
	connect(m_pVapourSynthScriptProcessor,
		SIGNAL(signalFrameQueueStateChanged(size_t, size_t, size_t)),
		this, SLOT(slotFrameQueueStateChanged(size_t, size_t, size_t)));
}

// END OF VSScriptProcessorDialog::VSScriptProcessorDialog(
//		SettingsManager * a_pSettingsManager, QWidget * a_pParent,
//		Qt::WindowFlags a_flags)
//==============================================================================

VSScriptProcessorDialog::~VSScriptProcessorDialog()
{
	m_pVapourSynthScriptProcessor->finalize();
}

// END OF VSScriptProcessorDialog::~VSScriptProcessorDialog()
//==============================================================================

bool VSScriptProcessorDialog::initialize(const QString & a_script,
	const QString & a_scriptName)
{
	assert(m_pVapourSynthScriptProcessor);

	if(m_pVapourSynthScriptProcessor->isInitialized())
	{
		bool finalized = m_pVapourSynthScriptProcessor->finalize();
		if(!finalized)
			return false;
	}

	bool initialized = m_pVapourSynthScriptProcessor->initialize(a_script,
		a_scriptName);
	if(!initialized)
		return false;

	m_script = a_script;
	m_scriptName = a_scriptName;

	m_cpVSAPI = m_pVapourSynthScriptProcessor->api();
	assert(m_cpVSAPI);

	m_cpVideoInfo = m_pVapourSynthScriptProcessor->videoInfo();

	return true;
}

// END OF bool VSScriptProcessorDialog::initialize(const QString & a_script,
//		const QString & a_scriptName)
//==============================================================================

void VSScriptProcessorDialog::slotWriteLogMessage(int a_messageType,
	const QString & a_message)
{
	emit signalWriteLogMessage(a_messageType, a_message);
}

// END OF void VSScriptProcessorDialog::slotWriteLogMessage(int a_messageType,
//		const QString & a_message)
//==============================================================================

void VSScriptProcessorDialog::slotFrameQueueStateChanged(size_t a_inQueue,
	size_t a_inProcess, size_t a_maxThreads)
{
	m_framesInQueue = a_inQueue;
	m_framesInProcess = a_inProcess;
	m_maxThreads = a_maxThreads;
}

// END OF void VSScriptProcessorDialog::slotFrameQueueStateChanged(
//		size_t a_inQueue, size_t a_inProcess, size_t a_maxThreads)
//==============================================================================
