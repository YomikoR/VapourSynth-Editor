#ifndef SETTINGS_DEFINITIONS_H_INCLUDED
#define SETTINGS_DEFINITIONS_H_INCLUDED

#include "../preview/timelineslider.h"

#include <QString>
#include <QIcon>
#include <QKeySequence>
#include <QStringList>

//==============================================================================

enum class ZoomMode
{
	NoZoom,
	FixedRatio,
	FitToFrame,
};

enum class CropMode
{
	Absolute,
	Relative,
};

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

enum class PlayFPSLimitMode
{
	FromVideo,
	NoLimit,
	Custom,
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
};

struct EncodingPreset
{
	QString name;
	EncodingType type;
	EncodingHeaderType headerType;
	QString executablePath;
	QString arguments;

	EncodingPreset();
	EncodingPreset(const QString & a_name);
	bool operator==(const EncodingPreset & a_other) const;
	bool operator<(const EncodingPreset & a_other) const;
	bool isEmpty() const;
};

struct StandardAction
{
	QString id;
	QString title;
	QIcon icon;
	QKeySequence hotkey;

	bool operator==(const StandardAction & a_other) const;
	bool operator<(const StandardAction & a_other) const;
};

struct CodeSnippet
{
	QString name;
	QString text;

	CodeSnippet();
	CodeSnippet(const QString & a_name, const QString & a_text = QString());
	bool operator==(const CodeSnippet & a_other) const;
	bool operator<(const CodeSnippet & a_other) const;
	bool isEmpty() const;
};

struct DropFileCategory
{
	QString name;
	QStringList maskList;
	QString sourceTemplate;
};

//==============================================================================

extern const int DEFAULT_CHARACTERS_TYPED_TO_START_COMPLETION;
extern const ResamplingFilter DEFAULT_CHROMA_RESAMPLING_FILTER;
extern const YuvMatrixCoefficients DEFAULT_YUV_MATRIX_COEFFICIENTS;
extern const ChromaPlacement DEFAULT_CHROMA_PLACEMENT;
extern const double DEFAULT_BICUBIC_FILTER_PARAMETER_B;
extern const double DEFAULT_BICUBIC_FILTER_PARAMETER_C;
extern const int DEFAULT_LANCZOS_FILTER_TAPS;
extern const int DEFAULT_SPACES_IN_TAB;
extern const bool DEFAULT_HIGHLIGHT_SELECTION_MATCHES;
extern const int DEFAULT_HIGHLIGHT_SELECTION_MATCHES_MIN_LENGTH;

//==============================================================================

extern const char ACTION_ID_NEW_SCRIPT[];
extern const char ACTION_ID_OPEN_SCRIPT[];
extern const char ACTION_ID_SAVE_SCRIPT[];
extern const char ACTION_ID_SAVE_SCRIPT_AS[];
extern const char ACTION_ID_TEMPLATES[];
extern const char ACTION_ID_SETTINGS[];
extern const char ACTION_ID_PREVIEW[];
extern const char ACTION_ID_CHECK_SCRIPT[];
extern const char ACTION_ID_BENCHMARK[];
extern const char ACTION_ID_CLI_ENCODE[];
extern const char ACTION_ID_EXIT[];
extern const char ACTION_ID_ABOUT[];
extern const char ACTION_ID_AUTOCOMPLETE[];
extern const char ACTION_ID_SAVE_SNAPSHOT[];
extern const char ACTION_ID_TOGGLE_ZOOM_PANEL[];
extern const char ACTION_ID_SET_ZOOM_MODE_NO_ZOOM[];
extern const char ACTION_ID_SET_ZOOM_MODE_FIXED_RATIO[];
extern const char ACTION_ID_SET_ZOOM_MODE_FIT_TO_FRAME[];
extern const char ACTION_ID_SET_ZOOM_SCALE_MODE_NEAREST[];
extern const char ACTION_ID_SET_ZOOM_SCALE_MODE_BILINEAR[];
extern const char ACTION_ID_TOGGLE_CROP_PANEL[];
extern const char ACTION_ID_PASTE_CROP_SNIPPET_INTO_SCRIPT[];
extern const char ACTION_ID_FRAME_TO_CLIPBOARD[];
extern const char ACTION_ID_TOGGLE_TIMELINE_PANEL[];
extern const char ACTION_ID_SET_TIMELINE_MODE_TIME[];
extern const char ACTION_ID_SET_TIMELINE_MODE_FRAMES[];
extern const char ACTION_ID_TIME_STEP_FORWARD[];
extern const char ACTION_ID_TIME_STEP_BACK[];
extern const char ACTION_ID_ADVANCED_PREVIEW_SETTINGS[];
extern const char ACTION_ID_TOGGLE_COLOR_PICKER[];
extern const char ACTION_ID_PLAY[];
extern const char ACTION_ID_DUPLICATE_SELECTION[];
extern const char ACTION_ID_COMMENT_SELECTION[];
extern const char ACTION_ID_UNCOMMENT_SELECTION[];
extern const char ACTION_ID_REPLACE_TAB_WITH_SPACES[];
extern const char ACTION_ID_TIMELINE_CLEAR_BOOKMARKS[];
extern const char ACTION_ID_TIMELINE_BOOKMARK_CURRENT_FRAME[];
extern const char ACTION_ID_TIMELINE_UNBOOKMARK_CURRENT_FRAME[];
extern const char ACTION_ID_TIMELINE_GO_TO_PREVIOUS_BOOKMARK[];
extern const char ACTION_ID_TIMELINE_GO_TO_NEXT_BOOKMARK[];

//==============================================================================

extern const char TEXT_FORMAT_ID_COMMON_SCRIPT_TEXT[];
extern const char TEXT_FORMAT_ID_KEYWORD[];
extern const char TEXT_FORMAT_ID_OPERATOR[];
extern const char TEXT_FORMAT_ID_STRING[];
extern const char TEXT_FORMAT_ID_NUMBER[];
extern const char TEXT_FORMAT_ID_COMMENT[];
extern const char TEXT_FORMAT_ID_VS_CORE[];
extern const char TEXT_FORMAT_ID_VS_NAMESPACE[];
extern const char TEXT_FORMAT_ID_VS_FUNCTION[];
extern const char TEXT_FORMAT_ID_VS_ARGUMENT[];
extern const char TEXT_FORMAT_ID_TIMELINE[];

extern const char COLOR_ID_TEXT_BACKGROUND[];
extern const char COLOR_ID_ACTIVE_LINE[];
extern const char COLOR_ID_SELECTION_MATCHES[];
extern const char COLOR_ID_TIMELINE_BOOKMARKS[];

//==============================================================================

extern const EncodingType DEFAULT_ENCODING_TYPE;
extern const EncodingHeaderType DEFAULT_ENCODING_HEADER_TYPE;

//==============================================================================

#endif // SETTINGS_DEFINITIONS_H_INCLUDED
