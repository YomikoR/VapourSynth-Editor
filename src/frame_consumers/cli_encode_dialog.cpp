#include "cli_encode_dialog.h"

#include "../common/helpers.h"
#include "../vapoursynth/vapoursynthscriptprocessor.h"

#include <vapoursynth/VapourSynth.h>
#include <vapoursynth/VSHelper.h>

#include <QMessageBox>
#include <QFileDialog>

//==============================================================================

NumberedFrameRef::NumberedFrameRef(int a_number,
	const VSFrameRef * a_cpFrameRef):
	number(a_number)
	, cpFrameRef(a_cpFrameRef)
{
}

bool NumberedFrameRef::operator<(const NumberedFrameRef & a_other) const
{
	if(this == &a_other)
		return false;

	if(number < a_other.number)
		return true;

	return false;
}

//==============================================================================

CLIEncodeDialog::CLIEncodeDialog(
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
	, m_lastFrameProcessed(-1)
{
	m_ui.setupUi(this);
	setWindowIcon(QIcon(":cli.png"));

	m_ui.executableBrowseButton->setIcon(QIcon(":folder.png"));

	m_ui.argumentsHelpButton->setIcon(QIcon(":information.png"));

	connect(m_ui.wholeVideoButton, SIGNAL(clicked()),
		this, SLOT(slotWholeVideoButtonPressed()));
	connect(m_ui.startStopBenchmarkButton, SIGNAL(clicked()),
		this, SLOT(slotStartStopBenchmarkButtonPressed()));
	connect(m_ui.executableBrowseButton, SIGNAL(clicked()),
		this, SLOT(slotExecutableBrowseButtonPressed()));
	connect(m_ui.argumentsHelpButton, SIGNAL(clicked()),
		this, SLOT(slotArgumentsHelpButtonPressed()));
}

// END OF CLIEncodeDialog::CLIEncodeDialog(
//		VapourSynthScriptProcessor * a_pVapourSynthScriptProcessor,
//		QWidget * a_pParent
//==============================================================================

CLIEncodeDialog::~CLIEncodeDialog()
{
	stopProcessing();
}

// END OF CLIEncodeDialog::~CLIEncodeDialog()
//==============================================================================

void CLIEncodeDialog::call()
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

	m_ui.feedbackTextEdit->clear();
	m_ui.metricsEdit->clear();
	int lastFrame = cpVideoInfo->numFrames - 1;
	m_ui.fromFrameSpinBox->setMaximum(lastFrame);
	m_ui.fromFrameSpinBox->setValue(0);
	m_ui.toFrameSpinBox->setMaximum(lastFrame);
	m_ui.toFrameSpinBox->setValue(lastFrame);
	m_ui.processingProgressBar->setMaximum(cpVideoInfo->numFrames);
	m_ui.processingProgressBar->setValue(0);
	show();
}

// END OF void CLIEncodeDialog::call()
//==============================================================================

void CLIEncodeDialog::closeEvent(QCloseEvent * a_pEvent)
{
	stopProcessing();

	bool finalized = m_pVapourSynthScriptProcessor->finalize();
	if(!finalized)
	{
		a_pEvent->ignore();
		return;
	}

	QDialog::closeEvent(a_pEvent);
}

// END OF void CLIEncodeDialog::call()
//==============================================================================

void CLIEncodeDialog::slotWholeVideoButtonPressed()
{
	const VSVideoInfo * cpVideoInfo =
		m_pVapourSynthScriptProcessor->videoInfo();
	assert(cpVideoInfo);

	int lastFrame = cpVideoInfo->numFrames - 1;
	m_ui.fromFrameSpinBox->setValue(0);
	m_ui.toFrameSpinBox->setValue(lastFrame);
}

// END OF void CLIEncodeDialog::slotWholeVideoButtonPressed()
//==============================================================================

void CLIEncodeDialog::slotStartStopBenchmarkButtonPressed()
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

	m_ui.feedbackTextEdit->appendPlainText(trUtf8("Command line:"));
	QString executable = m_ui.executablePathEdit->text();
	QString decodedArguments =
		decodeArguments(m_ui.argumentsTextEdit->toPlainText());
	QString commandLine = QString("\"%1\" %2").arg(executable)
		.arg(decodedArguments);
	m_ui.feedbackTextEdit->appendPlainText(commandLine);

	m_encoder.start(commandLine);
	if(!m_encoder.waitForStarted())
	{
		m_ui.feedbackTextEdit->appendPlainText(
			trUtf8("Couldn't start the encoder."));
		return;
	}

	m_processing = true;
	m_lastFrameProcessed = firstFrame - 1;
	m_framesTotal = lastFrame - firstFrame + 1;
	m_ui.processingProgressBar->setMaximum(m_framesTotal);
	m_ui.startStopBenchmarkButton->setText(trUtf8("Stop"));
	connect(m_pVapourSynthScriptProcessor,
		SIGNAL(signalDistributeFrame(int, const VSFrameRef *)),
		this, SLOT(slotReceiveFrame(int, const VSFrameRef *)));

	m_encodeStartTime = hr_clock::now();

	for(int i = firstFrame; i <= lastFrame; ++i)
		m_pVapourSynthScriptProcessor->requestFrameAsync(i);
}

// END OF void CLIEncodeDialog::slotStartStopBenchmarkButtonPressed()
//==============================================================================

void CLIEncodeDialog::slotExecutableBrowseButtonPressed()
{
	QString applicationPath = QCoreApplication::applicationDirPath();
	QFileDialog fileDialog;
	fileDialog.setWindowTitle(trUtf8("Choose encoder executable"));
	fileDialog.setDirectory(applicationPath);

#ifdef Q_OS_WIN
	fileDialog.setNameFilter("*.exe");
#endif

	if(!fileDialog.exec())
		return;

	QStringList filesList = fileDialog.selectedFiles();
	m_ui.executablePathEdit->setText(filesList[0]);
}

// END OF void CLIEncodeDialog::slotExecutableBrowseButtonPressed()
//==============================================================================

void CLIEncodeDialog::slotArgumentsHelpButtonPressed()
{
	QString argumentsHelpString = trUtf8("Use following placeholders:\n"
		"%w - video width\n"
		"%h - video height\n"
		"%fpsnum - video framerate numerator\n"
		"%fpsden - video framerate denominator\n"
		"%fps - video framerate as fraction\n"
		"%bits - video colour bitdepth");
	QString title = trUtf8("Encoder arguments");
	QMessageBox::information(this, title, argumentsHelpString);
}

// END OF void CLIEncodeDialog::slotArgumentsHelpButtonPressed()
//==============================================================================


void CLIEncodeDialog::slotReceiveFrame(int a_frameNumber,
	const VSFrameRef * a_cpFrameRef)
{
	if(!m_processing)
		return;

	const VSAPI * cpVSAPI = m_pVapourSynthScriptProcessor->api();
	assert(cpVSAPI);
	const VSVideoInfo * cpVideoInfo =
		m_pVapourSynthScriptProcessor->videoInfo();
	assert(cpVideoInfo);
	const VSFormat * cpFormat = cpVideoInfo->format;
	assert(cpFormat);

	const VSFrameRef * cpFrameRef = cpVSAPI->cloneFrameRef(a_cpFrameRef);
	NumberedFrameRef newFrame(a_frameNumber, cpFrameRef);
	m_framesQueue.insert(std::upper_bound(m_framesQueue.begin(),
		m_framesQueue.end(), newFrame), newFrame);

	while((!m_framesQueue.empty()) &&
		(m_framesQueue.front().number == (m_lastFrameProcessed + 1)))
	{
		cpFrameRef = m_framesQueue.front().cpFrameRef;
		size_t currentDataSize = 0;

		for(int i = 0; i < cpFormat->numPlanes; ++i)
		{
			const uint8_t * cpPlane = cpVSAPI->getReadPtr(cpFrameRef, i);
			int stride = cpVSAPI->getStride(cpFrameRef, i);
			int width = cpVSAPI->getFrameWidth(cpFrameRef, i);
			int height = cpVSAPI->getFrameHeight(cpFrameRef, i);
			int bytes = cpFormat->bytesPerSample;

			size_t planeSize = width * bytes * height;
			size_t neededFramebufferSize = currentDataSize + planeSize;
			if(neededFramebufferSize > m_framebuffer.size())
				m_framebuffer.resize(neededFramebufferSize);
			size_t framebufferStride = width * bytes;

			vs_bitblt(m_framebuffer.data() + currentDataSize, framebufferStride,
				cpPlane, stride, framebufferStride, height);

			currentDataSize += planeSize;
		}

		m_encoder.write(m_framebuffer.data(), (qint64)currentDataSize);
		m_encoder.waitForBytesWritten(-1);

		hr_time_point now = hr_clock::now();

		m_lastFrameProcessed = m_framesQueue.front().number;
		cpVSAPI->freeFrame(cpFrameRef);
		m_framesQueue.pop_front();

		m_framesProcessed++;
		m_ui.processingProgressBar->setValue(m_framesProcessed);
		double passed = duration_to_double(now - m_encodeStartTime);
		QString passedString = vsedit::timeToString(passed);
		double fps = (double)m_framesProcessed / passed;
		QString text = trUtf8("Time elapsed: %1 - %2 FPS")
			.arg(passedString).arg(QString::number(fps, 'f', 20));
		m_ui.metricsEdit->setText(text);
	}

	outputStandardError();

	if(m_framesProcessed == m_framesTotal)
		stopProcessing();
}

// END OF void CLIEncodeDialog::slotReceiveFrame(int a_frameNumber,
//		const VSFrameRef * a_cpFrameRef)
//==============================================================================

void CLIEncodeDialog::stopProcessing()
{
	if(!m_processing)
		return;

	m_processing = false;
	m_pVapourSynthScriptProcessor->flushFrameTicketsQueueForConsumer();
	disconnect(m_pVapourSynthScriptProcessor,
		SIGNAL(signalDistributeFrame(int, const VSFrameRef *)),
		this, SLOT(slotReceiveFrame(int, const VSFrameRef *)));
	m_ui.startStopBenchmarkButton->setText(trUtf8("Start"));

	m_encoder.closeWriteChannel();
    if(!m_encoder.waitForFinished())
        m_ui.feedbackTextEdit->appendPlainText(
			trUtf8("\nCouldn't close the encoder."));
	outputStandardError();

	m_framebuffer.clear();
	clearFramesQueue();
}

// END OF void CLIEncodeDialog::stopProcessing()
//==============================================================================

QString CLIEncodeDialog::decodeArguments(const QString & a_arguments)
{
	QString decodedString = a_arguments.simplified();

	const VSVideoInfo * cpVideoInfo =
		m_pVapourSynthScriptProcessor->videoInfo();
	assert(cpVideoInfo);
	const VSFormat * cpFormat = cpVideoInfo->format;
	assert(cpFormat);

	double fps = 0.0;
	if(cpVideoInfo->fpsDen > 0)
		fps = (double)cpVideoInfo->fpsNum / (double)cpVideoInfo->fpsDen;

	struct ReplacePair
	{
		QString from;
		QString to;
	};

	ReplacePair replaceList[] =
	{
		{"%w", QString::number(cpVideoInfo->width)},
		{"%h", QString::number(cpVideoInfo->height)},
		{"%fpsnum", QString::number(cpVideoInfo->fpsNum)},
		{"%fpsden", QString::number(cpVideoInfo->fpsDen)},
		{"%fps", QString::number(fps, 'f', 10)},
		{"%bits", QString::number(cpFormat->bitsPerSample)},
	};

	for(const ReplacePair & record : replaceList)
		decodedString = decodedString.replace(record.from, record.to);

	return decodedString;
}

// END OF void QString CLIEncodeDialog::decodeArguments(
//		const QString & a_arguments)
//==============================================================================

void CLIEncodeDialog::clearFramesQueue()
{
	const VSAPI * cpVSAPI = m_pVapourSynthScriptProcessor->api();
	assert(cpVSAPI);

	for(NumberedFrameRef & ref : m_framesQueue)
		cpVSAPI->freeFrame(ref.cpFrameRef);

	m_framesQueue.clear();
}

// END OF void CLIEncodeDialog::clearFramesQueue()
//==============================================================================

void CLIEncodeDialog::outputStandardError()
{
	QByteArray standardError = m_encoder.readAllStandardError();
	QString standardErrorText = QString::fromUtf8(standardError);
	standardErrorText = standardErrorText.trimmed();
	if(!standardErrorText.isEmpty())
		m_ui.feedbackTextEdit->appendPlainText(standardErrorText);
}

// END OF void CLIEncodeDialog::clearFramesQueue()
//==============================================================================
