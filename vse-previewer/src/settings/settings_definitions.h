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

enum class SyncOutputNodesMode
{
	Frame,
	Time,
	FromTimeLine,
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

//==============================================================================

extern const bool DEFAULT_PREVIEW_DIALOG_MAXIMIZED;
extern const bool DEFAULT_ZOOM_PANEL_VISIBLE;
extern const ZoomMode DEFAULT_ZOOM_MODE;
extern const double DEFAULT_ZOOM_RATIO;
extern const Qt::TransformationMode DEFAULT_SCALE_MODE;
extern const CropMode DEFAULT_CROP_MODE;
extern const int DEFAULT_CROP_ZOOM_RATIO;
extern const double DEFAULT_TIME_STEP;
extern const TimeLineSlider::DisplayMode DEFAULT_TIMELINE_MODE;
extern const bool DEFAULT_COLOR_PICKER_VISIBLE;
extern const PlayFPSLimitMode DEFAULT_PLAY_FPS_LIMIT_MODE;
extern const double DEFAULT_PLAY_FPS_LIMIT;
extern const int DEFAULT_LAST_PREVIEW_FRAME;
extern const qlonglong DEFAULT_LAST_PREVIEW_TIMESTAMP;
extern const SyncOutputNodesMode DEFAULT_SYNC_OUTPUT_MODE;
extern const bool DEFAULT_TIMELINE_PANEL_VISIBLE;
extern const QString DEFAULT_LAST_SNAPSHOT_EXTENSION;
extern const int DEFAULT_FPS_DISPLAY_PRECISION;
extern const double DEFAULT_TIMELINE_LABELS_HEIGHT;
extern const int DEFAULT_PNG_COMPRESSION_LEVEL;
extern const bool DEFAULT_RELOAD_BEFORE_EXECUTION;
extern const bool DEFAULT_DEBUG_MESSAGES;
extern const bool DEFAULT_SILENT_SNAPSHOT;
extern const QString DEFAULT_SNAPSHOT_TEMPLATE;

//==============================================================================

extern const char ACTION_ID_SETTINGS[];
extern const char ACTION_ID_ABOUT[];
extern const char ACTION_ID_SAVE_SNAPSHOT[];
extern const char ACTION_ID_TOGGLE_ZOOM_PANEL[];
extern const char ACTION_ID_SET_ZOOM_MODE_NO_ZOOM[];
extern const char ACTION_ID_SET_ZOOM_MODE_FIXED_RATIO[];
extern const char ACTION_ID_SET_ZOOM_MODE_FIT_TO_FRAME[];
extern const char ACTION_ID_SET_ZOOM_SCALE_MODE_NEAREST[];
extern const char ACTION_ID_SET_ZOOM_SCALE_MODE_BILINEAR[];
extern const char ACTION_ID_TOGGLE_CROP_PANEL[];
extern const char ACTION_ID_FRAME_TO_CLIPBOARD[];
extern const char ACTION_ID_TOGGLE_TIMELINE_PANEL[];
extern const char ACTION_ID_SET_TIMELINE_MODE_TIME[];
extern const char ACTION_ID_SET_TIMELINE_MODE_FRAMES[];
extern const char ACTION_ID_TIME_STEP_FORWARD[];
extern const char ACTION_ID_TIME_STEP_BACK[];
extern const char ACTION_ID_ADVANCED_PREVIEW_SETTINGS[];
extern const char ACTION_ID_TOGGLE_COLOR_PICKER[];
extern const char ACTION_ID_PLAY[];
extern const char ACTION_ID_TIMELINE_LOAD_CHAPTERS[];
extern const char ACTION_ID_TIMELINE_CLEAR_BOOKMARKS[];
extern const char ACTION_ID_TIMELINE_BOOKMARK_CURRENT_FRAME[];
extern const char ACTION_ID_TIMELINE_UNBOOKMARK_CURRENT_FRAME[];
extern const char ACTION_ID_TIMELINE_GO_TO_PREVIOUS_BOOKMARK[];
extern const char ACTION_ID_TIMELINE_GO_TO_NEXT_BOOKMARK[];
extern const char ACTION_ID_JUMP_TO_FRAME[];
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
extern const char ACTION_ID_SET_OUTPUT_INDEX_10[];
extern const char ACTION_ID_SET_OUTPUT_INDEX_11[];
extern const char ACTION_ID_SET_OUTPUT_INDEX_12[];
extern const char ACTION_ID_SET_OUTPUT_INDEX_13[];
extern const char ACTION_ID_SET_OUTPUT_INDEX_14[];
extern const char ACTION_ID_SET_OUTPUT_INDEX_15[];
extern const char ACTION_ID_SET_OUTPUT_INDEX_16[];
extern const char ACTION_ID_SET_OUTPUT_INDEX_17[];
extern const char ACTION_ID_SET_OUTPUT_INDEX_18[];
extern const char ACTION_ID_SET_OUTPUT_INDEX_19[];

//==============================================================================

extern const char TEXT_FORMAT_ID_COMMON_SCRIPT_TEXT[];
extern const char TEXT_FORMAT_ID_TIMELINE[];

extern const char COLOR_ID_TIMELINE_BOOKMARKS[];

//==============================================================================

#endif // SETTINGS_DEFINITIONS_H_INCLUDED
