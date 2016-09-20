#include "benchmark_dialog.h"

#include "../common/helpers.h"
#include "../vapoursynth/vapoursynthscriptprocessor.h"

#include <vapoursynth/VapourSynth.h>

//==============================================================================

ScriptBenchmarkDialog::ScriptBenchmarkDialog(
	SettingsManager * a_pSettingsManager, QWidget * a_pParent):
	VSScriptProcessorDialog(a_pSettingsManager, a_pParent, (Qt::WindowFlags)0
		| Qt::Window
		| Qt::CustomizeWindowHint
		| Qt::WindowMinimizeButtonHint
		| Qt::WindowCloseButtonHint
		)
	, m_processing(false)
	, m_framesTotal(0)
	, m_framesProcessed(0)
{
	m_ui.setupUi(this);
	setWindowIcon(QIcon(":benchmark.png"));

	createStatusBar();

	connect(m_pVapourSynthScriptProcessor,
		SIGNAL(signalDistributeFrame(int, const VSFrameRef *)),
		this, SLOT(slotReceiveFrame(int, const VSFrameRef *)));
	connect(m_ui.wholeVideoButton, SIGNAL(clicked()),
		this, SLOT(slotWholeVideoButtonPressed()));
	connect(m_ui.startStopBenchmarkButton, SIGNAL(clicked()),
		this, SLOT(slotStartStopBenchmarkButtonPressed()));
}

// END OF ScriptBenchmarkDialog::ScriptBenchmarkDialog(
//		VapourSynthScriptProcessor * a_pVapourSynthScriptProcessor,
//		QWidget * a_pParent
//==============================================================================

ScriptBenchmarkDialog::~ScriptBenchmarkDialog()
{
}

// END OF ScriptBenchmarkDialog::~ScriptBenchmarkDialog()
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

	assert(m_cpVideoInfo);

	QString text = trUtf8("Ready to benchmark script %1").arg(m_scriptName);
	m_ui.feedbackTextEdit->setPlainText(text);
	m_ui.metricsEdit->clear();
	int lastFrame = m_cpVideoInfo->numFrames - 1;
	m_ui.fromFrameSpinBox->setMaximum(lastFrame);
	m_ui.fromFrameSpinBox->setValue(0);
	m_ui.toFrameSpinBox->setMaximum(lastFrame);
	m_ui.toFrameSpinBox->setValue(lastFrame);
	m_ui.processingProgressBar->setMaximum(m_cpVideoInfo->numFrames);
	m_ui.processingProgressBar->setValue(0);
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
	m_ui.feedbackTextEdit->appendPlainText(a_message);
}

// END OF void ScriptBenchmarkDialog::slotWriteLogMessage(int a_messageType,
//		const QString & a_message)
//==============================================================================

void ScriptBenchmarkDialog::slotWholeVideoButtonPressed()
{
	assert(m_cpVideoInfo);
	int lastFrame = m_cpVideoInfo->numFrames - 1;
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
	int firstFrame = m_ui.fromFrameSpinBox->value();
	int lastFrame = m_ui.toFrameSpinBox->value();

	if(firstFrame > lastFrame)
	{
		m_ui.feedbackTextEdit->appendPlainText(trUtf8(
			"First frame number is larger than the last frame number."));
			return;
	}

	m_framesTotal = lastFrame - firstFrame + 1;
	m_ui.processingProgressBar->setMaximum(m_framesTotal);
	m_ui.startStopBenchmarkButton->setText(trUtf8("Stop"));
	m_processing = true;

	m_benchmarkStartTime = hr_clock::now();

	for(int i = firstFrame; i <= lastFrame; ++i)
		m_pVapourSynthScriptProcessor->requestFrameAsync(i);
}

// END OF void ScriptBenchmarkDialog::slotStartStopBenchmarkButtonPressed()
//==============================================================================

void ScriptBenchmarkDialog::slotReceiveFrame(int a_frameNumber,
	const VSFrameRef * a_cpFrameRef)
{
	(void)(a_frameNumber);
	(void)(a_cpFrameRef);

	if(!m_processing)
		return;

	hr_time_point now = hr_clock::now();
	m_framesProcessed++;
	m_ui.processingProgressBar->setValue(m_framesProcessed);
	double passed = duration_to_double(now - m_benchmarkStartTime);
	QString passedString = vsedit::timeToString(passed);
	double fps = (double)m_framesProcessed / passed;
	QString text = trUtf8("Time elapsed: %1 - %2 FPS")
		.arg(passedString).arg(QString::number(fps, 'f', 20));
	m_ui.metricsEdit->setText(text);

	if(m_framesProcessed == m_framesTotal)
		stopProcessing();
}

// END OF void ScriptBenchmarkDialog::slotReceiveFrame(int a_frameNumber,
//		const VSFrameRef * a_cpFrameRef)
//==============================================================================

void ScriptBenchmarkDialog::stopProcessing()
{
	if(!m_processing)
		return;

	m_processing = false;
	m_pVapourSynthScriptProcessor->flushFrameTicketsQueue();
	m_ui.startStopBenchmarkButton->setText(trUtf8("Start"));
}

// END OF void ScriptBenchmarkDialog::stopProcessing()
//==============================================================================
