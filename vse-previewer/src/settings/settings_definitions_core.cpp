#include "settings_definitions_core.h"

const bool DEFAULT_PREFER_VS_LIBRARIES_FROM_LIST = false;
const ResamplingFilter DEFAULT_CHROMA_RESAMPLING_FILTER =
	ResamplingFilter::Bicubic;
const YuvMatrixCoefficients DEFAULT_YUV_MATRIX_COEFFICIENTS =
	YuvMatrixCoefficients::m709;
const ChromaPlacement DEFAULT_CHROMA_PLACEMENT = ChromaPlacement::LEFT;
const double DEFAULT_BICUBIC_FILTER_PARAMETER_B = 0.0;
const double DEFAULT_BICUBIC_FILTER_PARAMETER_C = 0.5;
const int DEFAULT_LANCZOS_FILTER_TAPS = 3;
const DitherType DEFAULT_DITHER_TYPE = DitherType::ERROR_DIFFUSION;
const int DEFAULT_WINDOW_GEOMETRY_SAVE_DELAY = 2000;

//==============================================================================
