#include "settings_definitions_core.h"

#include <QObject>
#include <map>

//==============================================================================

const ResamplingFilter DEFAULT_CHROMA_RESAMPLING_FILTER =
	ResamplingFilter::Spline16;
const YuvMatrixCoefficients DEFAULT_YUV_MATRIX_COEFFICIENTS =
	YuvMatrixCoefficients::m709;
const ChromaPlacement DEFAULT_CHROMA_PLACEMENT = ChromaPlacement::MPEG2;
const double DEFAULT_BICUBIC_FILTER_PARAMETER_B = 1.0 / 3.0;
const double DEFAULT_BICUBIC_FILTER_PARAMETER_C = 1.0 / 3.0;
const int DEFAULT_LANCZOS_FILTER_TAPS = 3;
const EncodingType DEFAULT_ENCODING_TYPE = EncodingType::CLI;
const EncodingHeaderType DEFAULT_ENCODING_HEADER_TYPE =
	EncodingHeaderType::NoHeader;
const JobType DEFAULT_JOB_TYPE = JobType::EncodeScriptCLI;
const JobState DEFAULT_JOB_STATE = JobState::Waiting;
const int DEFAULT_JOB_FIRST_FRAME = -1;
const int DEFAULT_JOB_LAST_FRAME = -1;
const int DEFAULT_JOB_FRAMES_PROCESSED = 0;
const double DEFAULT_JOB_FPS = 0.0;

//==============================================================================

const std::vector<JobState> ACTIVE_JOB_STATES = {JobState::Running,
	JobState::Pausing, JobState::Paused, JobState::Aborting,
	JobState::FailedCleanUp, JobState::CompletedCleanUp};

JobProperties::JobProperties():
	  id(QUuid::createUuid())
	, type(JobType::EncodeScriptCLI)
	, jobState(JobState::Waiting)
	, encodingType(EncodingType::CLI)
	, encodingHeaderType(EncodingHeaderType::NoHeader)
	, firstFrame(-1)
	, firstFrameReal(-1)
	, lastFrame(-1)
	, lastFrameReal(-1)
	, framesProcessed(0)
	, fps(0.0)
{
}

QString JobProperties::typeName(JobType a_type)
{
	static std::map<JobType, QString> typeNameMap =
	{
		{JobType::EncodeScriptCLI, QObject::trUtf8("CLI encoding")},
		{JobType::RunProcess, QObject::trUtf8("Process run")},
		{JobType::RunShellCommand, QObject::trUtf8("Shell command")},
	};

	return typeNameMap[a_type];
}

QString JobProperties::stateName(JobState a_state)
{
	static std::map<JobState, QString> stateNameMap =
	{
		{JobState::Waiting, QObject::trUtf8("Waiting")},
		{JobState::Running, QObject::trUtf8("Running")},
		{JobState::Paused, QObject::trUtf8("Paused")},
		{JobState::Pausing, QObject::trUtf8("Pausing")},
		{JobState::Aborted, QObject::trUtf8("Aborted")},
		{JobState::Aborting, QObject::trUtf8("Aborting")},
		{JobState::FailedCleanUp, QObject::trUtf8("Failed. Cleaning up.")},
		{JobState::Failed, QObject::trUtf8("Failed")},
		{JobState::DependencyNotMet, QObject::trUtf8("Dependency not met")},
		{JobState::CompletedCleanUp, QObject::trUtf8("Completing")},
		{JobState::Completed, QObject::trUtf8("Completed")},
	};

	return stateNameMap[a_state];
}

QString JobProperties::subject() const
{
	QString subjectString;

	if(type == JobType::EncodeScriptCLI)
	{
		subjectString = QString("%sn%:\n\"%ep%\" %arg%");
		subjectString = subjectString.replace("%sn%", scriptName);
		subjectString = subjectString.replace("%ep%", executablePath);
		subjectString = subjectString.replace("%arg%", arguments);
	}
	else if(type == JobType::RunProcess)
	{
		subjectString = QString("\"%ep%\" %arg%");
		subjectString = subjectString.replace("%ep%", executablePath);
		subjectString = subjectString.replace("%arg%", arguments.simplified());
	}
	else if(type == JobType::RunShellCommand)
		subjectString = shellCommand.simplified();

	return subjectString;
}

int JobProperties::framesTotal() const
{
	return lastFrameReal - firstFrameReal + 1;
}

//==============================================================================

EncodingPreset::EncodingPreset(const QString & a_name):
	  name(a_name)
	, type(DEFAULT_ENCODING_TYPE)
	, headerType(DEFAULT_ENCODING_HEADER_TYPE)
{
}

bool EncodingPreset::operator==(const EncodingPreset & a_other) const
{
	return (name == a_other.name);
}

bool EncodingPreset::operator<(const EncodingPreset & a_other) const
{
	return (name < a_other.name);
}

bool EncodingPreset::isEmpty() const
{
	return name.isEmpty();
}

//==============================================================================
