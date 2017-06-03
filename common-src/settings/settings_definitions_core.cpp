#include "settings_definitions_core.h"

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
