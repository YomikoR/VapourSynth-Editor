#include "settings_definitions.h"

//==============================================================================

const bool DEFAULT_PREVIEW_DIALOG_MAXIMIZED = false;
const bool DEFAULT_ZOOM_PANEL_VISIBLE = true;
const ZoomMode DEFAULT_ZOOM_MODE = ZoomMode::NoZoom;
const double DEFAULT_ZOOM_RATIO = 2.0;
const Qt::TransformationMode DEFAULT_SCALE_MODE = Qt::FastTransformation;
const CropMode DEFAULT_CROP_MODE = CropMode::Relative;
const int DEFAULT_CROP_ZOOM_RATIO = 1;
const double DEFAULT_TIME_STEP = 5.0;
const TimeLineSlider::DisplayMode DEFAULT_TIMELINE_MODE =
	TimeLineSlider::DisplayMode::Time;
const bool DEFAULT_COLOR_PICKER_VISIBLE = false;
const PlayFPSLimitMode DEFAULT_PLAY_FPS_LIMIT_MODE =
	PlayFPSLimitMode::FromVideo;
const double DEFAULT_PLAY_FPS_LIMIT = 23.976;
const int DEFAULT_LAST_PREVIEW_FRAME = 0;
const qlonglong DEFAULT_LAST_PREVIEW_TIMESTAMP = 0;
const SyncOutputNodesMode DEFAULT_SYNC_OUTPUT_MODE =
	SyncOutputNodesMode::Frame;
const bool DEFAULT_TIMELINE_PANEL_VISIBLE = true;
const QString DEFAULT_LAST_SNAPSHOT_EXTENSION = "png";
const int DEFAULT_FPS_DISPLAY_PRECISION = 3;
const double DEFAULT_TIMELINE_LABELS_HEIGHT = 5.0;
const int DEFAULT_PNG_COMPRESSION_LEVEL = 0;
const bool DEFAULT_DEBUG_MESSAGES = false;
const bool DEFAULT_SILENT_SNAPSHOT = false;
const QString DEFAULT_SNAPSHOT_TEMPLATE = "{fn}-{i}.png";

//==============================================================================

const char ACTION_ID_SETTINGS[] = "settings";
const char ACTION_ID_ABOUT[] = "about";
const char ACTION_ID_SAVE_SNAPSHOT[] = "save_snapshot";
const char ACTION_ID_TOGGLE_ZOOM_PANEL[] = "toggle_zoom_panel";
const char ACTION_ID_SET_ZOOM_MODE_NO_ZOOM[] = "set_zoom_mode_no_zoom";
const char ACTION_ID_SET_ZOOM_MODE_FIXED_RATIO[] = "set_zoom_mode_fixed_ratio";
const char ACTION_ID_SET_ZOOM_MODE_FIT_TO_FRAME[] =
	"set_zoom_mode_fit_to_frame";
const char ACTION_ID_SET_ZOOM_SCALE_MODE_NEAREST[] =
	"set_zoom_scale_mode_nearest";
const char ACTION_ID_SET_ZOOM_SCALE_MODE_BILINEAR[] =
	"set_zoom_scale_mode_bilinear";
const char ACTION_ID_TOGGLE_CROP_PANEL[] = "toggle_crop_panel";
const char ACTION_ID_PASTE_CROP_SNIPPET_INTO_SCRIPT[] =
	"paste_crop_snippet_into_script";
const char ACTION_ID_FRAME_TO_CLIPBOARD[] = "frame_to_clipboard";
const char ACTION_ID_TOGGLE_TIMELINE_PANEL[] = "toggle_timeline_panel";
const char ACTION_ID_SET_TIMELINE_MODE_TIME[] = "set_timeline_mode_time";
const char ACTION_ID_SET_TIMELINE_MODE_FRAMES[] = "set_timeline_mode_frames";
const char ACTION_ID_TIME_STEP_FORWARD[] = "time_step_forward";
const char ACTION_ID_TIME_STEP_BACK[] = "time_step_back";
const char ACTION_ID_ADVANCED_PREVIEW_SETTINGS[] = "advanced_preview_settings";
const char ACTION_ID_TOGGLE_COLOR_PICKER[] = "toggle_color_picker";
const char ACTION_ID_PLAY[] = "play";
const char ACTION_ID_TIMELINE_LOAD_CHAPTERS[] = "timeline_load_chapters";
const char ACTION_ID_TIMELINE_CLEAR_BOOKMARKS[] = "timeline_clear_bookmarks";
const char ACTION_ID_TIMELINE_BOOKMARK_CURRENT_FRAME[] =
	"timeline_bookmark_current_frame";
const char ACTION_ID_TIMELINE_UNBOOKMARK_CURRENT_FRAME[] =
	"timeline_unbookmark_current_frame";
const char ACTION_ID_TIMELINE_GO_TO_PREVIOUS_BOOKMARK[] =
	"timeline_go_to_previous_bookmark";
const char ACTION_ID_TIMELINE_GO_TO_NEXT_BOOKMARK[] =
	"timeline_go_to_next_bookmark";
const char ACTION_ID_JUMP_TO_FRAME[] = "jump_to_frame";
const char ACTION_ID_TOGGLE_FRAME_PROPS[] = "toggle_frame_props_panel";
const char ACTION_ID_SET_OUTPUT_INDEX_0[] = "switch_to_output_index_0";
const char ACTION_ID_SET_OUTPUT_INDEX_1[] = "switch_to_output_index_1";
const char ACTION_ID_SET_OUTPUT_INDEX_2[] = "switch_to_output_index_2";
const char ACTION_ID_SET_OUTPUT_INDEX_3[] = "switch_to_output_index_3";
const char ACTION_ID_SET_OUTPUT_INDEX_4[] = "switch_to_output_index_4";
const char ACTION_ID_SET_OUTPUT_INDEX_5[] = "switch_to_output_index_5";
const char ACTION_ID_SET_OUTPUT_INDEX_6[] = "switch_to_output_index_6";
const char ACTION_ID_SET_OUTPUT_INDEX_7[] = "switch_to_output_index_7";
const char ACTION_ID_SET_OUTPUT_INDEX_8[] = "switch_to_output_index_8";
const char ACTION_ID_SET_OUTPUT_INDEX_9[] = "switch_to_output_index_9";
const char ACTION_ID_SET_OUTPUT_INDEX_10[] = "switch_to_output_index_10";
const char ACTION_ID_SET_OUTPUT_INDEX_11[] = "switch_to_output_index_11";
const char ACTION_ID_SET_OUTPUT_INDEX_12[] = "switch_to_output_index_12";
const char ACTION_ID_SET_OUTPUT_INDEX_13[] = "switch_to_output_index_13";
const char ACTION_ID_SET_OUTPUT_INDEX_14[] = "switch_to_output_index_14";
const char ACTION_ID_SET_OUTPUT_INDEX_15[] = "switch_to_output_index_15";
const char ACTION_ID_SET_OUTPUT_INDEX_16[] = "switch_to_output_index_16";
const char ACTION_ID_SET_OUTPUT_INDEX_17[] = "switch_to_output_index_17";
const char ACTION_ID_SET_OUTPUT_INDEX_18[] = "switch_to_output_index_18";
const char ACTION_ID_SET_OUTPUT_INDEX_19[] = "switch_to_output_index_19";

//==============================================================================

const char TEXT_FORMAT_ID_COMMON_SCRIPT_TEXT[] = "common_script_text";
const char TEXT_FORMAT_ID_TIMELINE[] = "timeline_text";
const char COLOR_ID_TIMELINE_BOOKMARKS[] = "timeline_bookmarks";

//==============================================================================

bool StandardAction::operator==(const StandardAction & a_other) const
{
	return id == a_other.id;
}

bool StandardAction::operator<(const StandardAction & a_other) const
{
	return id < a_other.id;
}

//==============================================================================
