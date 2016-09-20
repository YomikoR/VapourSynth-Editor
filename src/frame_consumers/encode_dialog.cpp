#include "encode_dialog.h"

#include "../common/helpers.h"
#include "../vapoursynth/vapoursynthscriptprocessor.h"
#include "../settings/settingsdialog.h"

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

EncodeDialog::EncodeDialog(SettingsManager * a_pSettingsManager,
	QWidget * a_pParent) :
	VSScriptProcessorDialog(a_pSettingsManager, a_pParent, (Qt::WindowFlags)0
		| Qt::Window
		| Qt::CustomizeWindowHint
		| Qt::WindowMinimizeButtonHint
		| Qt::WindowCloseButtonHint
		)
	, m_processing(false)
	, m_framesTotal(0)
	, m_framesProcessed(0)
	, m_lastFrameProcessed(-1)
{
	m_ui.setupUi(this);
	setWindowIcon(QIcon(":film_save.png"));

	fillVariables();

	createStatusBar();

	m_ui.executableBrowseButton->setIcon(QIcon(":folder.png"));

	m_ui.argumentsHelpButton->setIcon(QIcon(":information.png"));

	connect(m_pVapourSynthScriptProcessor,
		SIGNAL(signalDistributeFrame(int, const VSFrameRef *)),
		this, SLOT(slotReceiveFrame(int, const VSFrameRef *)));
	connect(m_ui.wholeVideoButton, SIGNAL(clicked()),
		this, SLOT(slotWholeVideoButtonPressed()));
	connect(m_ui.startStopBenchmarkButton, SIGNAL(clicked()),
		this, SLOT(slotStartStopBenchmarkButtonPressed()));
	connect(m_ui.executableBrowseButton, SIGNAL(clicked()),
		this, SLOT(slotExecutableBrowseButtonPressed()));
	connect(m_ui.argumentsHelpButton, SIGNAL(clicked()),
		this, SLOT(slotArgumentsHelpButtonPressed()));
}

// END OF EncodeDialog::EncodeDialog(SettingsManager * a_pSettingsManager,
//		QWidget * a_pParent)
//==============================================================================

EncodeDialog::~EncodeDialog()
{
}

// END OF EncodeDialog::~EncodeDialog()
//==============================================================================

void EncodeDialog::call()
{
	if(m_processing)
	{
		show();
		return;
	}

	if((!m_pVapourSynthScriptProcessor->isInitialized()) || m_wantToFinalize)
		return;

	assert(m_cpVideoInfo);

	QString text = trUtf8("Ready to encode script %1").arg(m_scriptName);
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

// END OF void EncodeDialog::call()
//==============================================================================

void EncodeDialog::slotWriteLogMessage(int a_messageType,
	const QString & a_message)
{
	m_ui.feedbackTextEdit->appendPlainText(a_message);
}

// END OF void EncodeDialog::slotWriteLogMessage(int a_messageType,
//		const QString & a_message)
//==============================================================================

void EncodeDialog::slotWholeVideoButtonPressed()
{
	assert(m_cpVideoInfo);
	int lastFrame = m_cpVideoInfo->numFrames - 1;
	m_ui.fromFrameSpinBox->setValue(0);
	m_ui.toFrameSpinBox->setValue(lastFrame);
}

// END OF void EncodeDialog::slotWholeVideoButtonPressed()
//==============================================================================

void EncodeDialog::slotStartStopBenchmarkButtonPressed()
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

	m_encodeStartTime = hr_clock::now();

	for(int i = firstFrame; i <= lastFrame; ++i)
		m_pVapourSynthScriptProcessor->requestFrameAsync(i);
}

// END OF void EncodeDialog::slotStartStopBenchmarkButtonPressed()
//==============================================================================

void EncodeDialog::slotExecutableBrowseButtonPressed()
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

// END OF void EncodeDialog::slotExecutableBrowseButtonPressed()
//==============================================================================

void EncodeDialog::slotArgumentsHelpButtonPressed()
{
	QString argumentsHelpString = trUtf8("Use following placeholders:");
	for(const VariableToken & variable : m_variables)
	{
		argumentsHelpString += QString("\n%1 - %2")
			.arg(variable.token).arg(variable.description);
	}
	QString title = trUtf8("Encoder arguments");
	QMessageBox::information(this, title, argumentsHelpString);
}

// END OF void EncodeDialog::slotArgumentsHelpButtonPressed()
//==============================================================================


void EncodeDialog::slotReceiveFrame(int a_frameNumber,
	const VSFrameRef * a_cpFrameRef)
{
	if(!m_processing)
		return;

	assert(m_cpVSAPI);
	assert(m_cpVideoInfo);
	const VSFormat * cpFormat = m_cpVideoInfo->format;
	assert(cpFormat);

	const VSFrameRef * cpFrameRef = m_cpVSAPI->cloneFrameRef(a_cpFrameRef);
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
			const uint8_t * cpPlane = m_cpVSAPI->getReadPtr(cpFrameRef, i);
			int stride = m_cpVSAPI->getStride(cpFrameRef, i);
			int width = m_cpVSAPI->getFrameWidth(cpFrameRef, i);
			int height = m_cpVSAPI->getFrameHeight(cpFrameRef, i);
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
		m_cpVSAPI->freeFrame(cpFrameRef);
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

// END OF void EncodeDialog::slotReceiveFrame(int a_frameNumber,
//		const VSFrameRef * a_cpFrameRef)
//==============================================================================

void EncodeDialog::stopAndCleanUp()
{
	stopProcessing();
	m_ui.metricsEdit->clear();
	m_ui.processingProgressBar->setValue(0);
}

// END OF void EncodeDialog::stopAndCleanUp()
//==============================================================================

void EncodeDialog::stopProcessing()
{
	if(!m_processing)
		return;

	m_processing = false;
	m_pVapourSynthScriptProcessor->flushFrameTicketsQueue();
	m_ui.startStopBenchmarkButton->setText(trUtf8("Start"));

	m_encoder.closeWriteChannel();
    if(!m_encoder.waitForFinished())
        m_ui.feedbackTextEdit->appendPlainText(
			trUtf8("\nCouldn't close the encoder."));
	outputStandardError();

	m_framebuffer.clear();
	clearFramesQueue();
}

// END OF void EncodeDialog::stopProcessing()
//==============================================================================

QString EncodeDialog::decodeArguments(const QString & a_arguments)
{
	QString decodedString = a_arguments.simplified();

	for(const VariableToken & variable : m_variables)
	{
		decodedString = decodedString.replace(variable.token,
			variable.evaluate());
	}

	return decodedString;
}

// END OF void QString EncodeDialog::decodeArguments(
//		const QString & a_arguments)
//==============================================================================

void EncodeDialog::clearFramesQueue()
{
	assert(m_cpVSAPI);

	for(NumberedFrameRef & ref : m_framesQueue)
		m_cpVSAPI->freeFrame(ref.cpFrameRef);

	m_framesQueue.clear();
}

// END OF void EncodeDialog::clearFramesQueue()
//==============================================================================

void EncodeDialog::outputStandardError()
{
	QByteArray standardError = m_encoder.readAllStandardError();
	QString standardErrorText = QString::fromUtf8(standardError);
	standardErrorText = standardErrorText.trimmed();
	if(!standardErrorText.isEmpty())
		m_ui.feedbackTextEdit->appendPlainText(standardErrorText);
}

// END OF void EncodeDialog::clearFramesQueue()
//==============================================================================

void EncodeDialog::fillVariables()
{
	m_variables =
	{
		{"%w", trUtf8("video width"),
			[&]()
			{
				return QString::number(m_cpVideoInfo->width);
			}
		},

		{"%h", trUtf8("video height"),
			[&]()
			{
				return QString::number(m_cpVideoInfo->height);
			}
		},

		{"%fpsn", trUtf8("video framerate numerator"),
			[&]()
			{
				return QString::number(m_cpVideoInfo->fpsNum);
			}
		},

		{"%fpsd", trUtf8("video framerate denominator"),
			[&]()
			{
				return QString::number(m_cpVideoInfo->fpsDen);
			}
		},

		{"%fps", trUtf8("video framerate as fraction"),
			[&]()
			{
				double fps = (double)m_cpVideoInfo->fpsNum /
					(double)m_cpVideoInfo->fpsDen;
				return QString::number(fps, 'f', 10);
			}
		},

		{"%bits", trUtf8("video colour bitdepth"),
			[&]()
			{
				return QString::number(m_cpVideoInfo->format->bitsPerSample);
			}
		},

		{"%sd", trUtf8("script directory"),
			[&]()
			{
				QFileInfo scriptFile(m_scriptName);
				return scriptFile.canonicalPath();
			}
		},

		{"%sn", trUtf8("script name without extension"),
			[&]()
			{
				QFileInfo scriptFile(m_scriptName);
				return scriptFile.completeBaseName();
			}
		},
	};
}

// END OF void EncodeDialog::fillVariables()
//==============================================================================
