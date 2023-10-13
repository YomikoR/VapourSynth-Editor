#ifndef SETTINGS_DEFINITIONS_CORE_H_INCLUDED
#define SETTINGS_DEFINITIONS_CORE_H_INCLUDED

#include <QString>
#include <QDateTime>
#include <QUuid>
#include <QJsonObject>
#include <vector>

//==============================================================================

enum class ResamplingFilter : int
{
	Point,
	Bilinear,
	Bicubic,
	Spline16,
	Spline36,
	Spline64,
	Lanczos,
};

enum class YuvMatrixCoefficients : int
{
	m709,
	m470BG,
	m170M,
	m2020_NCL,
	m2020_CL,
};

enum class ChromaPlacement : int
{
	LEFT,
	CENTER,
	TOP_LEFT,
};

enum class DitherType: int
{
	NONE,
	ORDERED,
	RANDOM,
	ERROR_DIFFUSION,
};

enum class EncodingType
{
	CLI,
	Raw,
	VfW,
};

enum class EncodingHeaderType
{
	NoHeader,
	Y4M,
};

enum class JobType
{
	EncodeScriptCLI,
	RunProcess,
	RunShellCommand,
};

enum class JobState
{
	Waiting,
	Running,
	Pausing,
	Paused,
	Aborting,
	Aborted,
	FailedCleanUp,
	Failed,
	DependencyNotMet,
	CompletedCleanUp,
	Completed,
};

extern const std::vector<JobState> ACTIVE_JOB_STATES;

extern const char JP_ID[];
extern const char JP_TYPE[];
extern const char JP_JOB_STATE[];
extern const char JP_DEPENDS_ON_JOB_IDS[];
extern const char JP_TIME_STARTED[];
extern const char JP_TIME_ENDED[];
extern const char JP_SCRIPT_NAME[];
extern const char JP_ENCODING_TYPE[];
extern const char JP_ENCODING_HEADER_TYPE[];
extern const char JP_EXECUTABLE_PATH[];
extern const char JP_ARGUMENTS[];
extern const char JP_SHELL_COMMAND[];
extern const char JP_FIRST_FRAME[];
extern const char JP_FIRST_FRAME_REAL[];
extern const char JP_LAST_FRAME[];
extern const char JP_LAST_FRAME_REAL[];
extern const char JP_FRAMES_PROCESSED[];
extern const char JP_FPS[];

struct JobProperties
{
	QUuid id;
	JobType type;
	JobState jobState;
	std::vector<QUuid> dependsOnJobIds;
	QDateTime timeStarted;
	QDateTime timeEnded;
	QString scriptName;
	QString scriptText;
	EncodingType encodingType;
	EncodingHeaderType encodingHeaderType;
	QString executablePath;
	QString arguments;
	QString shellCommand;
	int firstFrame;
	int firstFrameReal;
	int lastFrame;
	int lastFrameReal;
	int framesProcessed;
	double fps;

	JobProperties();
	JobProperties(const JobProperties &) = default;
	JobProperties(JobProperties &&) = default;
	JobProperties & operator=(const JobProperties &) = default;
	JobProperties & operator=(JobProperties &&) = default;

	static QString typeName(JobType a_type);
	static QString stateName(JobState a_state);

	QString subject() const;
	int framesTotal() const;

	QJsonObject toJson() const;
	static JobProperties fromJson(const QJsonObject & a_object);
};

struct EncodingPreset
{
	QString name;
	EncodingType type;
	EncodingHeaderType headerType;
	QString executablePath;
	QString arguments;

	EncodingPreset(const QString & a_name = QString());
	bool operator==(const EncodingPreset & a_other) const;
	bool operator<(const EncodingPreset & a_other) const;
	bool isEmpty() const;
};

//==============================================================================

extern const bool DEFAULT_PREFER_VS_LIBRARIES_FROM_LIST;
extern const ResamplingFilter DEFAULT_CHROMA_RESAMPLING_FILTER;
extern const YuvMatrixCoefficients DEFAULT_YUV_MATRIX_COEFFICIENTS;
extern const ChromaPlacement DEFAULT_CHROMA_PLACEMENT;
extern const double DEFAULT_BICUBIC_FILTER_PARAMETER_B;
extern const double DEFAULT_BICUBIC_FILTER_PARAMETER_C;
extern const int DEFAULT_LANCZOS_FILTER_TAPS;
extern const DitherType DEFAULT_DITHER_TYPE;
extern const EncodingType DEFAULT_ENCODING_TYPE;
extern const EncodingHeaderType DEFAULT_ENCODING_HEADER_TYPE;
extern const JobType DEFAULT_JOB_TYPE;
extern const JobState DEFAULT_JOB_STATE;
extern const int DEFAULT_JOB_FIRST_FRAME;
extern const int DEFAULT_JOB_LAST_FRAME;
extern const int DEFAULT_JOB_FRAMES_PROCESSED;
extern const double DEFAULT_JOB_FPS;
extern const int DEFAULT_RECENT_JOB_SERVERS_NUMBER;
extern const int DEFAULT_WINDOW_GEOMETRY_SAVE_DELAY;

//==============================================================================

#endif // SETTINGS_DEFINITIONS_CORE_H_INCLUDED
