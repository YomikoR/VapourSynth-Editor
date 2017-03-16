#include "job.h"

#include "../common/helpers.h"
#include "../settings/settings_manager.h"
#include "../vapoursynth/vs_script_library.h"
#include "../vapoursynth/vapoursynth_script_processor.h"
#include "../frame_consumers/frame_header_writers/frame_header_writer_null.h"
#include "../frame_consumers/frame_header_writers/frame_header_writer_y4m.h"

#include <QFileInfo>
#include <cassert>
#include <algorithm>

//==============================================================================

vsedit::Job::Job(SettingsManager * a_pSettingsManager,
	VSScriptLibrary * a_pVSScriptLibrary, QObject * a_pParent) :
	  QObject(a_pParent)
	, m_type(JobType::EncodeScriptCLI)
	, m_jobState(JobState::Waiting)
	, m_encodingType(EncodingType::CLI)
	, m_encodingHeaderType(EncodingHeaderType::NoHeader)
	, m_firstFrame(-1)
	, m_lastFrame(-1)
	, m_framesProcessed(-1)
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
	assert(m_pSettingsManager);
	assert(m_pVSScriptLibrary);
	m_id = QUuid::createUuid();
	fillVariables();
	m_cpVSAPI = m_pVSScriptLibrary->getVSAPI();
	assert(m_cpVSAPI);
}

// END OF
//==============================================================================

vsedit::Job::~Job()
{
}

// END OF
//==============================================================================

void vsedit::Job::start()
{
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
}

// END OF
//==============================================================================

QUuid vsedit::Job::id() const
{
	return m_id;
}

// END OF
//==============================================================================

bool vsedit::Job::setId(const QUuid & a_id)
{
	m_id = a_id;
	return true;
}

// END OF
//==============================================================================

JobType vsedit::Job::type() const
{
	return m_type;
}

// END OF
//==============================================================================

bool vsedit::Job::setType(JobType a_type)
{
	m_type = a_type;
	return true;
}

// END OF
//==============================================================================

JobState vsedit::Job::state() const
{
	return m_jobState;
}

// END OF
//==============================================================================

bool vsedit::Job::setState(JobState a_state)
{
	m_jobState = a_state;
	return true;
}

// END OF
//==============================================================================

std::vector<QUuid> vsedit::Job::dependsOnJobIds() const
{
	return m_dependsOnJobIds;
}

// END OF
//==============================================================================

bool vsedit::Job::setDependsOnJobIds(const std::vector<QUuid> & a_ids)
{
	m_dependsOnJobIds = a_ids;
	return true;
}

// END OF
//==============================================================================

QString vsedit::Job::subject() const
{
	if(m_type == JobType::EncodeScriptCLI)
		return m_scriptName;
	else if(m_type == JobType::RunProcess)
		return m_executablePath;
	else if(m_type == JobType::RunShellCommand)
		return m_shellCommand;
	else
		return "-";
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
		{JobState::Aborted, trUtf8("Aborted")},
		{JobState::Failed, trUtf8("Failed")},
		{JobState::DependencyNotMet, trUtf8("Dependency not met")},
		{JobState::Completed, trUtf8("Completed")},
	};

	return stateNameMap[a_state];
}

// END OF
//==============================================================================

int vsedit::Job::framesTotal()
{
	return (m_lastFrame - m_firstFrame + 1);
}

// END OF
//==============================================================================

void vsedit::Job::fillVariables()
{
	m_variables =
	{
		{"{w}", trUtf8("video width"),
			[&]()
			{
				return QString::number(m_cpVideoInfo->width);
			}
		},

		{"{h}", trUtf8("video height"),
			[&]()
			{
				return QString::number(m_cpVideoInfo->height);
			}
		},

		{"{fpsn}", trUtf8("video framerate numerator"),
			[&]()
			{
				return QString::number(m_cpVideoInfo->fpsNum);
			}
		},

		{"{fpsd}", trUtf8("video framerate denominator"),
			[&]()
			{
				return QString::number(m_cpVideoInfo->fpsDen);
			}
		},

		{"{fps}", trUtf8("video framerate as fraction"),
			[&]()
			{
				double fps = (double)m_cpVideoInfo->fpsNum /
					(double)m_cpVideoInfo->fpsDen;
				return QString::number(fps, 'f', 10);
			}
		},

		{"{bits}", trUtf8("video colour bitdepth"),
			[&]()
			{
				return QString::number(m_cpVideoInfo->format->bitsPerSample);
			}
		},

		{"{sd}", trUtf8("script directory"),
			[&]()
			{
				QFileInfo scriptFile(m_scriptName);
				return scriptFile.canonicalPath();
			}
		},

		{"{sn}", trUtf8("script name without extension"),
			[&]()
			{
				QFileInfo scriptFile(m_scriptName);
				return scriptFile.completeBaseName();
			}
		},

		{"{f}", trUtf8("total frames number"),
			[&]()
			{
				return QString::number(framesTotal());
			}
		},

		{"{ss}", trUtf8("subsampling string (like 420)"),
			[&]()
			{
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
