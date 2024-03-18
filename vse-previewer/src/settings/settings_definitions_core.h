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

//==============================================================================

extern const bool DEFAULT_PREFER_VS_LIBRARIES_FROM_LIST;
extern const ResamplingFilter DEFAULT_CHROMA_RESAMPLING_FILTER;
extern const YuvMatrixCoefficients DEFAULT_YUV_MATRIX_COEFFICIENTS;
extern const ChromaPlacement DEFAULT_CHROMA_PLACEMENT;
extern const double DEFAULT_BICUBIC_FILTER_PARAMETER_B;
extern const double DEFAULT_BICUBIC_FILTER_PARAMETER_C;
extern const int DEFAULT_LANCZOS_FILTER_TAPS;
extern const DitherType DEFAULT_DITHER_TYPE;
extern const int DEFAULT_WINDOW_GEOMETRY_SAVE_DELAY;

//==============================================================================

#endif // SETTINGS_DEFINITIONS_CORE_H_INCLUDED
