#include "settings_definitions_core.h"

#include <QObject>
#include <QJsonArray>
#include <QVariant>
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
const int DEFAULT_RECENT_JOB_SERVERS_NUMBER = 10;
const int DEFAULT_WINDOW_GEOMETRY_SAVE_DELAY = 2000;

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
		{JobType::EncodeScriptCLI, QObject::tr("CLI encoding")},
		{JobType::RunProcess, QObject::tr("Process run")},
		{JobType::RunShellCommand, QObject::tr("Shell command")},
	};

	return typeNameMap[a_type];
}

QString JobProperties::stateName(JobState a_state)
{
	static std::map<JobState, QString> stateNameMap =
	{
		{JobState::Waiting, QObject::tr("Waiting")},
		{JobState::Running, QObject::tr("Running")},
		{JobState::Paused, QObject::tr("Paused")},
		{JobState::Pausing, QObject::tr("Pausing")},
		{JobState::Aborted, QObject::tr("Aborted")},
		{JobState::Aborting, QObject::tr("Aborting")},
		{JobState::FailedCleanUp, QObject::tr("Failed. Cleaning up.")},
		{JobState::Failed, QObject::tr("Failed")},
		{JobState::DependencyNotMet, QObject::tr("Dependency not met")},
		{JobState::CompletedCleanUp, QObject::tr("Completing")},
		{JobState::Completed, QObject::tr("Completed")},
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
		subjectString = subjectString.replace("%arg%", arguments);
	}
	else if(type == JobType::RunShellCommand)
		subjectString = shellCommand;

	subjectString = subjectString.simplified();

	return subjectString;
}

int JobProperties::framesTotal() const
{
	return lastFrameReal - firstFrameReal + 1;
}

const char JP_ID[] = "id";
const char JP_TYPE[] = "type";
const char JP_JOB_STATE[] = "jobState";
const char JP_DEPENDS_ON_JOB_IDS[] = "dependsOnJobIds";
const char JP_TIME_STARTED[] = "timeStarted";
const char JP_TIME_ENDED[] = "timeEnded";
const char JP_SCRIPT_NAME[] = "scriptName";
const char JP_SCRIPT_TEXT[] = "scriptText";
const char JP_ENCODING_TYPE[] = "encodingType";
const char JP_ENCODING_HEADER_TYPE[] = "encodingHeaderType";
const char JP_EXECUTABLE_PATH[] = "executablePath";
const char JP_ARGUMENTS[] = "arguments";
const char JP_SHELL_COMMAND[] = "shellCommand";
const char JP_FIRST_FRAME[] = "firstFrame";
const char JP_FIRST_FRAME_REAL[] = "firstFrameReal";
const char JP_LAST_FRAME[] = "lastFrame";
const char JP_LAST_FRAME_REAL[] = "lastFrameReal";
const char JP_FRAMES_PROCESSED[] = "framesProcessed";
const char JP_FPS[] = "fps";

QJsonObject JobProperties::toJson() const
{
	QJsonObject jsJob;
	jsJob[JP_ID] = id.toString();
	jsJob[JP_TYPE] = (int)type;
	jsJob[JP_JOB_STATE] = (int)jobState;

	QJsonArray jsDependencies;
	for(const QUuid & dependencyId : dependsOnJobIds)
		jsDependencies.push_back(QJsonValue(dependencyId.toString()));
	jsJob[JP_DEPENDS_ON_JOB_IDS] = jsDependencies;

	jsJob[JP_TIME_STARTED] = timeStarted.toMSecsSinceEpoch();
	jsJob[JP_TIME_ENDED] = timeEnded.toMSecsSinceEpoch();
	jsJob[JP_SCRIPT_NAME] = scriptName;
	jsJob[JP_SCRIPT_TEXT] = scriptText;
	jsJob[JP_ENCODING_TYPE] = (int)encodingType;
	jsJob[JP_ENCODING_HEADER_TYPE] = (int)encodingHeaderType;
	jsJob[JP_EXECUTABLE_PATH] = executablePath;
	jsJob[JP_ARGUMENTS] = arguments;
	jsJob[JP_SHELL_COMMAND] = shellCommand;
	jsJob[JP_FIRST_FRAME] = firstFrame;
	jsJob[JP_FIRST_FRAME_REAL] = firstFrameReal;
	jsJob[JP_LAST_FRAME] = lastFrame;
	jsJob[JP_LAST_FRAME_REAL] = lastFrameReal;
	jsJob[JP_FRAMES_PROCESSED] = framesProcessed;
	jsJob[JP_FPS] = fps;
	return jsJob;
}

JobProperties JobProperties::fromJson(const QJsonObject & a_object)
{
	JobProperties properties;
	if(a_object.contains(JP_ID))
		properties.id = QUuid(a_object[JP_ID].toString());
	if(a_object.contains(JP_TYPE))
		properties.type = (JobType)a_object[JP_TYPE].toInt();
	if(a_object.contains(JP_JOB_STATE))
		properties.jobState = (JobState)a_object[JP_JOB_STATE].toInt();
	if(a_object.contains(JP_DEPENDS_ON_JOB_IDS))
	{
		if(a_object[JP_DEPENDS_ON_JOB_IDS].isArray())
		{
			for(const QJsonValue & value :
				a_object[JP_DEPENDS_ON_JOB_IDS].toArray())
				properties.dependsOnJobIds.push_back(QUuid(value.toString()));
		}
	}
	if(a_object.contains(JP_TIME_STARTED))
		properties.timeStarted = QDateTime::fromMSecsSinceEpoch(
			a_object[JP_TIME_STARTED].toVariant().toLongLong());
	if(a_object.contains(JP_TIME_ENDED))
		properties.timeEnded = QDateTime::fromMSecsSinceEpoch(
			a_object[JP_TIME_ENDED].toVariant().toLongLong());
	if(a_object.contains(JP_SCRIPT_NAME))
		properties.scriptName = a_object[JP_SCRIPT_NAME].toString();
	if(a_object.contains(JP_SCRIPT_TEXT))
		properties.scriptText = a_object[JP_SCRIPT_TEXT].toString();
	if(a_object.contains(JP_ENCODING_TYPE))
		properties.encodingType =
			(EncodingType)a_object[JP_ENCODING_TYPE].toInt();
	if(a_object.contains(JP_ENCODING_HEADER_TYPE))
		properties.encodingHeaderType =
			(EncodingHeaderType)a_object[JP_ENCODING_HEADER_TYPE].toInt();
	if(a_object.contains(JP_EXECUTABLE_PATH))
		properties.executablePath = a_object[JP_EXECUTABLE_PATH].toString();
	if(a_object.contains(JP_ARGUMENTS))
		properties.arguments = a_object[JP_ARGUMENTS].toString();
	if(a_object.contains(JP_SHELL_COMMAND))
		properties.shellCommand = a_object[JP_SHELL_COMMAND].toString();
	if(a_object.contains(JP_FIRST_FRAME))
		properties.firstFrame = a_object[JP_FIRST_FRAME].toInt();
	if(a_object.contains(JP_FIRST_FRAME_REAL))
		properties.firstFrameReal = a_object[JP_FIRST_FRAME_REAL].toInt();
	if(a_object.contains(JP_LAST_FRAME))
		properties.lastFrame = a_object[JP_LAST_FRAME].toInt();
	if(a_object.contains(JP_LAST_FRAME_REAL))
		properties.lastFrameReal= a_object[JP_LAST_FRAME_REAL].toInt();
	if(a_object.contains(JP_FRAMES_PROCESSED))
		properties.framesProcessed = a_object[JP_FRAMES_PROCESSED].toInt();
	if(a_object.contains(JP_FPS))
		properties.fps = a_object[JP_FPS].toDouble();
	return properties;
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
