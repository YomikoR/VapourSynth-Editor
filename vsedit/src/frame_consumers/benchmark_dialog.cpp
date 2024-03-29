#include "benchmark_dialog.h"

#include "../../../common-src/helpers.h"
#include "../../../common-src/vapoursynth/vapoursynth_script_processor.h"
#include "../../../common-src/settings/settings_manager.h"

#include <vapoursynth/VapourSynth4.h>

//==============================================================================

ScriptBenchmarkDialog::ScriptBenchmarkDialog(
	SettingsManager * a_pSettingsManager, VSScriptLibrary * a_pVSScriptLibrary,
	QWidget * a_pParent):
	VSScriptProcessorDialog(a_pSettingsManager, a_pVSScriptLibrary, a_pParent,
	      Qt::Window
		| Qt::CustomizeWindowHint
		| Qt::WindowMinimizeButtonHint
		| Qt::WindowCloseButtonHint
		)
	, m_processing(false)
	, m_framesTotal(0)
	, m_framesProcessed(0)
	, m_framesFailed(0)
	, m_lastFromFrame(-1)
	, m_lastToFrame(-1)
{
	vsedit::disableFontKerning(this);
	m_ui.setupUi(this);
	setWindowIcon(QIcon(":benchmark.png"));

	createStatusBar();

	m_ui.feedbackTextEdit->setName("benchmark_log");
	m_ui.feedbackTextEdit->setSettingsManager(m_pSettingsManager);
	m_ui.feedbackTextEdit->loadSettings();

	connect(m_ui.wholeVideoButton, SIGNAL(clicked()),
		this, SLOT(slotWholeVideoButtonPressed()));
	connect(m_ui.startStopBenchmarkButton, SIGNAL(clicked()),
		this, SLOT(slotStartStopBenchmarkButtonPressed()));
}

// END OF ScriptBenchmarkDialog::ScriptBenchmarkDialog(
//		SettingsManager * a_pSettingsManager,
//		VSScriptLibrary * a_pVSScriptLibrary, QWidget * a_pParent
//==============================================================================

ScriptBenchmarkDialog::~ScriptBenchmarkDialog()
{
}

// END OF ScriptBenchmarkDialog::~ScriptBenchmarkDialog()
//==============================================================================

bool ScriptBenchmarkDialog::initialize(const QString & a_script,
	const QString & a_scriptName, ProcessReason a_reason)
{
	bool initialized = VSScriptProcessorDialog::initialize(a_script,
		a_scriptName, a_reason);
	if(!initialized)
		emit signalWriteLogMessage(mtCritical,
			m_pVapourSynthScriptProcessor->error());
	return initialized;
}

// END OF bool ScriptBenchmarkDialog::initialize(const QString & a_script,
//		const QString & a_scriptName)
//==============================================================================

void ScriptBenchmarkDialog::resetSavedRange()
{
	m_lastFromFrame = -1;
	m_lastToFrame = -1;
}

// END OF void ScriptBenchmarkDialog::resetSavedRange()
//==============================================================================

void ScriptBenchmarkDialog::call()
{
	if(m_processing)
	{
		show();
		return;
	}

	if((!m_pVapourSynthScriptProcessor->isInitialized()) || m_wantToFinalize)
		return;

	Q_ASSERT(!m_nodeInfo[0].isInvalid());

	m_ui.feedbackTextEdit->clear();
	setWindowTitle(tr("Benchmark: %1").arg(scriptName()));
	QString text = tr("Ready to benchmark script %1").arg(scriptName());
	m_ui.feedbackTextEdit->addEntry(text);
	m_ui.metricsEdit->clear();
	int firstFrame = 0;
	int lastFrame = m_nodeInfo[0].numFrames() - 1;
	m_ui.fromFrameSpinBox->setMaximum(lastFrame);
	m_ui.toFrameSpinBox->setMaximum(lastFrame);
	m_ui.processingProgressBar->setMaximum(m_nodeInfo[0].numFrames());
	m_ui.processingProgressBar->setValue(0);

	if(m_lastFromFrame >= 0)
		firstFrame = std::min(m_lastFromFrame, lastFrame);
	m_ui.fromFrameSpinBox->setValue(firstFrame);

	if(m_lastToFrame >= 0)
		lastFrame = std::min(m_lastToFrame, lastFrame);
	m_ui.toFrameSpinBox->setValue(lastFrame);

	show();
}

// END OF void ScriptBenchmarkDialog::call()
//==============================================================================

void ScriptBenchmarkDialog::stopAndCleanUp()
{
	stopProcessing();
	m_ui.metricsEdit->clear();
	m_ui.processingProgressBar->setValue(0);
}

// END OF void ScriptBenchmarkDialog::stopAndCleanUp()
//==============================================================================

void ScriptBenchmarkDialog::slotWriteLogMessage(int a_messageType,
	const QString & a_message)
{
	QString style = vsMessageTypeToStyleName(a_messageType);

	QString debugTypes[] = {
		LOG_STYLE_DEBUG,
		LOG_STYLE_QT_DEBUG,
		LOG_STYLE_VS_DEBUG,
	};
	if(m_pSettingsManager->getShowDebugMessages() ||
		!vsedit::contains(debugTypes, style))
	{
		m_ui.feedbackTextEdit->addEntry(a_message, style);
	}
}

// END OF void ScriptBenchmarkDialog::slotWriteLogMessage(int a_messageType,
//		const QString & a_message)
//==============================================================================

void ScriptBenchmarkDialog::slotWholeVideoButtonPressed()
{
	Q_ASSERT(!m_nodeInfo[0].isInvalid());
	int lastFrame = m_nodeInfo[0].numFrames() - 1;
	m_ui.fromFrameSpinBox->setValue(0);
	m_ui.toFrameSpinBox->setValue(lastFrame);
}

// END OF void ScriptBenchmarkDialog::slotWholeVideoButtonPressed()
//==============================================================================

void ScriptBenchmarkDialog::slotStartStopBenchmarkButtonPressed()
{
	if(m_processing)
	{
		stopProcessing();
		return;
	}

	m_framesProcessed = 0;
	m_framesFailed = 0;
	int firstFrame = m_ui.fromFrameSpinBox->value();
	int lastFrame = m_ui.toFrameSpinBox->value();

	if(firstFrame > lastFrame)
	{
		m_ui.feedbackTextEdit->addEntry(tr(
			"First frame number is larger than the last frame number."),
			LOG_STYLE_WARNING);
			return;
	}

	m_framesTotal = lastFrame - firstFrame + 1;
	m_ui.processingProgressBar->setMaximum(m_framesTotal);
	m_ui.startStopBenchmarkButton->setText(tr("Stop"));
	setWindowTitle(tr("0% Benchmark: %1").arg(scriptName()));

	m_lastFromFrame = firstFrame;
	m_lastToFrame = lastFrame;

	m_processing = true;
	m_benchmarkStartTime = hr_clock::now();

	for(int i = firstFrame; i <= lastFrame; ++i)
		m_pVapourSynthScriptProcessor->requestFrameAsync(i);
}

// END OF void ScriptBenchmarkDialog::slotStartStopBenchmarkButtonPressed()
//==============================================================================

void ScriptBenchmarkDialog::slotReceiveFrame(int a_frameNumber,
	int a_outputIndex, const VSFrame * a_cpOutputFrame,
	const VSFrame * a_cpPreviewFrame)
{
	(void)a_frameNumber;
	(void)a_outputIndex;
	(void)a_cpOutputFrame;
	(void)a_cpPreviewFrame;

	if(!m_processing)
		return;

	m_framesProcessed++;
	updateMetrics();
}

// END OF void ScriptBenchmarkDialog::slotReceiveFrame(int a_frameNumber,
//		int a_outputIndex, const VSFrame * a_cpOutputFrame,
//		const VSFrame * a_cpPreviewFrame)
//==============================================================================

void ScriptBenchmarkDialog::slotFrameRequestDiscarded(int a_frameNumber,
	int a_outputIndex, const QString & a_reason)
{
	(void)a_frameNumber;
	(void)a_outputIndex;
	(void)a_reason;

	if(!m_processing)
		return;

	m_framesProcessed++;
	m_framesFailed++;
	updateMetrics();
}

// END OF void ScriptBenchmarkDialog::slotFrameRequestDiscarded(
//		int a_frameNumber, int a_outputIndex, const QString & a_reason)
//==============================================================================

void ScriptBenchmarkDialog::stopProcessing()
{
	if(!m_processing)
		return;

	m_processing = false;
	m_pVapourSynthScriptProcessor->flushFrameTicketsQueue();
	m_ui.startStopBenchmarkButton->setText(tr("Start"));
}

// END OF void ScriptBenchmarkDialog::stopProcessing()
//==============================================================================

void ScriptBenchmarkDialog::updateMetrics()
{
	hr_time_point now = hr_clock::now();
	m_ui.processingProgressBar->setValue(m_framesProcessed);
	double passed = duration_to_double(now - m_benchmarkStartTime);
	QString passedString = vsedit::timeToString(passed);
	double fps = (double)m_framesProcessed / passed;
	QString text = tr("Time elapsed: %1 - %2 FPS")
		.arg(passedString).arg(QString::number(fps, 'f', 20));

	if(m_framesFailed > 0)
		text += tr("; %1 frames failed").arg(m_framesFailed);

	if(m_framesProcessed < m_framesTotal)
	{
		double estimated = (m_framesTotal - m_framesProcessed) / fps;
		QString estimatedString = vsedit::timeToString(estimated);
		text += tr("; estimated time to finish: %1").arg(estimatedString);
	}

	m_ui.metricsEdit->setText(text);

	int percentage = (int)((double)m_framesProcessed * 100.0 /
		(double)m_framesTotal);
	setWindowTitle(tr("%1% Benchmark: %2")
		.arg(percentage).arg(scriptName()));

	if(m_framesProcessed == m_framesTotal)
		stopProcessing();
}

// END OF void ScriptBenchmarkDialog::updateMetrics()
//==============================================================================

void ScriptBenchmarkDialog::keyPressEvent(QKeyEvent * a_pEvent)
{
	Qt::KeyboardModifiers modifiers = a_pEvent->modifiers();

	if(modifiers != Qt::NoModifier)
	{
		QDialog::keyPressEvent(a_pEvent);
		return;
	}

	if(a_pEvent->key() == Qt::Key_Escape)
		close();
	else
		QDialog::keyPressEvent(a_pEvent);
}

// END OF void ScriptBenchmarkDialog::keyPressEvent(QKeyEvent * a_pEvent)
//==============================================================================
