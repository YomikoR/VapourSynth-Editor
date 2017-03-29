#include "job.h"

#include "../common/helpers.h"
#include "../settings/settings_manager.h"
#include "../vapoursynth/vs_script_library.h"
#include "../vapoursynth/vapoursynth_script_processor.h"
#include "../frame_consumers/frame_header_writers/frame_header_writer_null.h"
#include "../frame_consumers/frame_header_writers/frame_header_writer_y4m.h"

#include <QFileInfo>
#include <QFile>
#include <cassert>
#include <algorithm>

//==============================================================================

vsedit::Job::Job(const JobProperties & a_properties,
	SettingsManager * a_pSettingsManager,
	VSScriptLibrary * a_pVSScriptLibrary,
	QObject * a_pParent) :
	  QObject(a_pParent)
	, m_properties(a_properties)
	, m_lastFrameProcessed(-1)
	, m_lastFrameRequested(-1)
	, m_encodingState(EncodingState::Idle)
	, m_bytesToWrite(0u)
	, m_bytesWritten(0u)
	, m_pSettingsManager(a_pSettingsManager)
	, m_pVSScriptLibrary(a_pVSScriptLibrary)
	, m_pVapourSynthScriptProcessor(nullptr)
	, m_cpVSAPI(nullptr)
	, m_cpVideoInfo(nullptr)
	, m_pFrameHeaderWriter(nullptr)
	, m_cachedFramesLimit(100)
{
	fillVariables();
	if(a_pVSScriptLibrary)
		m_cpVSAPI = m_pVSScriptLibrary->getVSAPI();

	connect(&m_process, SIGNAL(started()),
		this, SLOT(slotProcessStarted()));
	connect(&m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
		this, SLOT(slotProcessFinished(int, QProcess::ExitStatus)));
	connect(&m_process, SIGNAL(error(QProcess::ProcessError)),
		this, SLOT(slotProcessError(QProcess::ProcessError)));
	connect(&m_process, SIGNAL(readChannelFinished()),
		this, SLOT(slotProcessReadChannelFinished()));
	connect(&m_process, SIGNAL(bytesWritten(qint64)),
		this, SLOT(slotProcessBytesWritten(qint64)));
	connect(&m_process, SIGNAL(readyReadStandardError()),
		this, SLOT(slotProcessReadyReadStandardError()));
}

// END OF
//==============================================================================

vsedit::Job::~Job()
{
}

// END OF
//==============================================================================

QString vsedit::Job::typeName(JobType a_type)
{
	static std::map<JobType, QString> typeNameMap =
	{
		{JobType::EncodeScriptCLI, trUtf8("CLI encoding")},
		{JobType::RunProcess, trUtf8("Process run")},
		{JobType::RunShellCommand, trUtf8("Shell command")},
	};

	return typeNameMap[a_type];
}

// END OF
//==============================================================================

QString vsedit::Job::stateName(JobState a_state)
{
	static std::map<JobState, QString> stateNameMap =
	{
		{JobState::Waiting, trUtf8("Waiting")},
		{JobState::Running, trUtf8("Running")},
		{JobState::Paused, trUtf8("Paused")},
		{JobState::Paused, trUtf8("Pausing")},
		{JobState::Aborted, trUtf8("Aborted")},
		{JobState::Aborting, trUtf8("Aborting")},
		{JobState::Failed, trUtf8("Failed")},
		{JobState::DependencyNotMet, trUtf8("Dependency not met")},
		{JobState::Completed, trUtf8("Completed")},
	};

	return stateNameMap[a_state];
}

// END OF
//==============================================================================

void vsedit::Job::start()
{
	if(m_properties.type == JobType::EncodeScriptCLI)
		startEncodeScriptCLI();
	else if(m_properties.type == JobType::RunProcess)
		startRunProcess();
	else if(m_properties.type == JobType::RunShellCommand)
		startRunShellCommand();
}

// END OF
//==============================================================================

void vsedit::Job::pause()
{
}

// END OF
//==============================================================================

void vsedit::Job::abort()
{
	changeStateAndNotify(JobState::Aborting);
	if(m_properties.type == JobType::EncodeScriptCLI)
	{
		m_encodingState = EncodingState::Aborting;
		cleanUpEncoding();
		return;
	}
	else if(m_properties.type == JobType::RunProcess)
	{
		if(m_process.state() == QProcess::Running)
		{
			m_process.kill();
			m_process.waitForFinished(-1);
		}
	}

	changeStateAndNotify(JobState::Aborted);
}

// END OF
//==============================================================================

QUuid vsedit::Job::id() const
{
	return m_properties.id;
}

// END OF
//==============================================================================

bool vsedit::Job::setId(const QUuid & a_id)
{
	m_properties.id = a_id;
	return true;
}

// END OF
//==============================================================================

JobType vsedit::Job::type() const
{
	return m_properties.type;
}

// END OF
//==============================================================================

bool vsedit::Job::setType(JobType a_type)
{
	m_properties.type = a_type;
	return true;
}

// END OF
//==============================================================================

QString vsedit::Job::scriptName() const
{
	return m_properties.scriptName;
}

// END OF
//==============================================================================

bool vsedit::Job::setScriptName(const QString & a_scriptName)
{
	m_properties.scriptName = a_scriptName;
	return true;
}

// END OF
//==============================================================================

EncodingHeaderType vsedit::Job::encodingHeaderType() const
{
	return m_properties.encodingHeaderType;
}

// END OF
//==============================================================================

bool vsedit::Job::setEncodingHeaderType(EncodingHeaderType a_headerType)
{
	m_properties.encodingHeaderType = a_headerType;
	return true;
}

// END OF
//==============================================================================

QString vsedit::Job::executablePath() const
{
	return m_properties.executablePath;
}

// END OF
//==============================================================================

bool vsedit::Job::setExecutablePath(const QString & a_path)
{
	m_properties.executablePath = a_path;
	return true;
}

// END OF
//==============================================================================

QString vsedit::Job::arguments() const
{
	return decodeArguments(m_properties.arguments);
}

// END OF
//==============================================================================

bool vsedit::Job::setArguments(const QString & a_arguments)
{
	m_properties.arguments = a_arguments;
	return true;
}

// END OF
//==============================================================================

QString vsedit::Job::shellCommand() const
{
	return m_properties.shellCommand;
}

// END OF
//==============================================================================

bool vsedit::Job::setShellCommand(const QString & a_command)
{
	m_properties.shellCommand = a_command;
	return true;
}

// END OF
//==============================================================================

JobState vsedit::Job::state() const
{
	return m_properties.jobState;
}

// END OF
//==============================================================================

bool vsedit::Job::setState(JobState a_state)
{
	m_properties.jobState = a_state;
	return true;
}

// END OF
//==============================================================================

std::vector<QUuid> vsedit::Job::dependsOnJobIds() const
{
	return m_properties.dependsOnJobIds;
}

// END OF
//==============================================================================

bool vsedit::Job::setDependsOnJobIds(const std::vector<QUuid> & a_ids)
{
	m_properties.dependsOnJobIds = a_ids;
	return true;
}

// END OF
//==============================================================================

QString vsedit::Job::subject() const
{
	QString subjectString;

	if(m_properties.type == JobType::EncodeScriptCLI)
	{
		subjectString = QString("%sn%:\n\"%ep%\" %arg%");
		subjectString = subjectString.replace("%sn%", m_properties.scriptName);
		subjectString = subjectString.replace("%ep%",
			m_properties.executablePath);
		subjectString = subjectString.replace("%arg%",
			decodeArguments(m_properties.arguments));
	}
	else if(m_properties.type == JobType::RunProcess)
	{
		subjectString = QString("\"%ep%\" %arg%");
		subjectString = subjectString.replace("%ep%",
			m_properties.executablePath);
		subjectString = subjectString.replace("%arg%",
			m_properties.arguments.simplified());
	}
	else if(m_properties.type == JobType::RunShellCommand)
		subjectString = m_properties.shellCommand.simplified();

	return subjectString;
}

// END OF
//==============================================================================

std::vector<vsedit::VariableToken> vsedit::Job::variables() const
{
	std::vector<vsedit::VariableToken> cutVariables;
	for(const vsedit::VariableToken & variable : m_variables)
	{
		vsedit::VariableToken cutVariable =
			{variable.token, variable.description, std::function<QString()>()};
		cutVariables.push_back(cutVariable);
	}
	return cutVariables;
}

// END OF
//==============================================================================

int vsedit::Job::framesProcessed() const
{
	if(m_properties.type == JobType::EncodeScriptCLI)
		return m_properties.framesProcessed;
	return 0;
}

// END OF
//==============================================================================

int vsedit::Job::framesTotal() const
{
	if(m_properties.type == JobType::EncodeScriptCLI)
		return (m_properties.lastFrame - m_properties.firstFrame + 1);
	return 0;
}

// END OF
//==============================================================================

JobProperties vsedit::Job::properties() const
{
	return m_properties;
}

// END OF
//==============================================================================

bool vsedit::Job::setProperties(const JobProperties & a_properties)
{
	// TODO: sanity checks
	m_properties = a_properties;
	return true;
}

// END OF
//==============================================================================

void vsedit::Job::slotProcessStarted()
{
}

// END OF
//==============================================================================

void vsedit::Job::slotProcessFinished(int a_exitCode,
	QProcess::ExitStatus a_exitStatus)
{
}

// END OF
//==============================================================================

void vsedit::Job::slotProcessError(QProcess::ProcessError a_error)
{
}

// END OF
//==============================================================================

void vsedit::Job::slotProcessReadChannelFinished()
{
}

// END OF
//==============================================================================

void vsedit::Job::slotProcessBytesWritten(qint64 a_bytes)
{
}

// END OF
//==============================================================================

void vsedit::Job::slotProcessReadyReadStandardError()
{
}

// END OF
//==============================================================================

void vsedit::Job::slotWriteLogMessage(int a_messageType,
	const QString & a_message)
{
	QString style = vsMessageTypeToStyleName(a_messageType);
	emit signalLogMessage(a_message, style);
}

// END OF
//==============================================================================

void vsedit::Job::slotFrameQueueStateChanged(size_t a_inQueue,
	size_t a_inProcess, size_t a_maxThreads)
{

}

// END OF
//==============================================================================

void vsedit::Job::slotCoreFramebufferUsedBytes(int64_t a_bytes)
{

}

// END OF
//==============================================================================

void vsedit::Job::slotReceiveFrame(int a_frameNumber, int a_outputIndex,
	const VSFrameRef * a_cpOutputFrameRef,
	const VSFrameRef * a_cpPreviewFrameRef)
{

}

// END OF
//==============================================================================

void vsedit::Job::slotFrameRequestDiscarded(int a_frameNumber,
	int a_outputIndex, const QString & a_reason)
{

}

// END OF
//==============================================================================

void vsedit::Job::fillVariables()
{
	const char TOKEN_WIDTH[] = "{w}";
	const char TOKEN_HEIGHT[] = "{h}";
	const char TOKEN_FPS_NUMERATOR[] = "{fpsn}";
	const char TOKEN_FPS_DENOMINATOR[] = "{fpsd}";
	const char TOKEN_FPS[] = "{fps}";
	const char TOKEN_BITDEPTH[] = "{bits}";
	const char TOKEN_SCRIPT_DIRECTORY[] = "{sd}";
	const char TOKEN_SCRIPT_NAME[] = "{sn}";
	const char TOKEN_FRAMES_NUMBER[] = "{f}";
	const char TOKEN_SUBSAMPLING[] = "{ss}";

	m_variables =
	{
		{TOKEN_WIDTH, trUtf8("video width"),
			[&]() -> QString
			{
				if(!m_cpVideoInfo)
					return TOKEN_WIDTH;
				return QString::number(m_cpVideoInfo->width);
			}
		},

		{TOKEN_HEIGHT, trUtf8("video height"),
			[&]() -> QString
			{
				if(!m_cpVideoInfo)
					return TOKEN_HEIGHT;
				return QString::number(m_cpVideoInfo->height);
			}
		},

		{TOKEN_FPS_NUMERATOR, trUtf8("video framerate numerator"),
			[&]() -> QString
			{
				if(!m_cpVideoInfo)
					return TOKEN_FPS_NUMERATOR;
				return QString::number(m_cpVideoInfo->fpsNum);
			}
		},

		{TOKEN_FPS_DENOMINATOR, trUtf8("video framerate denominator"),
			[&]() -> QString
			{
				if(!m_cpVideoInfo)
					return TOKEN_FPS_DENOMINATOR;
				return QString::number(m_cpVideoInfo->fpsDen);
			}
		},

		{TOKEN_FPS, trUtf8("video framerate as fraction"),
			[&]() -> QString
			{
				if(!m_cpVideoInfo)
					return TOKEN_FPS;
				double fps = (double)m_cpVideoInfo->fpsNum /
					(double)m_cpVideoInfo->fpsDen;
				return QString::number(fps, 'f', 10);
			}
		},

		{TOKEN_BITDEPTH, trUtf8("video colour bitdepth"),
			[&]() -> QString
			{
				if(!m_cpVideoInfo)
					return TOKEN_BITDEPTH;
				return QString::number(m_cpVideoInfo->format->bitsPerSample);
			}
		},

		{TOKEN_SCRIPT_DIRECTORY, trUtf8("script directory"),
			[&]() -> QString
			{
				QFileInfo scriptFile(m_properties.scriptName);
				return scriptFile.canonicalPath();
			}
		},

		{TOKEN_SCRIPT_NAME, trUtf8("script name without extension"),
			[&]() -> QString
			{
				QFileInfo scriptFile(m_properties.scriptName);
				return scriptFile.completeBaseName();
			}
		},

		{TOKEN_FRAMES_NUMBER, trUtf8("total frames number"),
			[&]() -> QString
			{
				return QString::number(framesTotal());
			}
		},

		{TOKEN_SUBSAMPLING, trUtf8("subsampling string (like 420)"),
			[&]() -> QString
			{
				if(!m_cpVideoInfo)
					return TOKEN_SUBSAMPLING;
				const VSFormat * cpFormat = m_cpVideoInfo->format;
				if(!cpFormat)
					return QString();
				return vsedit::subsamplingString(cpFormat->subSamplingW,
					cpFormat->subSamplingH);
			}
		},
	};

	std::sort(m_variables.begin(), m_variables.end(),
		[&](const vsedit::VariableToken & a_first,
			const vsedit::VariableToken & a_second) -> bool
		{
			return (a_first.token.length() > a_second.token.length());
		});
}

// END OF
//==============================================================================

void vsedit::Job::changeStateAndNotify(JobState a_state)
{
	JobState oldState = m_properties.jobState;
	m_properties.jobState = a_state;
	emit signalStateChanged(m_properties.jobState, oldState);
}

// END OF
//==============================================================================

void vsedit::Job::startEncodeScriptCLI()
{
	QString absoluteScriptPath =
		resolvePathFromApplication(m_properties.scriptName);
	QFile scriptFile(absoluteScriptPath);
	bool opened = scriptFile.open(QIODevice::ReadOnly);
	if(!opened)
	{
		emit signalLogMessage(trUtf8("Could not open script file \"%1\".")
			.arg(m_properties.scriptName), LOG_STYLE_ERROR);
		changeStateAndNotify(JobState::Failed);
		return;
	}

	QString script = QString::fromUtf8(scriptFile.readAll());
	scriptFile.close();

	if((!m_pVSScriptLibrary) || (!m_pSettingsManager))
	{
		emit signalLogMessage(trUtf8("Job is not porerly initialized."),
			LOG_STYLE_ERROR);
		changeStateAndNotify(JobState::Failed);
		return;
	}

	m_cpVSAPI = m_pVSScriptLibrary->getVSAPI();
	assert(m_cpVSAPI);

	if(!m_pVapourSynthScriptProcessor)
	{
		m_pVapourSynthScriptProcessor = new VapourSynthScriptProcessor(
			m_pSettingsManager, m_pVSScriptLibrary, this);
		connect(m_pVapourSynthScriptProcessor,
			SIGNAL(signalWriteLogMessage(int, const QString &)),
			this, SLOT(slotWriteLogMessage(int, const QString &)));
		connect(m_pVapourSynthScriptProcessor,
			SIGNAL(signalFrameQueueStateChanged(size_t, size_t, size_t)),
			this, SLOT(slotFrameQueueStateChanged(size_t, size_t, size_t)));
		connect(m_pVapourSynthScriptProcessor,
			SIGNAL(signalCoreFramebufferUsedBytes(int64_t)),
			this, SLOT(slotCoreFramebufferUsedBytes(int64_t)));
		connect(m_pVapourSynthScriptProcessor,
			SIGNAL(signalDistributeFrame(int, int, const VSFrameRef *,
				const VSFrameRef *)),
			this, SLOT(slotReceiveFrame(int, int, const VSFrameRef *,
				const VSFrameRef *)));
		connect(m_pVapourSynthScriptProcessor,
			SIGNAL(signalFrameRequestDiscarded(int, int, const QString &)),
			this, SLOT(slotFrameRequestDiscarded(int, int, const QString &)));
	}

	bool scriptProcessorInitialized = m_pVapourSynthScriptProcessor->initialize(
		script, m_properties.scriptName);
	if(!scriptProcessorInitialized)
	{
		emit signalLogMessage(trUtf8("Failed to initialize script.\n%1")
			.arg(m_pVapourSynthScriptProcessor->error()), LOG_STYLE_ERROR);
		changeStateAndNotify(JobState::Failed);
		return;
	}

	m_cpVideoInfo = m_pVapourSynthScriptProcessor->videoInfo();
	assert(m_cpVideoInfo);

	if(m_properties.firstFrame == -1)
		m_properties.firstFrame = 0;
	if(m_properties.lastFrame == -1)
		m_properties.lastFrame = m_cpVideoInfo->numFrames - 1;

	if(m_pFrameHeaderWriter)
		delete m_pFrameHeaderWriter;

	if(m_properties.encodingHeaderType == EncodingHeaderType::Y4M)
		m_pFrameHeaderWriter =
			new FrameHeaderWriterY4M(m_cpVSAPI, m_cpVideoInfo, this);
	else
		m_pFrameHeaderWriter =
			new FrameHeaderWriterNull(m_cpVSAPI, m_cpVideoInfo, this);

	bool compatibleHeader = m_pFrameHeaderWriter->isCompatible();
	if(!compatibleHeader)
	{
		emit signalLogMessage(trUtf8("Video is not compatible "
			"with the chosen header."), LOG_STYLE_ERROR);
		cleanUpEncoding();
		changeStateAndNotify(JobState::Failed);
		return;
	}

	QString executable = vsedit::resolvePathFromApplication(
		m_properties.executablePath);
	QString decodedArguments =
		decodeArguments(m_properties.arguments);
	QString commandLine = QString("\"%1\" %2").arg(executable)
		.arg(decodedArguments);

	emit signalLogMessage(trUtf8("Command line:"));
	emit signalLogMessage(commandLine);

	emit signalLogMessage(trUtf8("Checking the encoder sanity."));
	m_encodingState = EncodingState::CheckingEncoderSanity;

	m_process.start(commandLine);
	if(!m_process.waitForStarted(3000))
	{
		emit signalLogMessage(trUtf8("Encoder wouldn't start."),
			LOG_STYLE_ERROR);
		cleanUpEncoding();
		changeStateAndNotify(JobState::Failed);
		return;
	}

	m_process.closeWriteChannel();
	if(!m_process.waitForFinished(3000))
	{
		emit signalLogMessage(trUtf8("Program is not behaving "
			"like a CLI encoder. Terminating."), LOG_STYLE_ERROR);
		m_process.kill();
		m_process.waitForFinished(-1);
		cleanUpEncoding();
		changeStateAndNotify(JobState::Failed);
		return;
	}

	emit signalLogMessage(trUtf8("Encoder seems sane. Starting."));
	m_encodingState = EncodingState::StartingEncoder;
	m_process.start(commandLine);
}

// END OF
//==============================================================================

void vsedit::Job::startRunProcess()
{

}

// END OF
//==============================================================================

void vsedit::Job::startRunShellCommand()
{

}

// END OF
//==============================================================================

QString vsedit::Job::decodeArguments(const QString & a_arguments) const
{
	QString decodedString = a_arguments.simplified();

	for(const vsedit::VariableToken & variable : m_variables)
	{
		decodedString = decodedString.replace(variable.token,
			variable.evaluate());
	}

	return decodedString;
}

// END OF
//==============================================================================

void vsedit::Job::cleanUpEncoding()
{
	m_pVapourSynthScriptProcessor->flushFrameTicketsQueue();
	clearFramesCache();
	m_framebuffer.clear();

	if(m_process.state() == QProcess::Running)
	{
		if(m_encodingState != EncodingState::Aborting)
			m_encodingState = EncodingState::Finishing;
		m_process.closeWriteChannel();
	}

	m_cpVideoInfo = nullptr;
	m_pVapourSynthScriptProcessor->finalize();
}

// END OF
//==============================================================================

void vsedit::Job::clearFramesCache()
{
	if(m_framesCache.empty())
		return;

	assert(m_cpVSAPI);
	for(Frame & frame : m_framesCache)
	{
		m_cpVSAPI->freeFrame(frame.cpOutputFrameRef);
		m_cpVSAPI->freeFrame(frame.cpPreviewFrameRef);
	}
	m_framesCache.clear();
}

// END OF
//==============================================================================
