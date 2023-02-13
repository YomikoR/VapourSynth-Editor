#ifndef SETTINGS_DEFINITIONS_H_INCLUDED
#define SETTINGS_DEFINITIONS_H_INCLUDED

#include "../timeline_slider/timeline_slider.h"
#include "../log/styled_log_view_structures.h"

#include <QString>
#include <QIcon>
#include <QKeySequence>
#include <QStringList>
#include <QDateTime>
#include <vector>

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

enum class PlayFPSLimitMode
{
	FromVideo,
	NoLimit,
	Custom,
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

	CodeSnippet(const QString & a_name = QString(),
		const QString & a_text = QString());
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

extern const bool DEFAULT_MAIN_WINDOW_MAXIMIZED;
extern const bool DEFAULT_PREVIEW_DIALOG_MAXIMIZED;
extern const bool DEFAULT_JOBS_DIALOG_MAXIMIZED;
extern const bool DEFAULT_JOB_SERVER_WATCHER_MAXIMIZED;
extern const bool DEFAULT_AUTO_LOAD_LAST_SCRIPT;
extern const bool DEFAULT_ZOOM_PANEL_VISIBLE;
extern const ZoomMode DEFAULT_ZOOM_MODE;
extern const double DEFAULT_ZOOM_RATIO;
extern const Qt::TransformationMode DEFAULT_SCALE_MODE;
extern const CropMode DEFAULT_CROP_MODE;
extern const int DEFAULT_CROP_ZOOM_RATIO;
extern const bool DEFAULT_PROMPT_TO_SAVE_CHANGES;
extern const unsigned int DEFAULT_MAX_RECENT_FILES_NUMBER;
extern const QStringList DEFAULT_DOCUMENTATION_PATHS;
extern const int DEFAULT_CHARACTERS_TYPED_TO_START_COMPLETION;
extern const double DEFAULT_TIME_STEP;
extern const TimeLineSlider::DisplayMode DEFAULT_TIMELINE_MODE;
extern const bool DEFAULT_COLOR_PICKER_VISIBLE;
extern const PlayFPSLimitMode DEFAULT_PLAY_FPS_LIMIT_MODE;
extern const double DEFAULT_PLAY_FPS_LIMIT;
extern const bool DEFAULT_USE_SPACES_AS_TAB;
extern const int DEFAULT_SPACES_IN_TAB;
extern const bool DEFAULT_REMEMBER_LAST_PREVIEW_FRAME;
extern const int DEFAULT_LAST_PREVIEW_FRAME;
extern const bool DEFAULT_HIGHLIGHT_SELECTION_MATCHES;
extern const int DEFAULT_HIGHLIGHT_SELECTION_MATCHES_MIN_LENGTH;
extern const bool DEFAULT_TIMELINE_PANEL_VISIBLE;
extern const bool DEFAULT_ALWAYS_KEEP_CURRENT_FRAME;
extern const QString DEFAULT_LAST_SNAPSHOT_EXTENSION;
extern const int DEFAULT_FPS_DISPLAY_PRECISION;
extern const double DEFAULT_TIMELINE_LABELS_HEIGHT;
extern const char DEFAULT_DROP_FILE_TEMPLATE[];
extern const int DEFAULT_MAX_WATCHER_CONNECTION_ATTEMPTS;
extern const int DEFAULT_PNG_COMPRESSION_LEVEL;
extern const bool DEFAULT_RELOAD_BEFORE_EXECUTION;
extern const bool DEFAULT_DARK_MODE;
extern const bool DEFAULT_SILENT_SNAPSHOT;
extern const QString DEFAULT_SNAPSHOT_TEMPLATE;

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
extern const char ACTION_ID_ENQUEUE_ENCODE_JOB[];
extern const char ACTION_ID_JOBS[];
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
extern const char ACTION_ID_TIMELINE_LOAD_CHAPTERS[];
extern const char ACTION_ID_TIMELINE_CLEAR_BOOKMARKS[];
extern const char ACTION_ID_TIMELINE_BOOKMARK_CURRENT_FRAME[];
extern const char ACTION_ID_TIMELINE_UNBOOKMARK_CURRENT_FRAME[];
extern const char ACTION_ID_TIMELINE_GO_TO_PREVIOUS_BOOKMARK[];
extern const char ACTION_ID_TIMELINE_GO_TO_NEXT_BOOKMARK[];
extern const char ACTION_ID_PASTE_SHOWN_FRAME_NUMBER_INTO_SCRIPT[];
extern const char ACTION_ID_MOVE_TEXT_BLOCK_UP[];
extern const char ACTION_ID_MOVE_TEXT_BLOCK_DOWN[];
extern const char ACTION_ID_TOGGLE_COMMENT[];
extern const char ACTION_ID_SHUTDOWN_SERVER_AND_EXIT[];
extern const char ACTION_ID_SET_TRUSTED_CLIENTS_ADDRESSES[];
extern const char ACTION_ID_TOGGLE_FRAME_PROPS[];
extern const char ACTION_ID_SET_OUTPUT_INDEX_0[];
extern const char ACTION_ID_SET_OUTPUT_INDEX_1[];
extern const char ACTION_ID_SET_OUTPUT_INDEX_2[];
extern const char ACTION_ID_SET_OUTPUT_INDEX_3[];
extern const char ACTION_ID_SET_OUTPUT_INDEX_4[];
extern const char ACTION_ID_SET_OUTPUT_INDEX_5[];
extern const char ACTION_ID_SET_OUTPUT_INDEX_6[];
extern const char ACTION_ID_SET_OUTPUT_INDEX_7[];
extern const char ACTION_ID_SET_OUTPUT_INDEX_8[];
extern const char ACTION_ID_SET_OUTPUT_INDEX_9[];

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

#endif // SETTINGS_DEFINITIONS_H_INCLUDED
