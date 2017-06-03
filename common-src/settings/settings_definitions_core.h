#ifndef SETTINGS_DEFINITIONS_CORE_H_INCLUDED
#define SETTINGS_DEFINITIONS_CORE_H_INCLUDED

#include <QString>
#include <QDateTime>
#include <QUuid>
#include <vector>

//==============================================================================

enum class ResamplingFilter : int
{
	Point,
	Bilinear,
	Bicubic,
	Spline16,
	Spline36,
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
	MPEG1,
	MPEG2,
	DV,
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

struct JobProperties
{
	QUuid id;
	JobType type;
	JobState jobState;
	std::vector<QUuid> dependsOnJobIds;
	QDateTime timeStarted;
	QDateTime timeEnded;
	QString scriptName;
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

extern const ResamplingFilter DEFAULT_CHROMA_RESAMPLING_FILTER;
extern const YuvMatrixCoefficients DEFAULT_YUV_MATRIX_COEFFICIENTS;
extern const ChromaPlacement DEFAULT_CHROMA_PLACEMENT;
extern const double DEFAULT_BICUBIC_FILTER_PARAMETER_B;
extern const double DEFAULT_BICUBIC_FILTER_PARAMETER_C;
extern const int DEFAULT_LANCZOS_FILTER_TAPS;
extern const EncodingType DEFAULT_ENCODING_TYPE;
extern const EncodingHeaderType DEFAULT_ENCODING_HEADER_TYPE;
extern const JobType DEFAULT_JOB_TYPE;
extern const JobState DEFAULT_JOB_STATE;
extern const int DEFAULT_JOB_FIRST_FRAME;
extern const int DEFAULT_JOB_LAST_FRAME;
extern const int DEFAULT_JOB_FRAMES_PROCESSED;
extern const double DEFAULT_JOB_FPS;

//==============================================================================

#endif // SETTINGS_DEFINITIONS_CORE_H_INCLUDED
