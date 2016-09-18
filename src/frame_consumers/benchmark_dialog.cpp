#include "benchmark_dialog.h"

#include "../common/helpers.h"
#include "../vapoursynth/vapoursynthscriptprocessor.h"

#include <vapoursynth/VapourSynth.h>

//==============================================================================

ScriptBenchmarkDialog::ScriptBenchmarkDialog(
	VapourSynthScriptProcessor * a_pVapourSynthScriptProcessor,
	QWidget * a_pParent) :
	QDialog(a_pParent, (Qt::WindowFlags)0
		| Qt::Window
		| Qt::CustomizeWindowHint
		| Qt::WindowMinimizeButtonHint
		| Qt::WindowCloseButtonHint
		)
	, m_pVapourSynthScriptProcessor(a_pVapourSynthScriptProcessor)
	, m_processing(false)
	, m_framesTotal(0)
	, m_framesProcessed(0)
{
	m_ui.setupUi(this);
	setWindowIcon(QIcon(":time.png"));

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

	if(!m_pVapourSynthScriptProcessor->isInitialized())
		return;

	const VSVideoInfo * cpVideoInfo =
		m_pVapourSynthScriptProcessor->videoInfo();
	assert(cpVideoInfo);

	m_ui.outputTextEdit->clear();
	int lastFrame = cpVideoInfo->numFrames - 1;
	m_ui.fromFrameSpinBox->setMaximum(lastFrame);
	m_ui.fromFrameSpinBox->setValue(0);
	m_ui.toFrameSpinBox->setMaximum(lastFrame);
	m_ui.toFrameSpinBox->setValue(lastFrame);
	m_ui.processingProgressBar->setMaximum(cpVideoInfo->numFrames);
	m_ui.processingProgressBar->setValue(0);
	show();
}

// END OF void ScriptBenchmarkDialog::call()
//==============================================================================

void ScriptBenchmarkDialog::slotWholeVideoButtonPressed()
{
	const VSVideoInfo * cpVideoInfo =
		m_pVapourSynthScriptProcessor->videoInfo();
	assert(cpVideoInfo);

	int lastFrame = cpVideoInfo->numFrames - 1;
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
		m_ui.outputTextEdit->setPlainText(trUtf8(
			"First frame number is larger than the last frame number."));
			return;
	}

	m_framesTotal = lastFrame - firstFrame + 1;
	m_ui.processingProgressBar->setMaximum(m_framesTotal);
	m_ui.startStopBenchmarkButton->setText(trUtf8("Stop"));
	m_processing = true;
	connect(m_pVapourSynthScriptProcessor,
		SIGNAL(signalDistributeFrame(int, const VSFrameRef *)),
		this, SLOT(slotReceiveFrame(int, const VSFrameRef *)));
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
	double fps = (double)m_framesProcessed / passed;
	QString text = trUtf8("%1 / %2\n%3 FPS").arg(m_framesProcessed)
		.arg(m_framesTotal).arg(QString::number(fps, 'f', 20));
	m_ui.outputTextEdit->setPlainText(text);

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
	m_pVapourSynthScriptProcessor->flushFrameTicketsQueueForConsumer();
	disconnect(m_pVapourSynthScriptProcessor,
		SIGNAL(signalDistributeFrame(int, const VSFrameRef *)),
		this, SLOT(slotReceiveFrame(int, const VSFrameRef *)));
	m_ui.startStopBenchmarkButton->setText(trUtf8("Start"));
}

// END OF void ScriptBenchmarkDialog::stopProcessing()
//==============================================================================
