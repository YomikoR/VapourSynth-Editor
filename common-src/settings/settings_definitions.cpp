#include "settings_definitions.h"

//==============================================================================

const bool DEFAULT_MAIN_WINDOW_MAXIMIZED = false;
const bool DEFAULT_PREVIEW_DIALOG_MAXIMIZED = false;
const bool DEFAULT_JOBS_DIALOG_MAXIMIZED = false;
const bool DEFAULT_JOB_SERVER_WATCHER_MAXIMIZED = false;
const bool DEFAULT_AUTO_LOAD_LAST_SCRIPT = true;
const bool DEFAULT_ZOOM_PANEL_VISIBLE = true;
const ZoomMode DEFAULT_ZOOM_MODE = ZoomMode::NoZoom;
const double DEFAULT_ZOOM_RATIO = 2.0;
const Qt::TransformationMode DEFAULT_SCALE_MODE = Qt::FastTransformation;
const CropMode DEFAULT_CROP_MODE = CropMode::Relative;
const int DEFAULT_CROP_ZOOM_RATIO = 1;
const bool DEFAULT_PROMPT_TO_SAVE_CHANGES = true;
const unsigned int DEFAULT_MAX_RECENT_FILES_NUMBER = 10;
const int DEFAULT_CHARACTERS_TYPED_TO_START_COMPLETION = 2;
const double DEFAULT_TIME_STEP = 5.0;
const TimeLineSlider::DisplayMode DEFAULT_TIMELINE_MODE =
	TimeLineSlider::DisplayMode::Time;
const bool DEFAULT_COLOR_PICKER_VISIBLE = false;
const PlayFPSLimitMode DEFAULT_PLAY_FPS_LIMIT_MODE =
	PlayFPSLimitMode::FromVideo;
const double DEFAULT_PLAY_FPS_LIMIT = 23.976;
const bool DEFAULT_USE_SPACES_AS_TAB = true;
const int DEFAULT_SPACES_IN_TAB = 4;
const bool DEFAULT_REMEMBER_LAST_PREVIEW_FRAME = false;
const int DEFAULT_LAST_PREVIEW_FRAME = 0;
const bool DEFAULT_HIGHLIGHT_SELECTION_MATCHES = true;
const int DEFAULT_HIGHLIGHT_SELECTION_MATCHES_MIN_LENGTH = 3;
const bool DEFAULT_TIMELINE_PANEL_VISIBLE = true;
const bool DEFAULT_ALWAYS_KEEP_CURRENT_FRAME = true;
const QString DEFAULT_LAST_SNAPSHOT_EXTENSION = "png";
const int DEFAULT_FPS_DISPLAY_PRECISION = 3;
const double DEFAULT_TIMELINE_LABELS_HEIGHT = 5.0;
const char DEFAULT_DROP_FILE_TEMPLATE[] = "r\'{f}\'";
const int DEFAULT_MAX_WATCHER_CONNECTION_ATTEMPTS = 5;
const int DEFAULT_PNG_COMPRESSION_LEVEL = 0;
const bool DEFAULT_RELOAD_BEFORE_EXECUTION = false;
const bool DEFAULT_DARK_MODE = false;
const bool DEFAULT_SILENT_SNAPSHOT = false;
const QString DEFAULT_SNAPSHOT_TEMPLATE = "{f}-{i}-{o}.png";

//==============================================================================

const char ACTION_ID_NEW_SCRIPT[] = "new_script";
const char ACTION_ID_OPEN_SCRIPT[] = "open_script";
const char ACTION_ID_SAVE_SCRIPT[] = "save_script";
const char ACTION_ID_SAVE_SCRIPT_AS[] = "save_script_as";
const char ACTION_ID_TEMPLATES[] = "templates";
const char ACTION_ID_SETTINGS[] = "settings";
const char ACTION_ID_PREVIEW[] = "preview";
const char ACTION_ID_CHECK_SCRIPT[] = "check_script";
const char ACTION_ID_BENCHMARK[] = "benchmark";
const char ACTION_ID_CLI_ENCODE[] = "cli_encode";
const char ACTION_ID_ENQUEUE_ENCODE_JOB[] = "enqueue_encode_job";
const char ACTION_ID_JOBS[] = "jobs";
const char ACTION_ID_EXIT[] = "exit";
const char ACTION_ID_ABOUT[] = "about";
const char ACTION_ID_AUTOCOMPLETE[] = "autocomplete";
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
const char ACTION_ID_DUPLICATE_SELECTION[] = "duplicate_selection";
const char ACTION_ID_COMMENT_SELECTION[] = "comment_selection";
const char ACTION_ID_UNCOMMENT_SELECTION[] = "uncomment_selection";
const char ACTION_ID_REPLACE_TAB_WITH_SPACES[] = "replace_tab_with_spaces";
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
const char ACTION_ID_PASTE_SHOWN_FRAME_NUMBER_INTO_SCRIPT[] =
	"paste_shown_frame_number_into_script";
const char ACTION_ID_MOVE_TEXT_BLOCK_UP[] = "move_text_block_up";
const char ACTION_ID_MOVE_TEXT_BLOCK_DOWN[] = "move_text_block_down";
const char ACTION_ID_TOGGLE_COMMENT[] = "toggle_comment";
const char ACTION_ID_SHUTDOWN_SERVER_AND_EXIT[] = "shutdown_server_and_exit";
const char ACTION_ID_SET_TRUSTED_CLIENTS_ADDRESSES[] =
	"set_trusted_clients_addresses";
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

//==============================================================================

const char TEXT_FORMAT_ID_COMMON_SCRIPT_TEXT[] = "common_script_text";
const char TEXT_FORMAT_ID_KEYWORD[] = "keyword";
const char TEXT_FORMAT_ID_OPERATOR[] = "operator";
const char TEXT_FORMAT_ID_STRING[] = "string";
const char TEXT_FORMAT_ID_NUMBER[] = "number";
const char TEXT_FORMAT_ID_COMMENT[] = "comment";
const char TEXT_FORMAT_ID_VS_CORE[] = "vs_core";
const char TEXT_FORMAT_ID_VS_NAMESPACE[] = "vs_namespace";
const char TEXT_FORMAT_ID_VS_FUNCTION[] = "vs_function";
const char TEXT_FORMAT_ID_VS_ARGUMENT[] = "vs_argument";
const char TEXT_FORMAT_ID_TIMELINE[] = "timeline_text";

const char COLOR_ID_TEXT_BACKGROUND[] = "text_background_color";
const char COLOR_ID_ACTIVE_LINE[] = "active_line_color";
const char COLOR_ID_SELECTION_MATCHES[] = "selection_matches";
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

CodeSnippet::CodeSnippet(const QString & a_name, const QString & a_text) :
	  name(a_name)
	, text(a_text)
{
}

bool CodeSnippet::operator==(const CodeSnippet & a_other) const
{
	return (name == a_other.name);
}

bool CodeSnippet::operator<(const CodeSnippet & a_other) const
{
	return (name < a_other.name);
}

bool CodeSnippet::isEmpty() const
{
	return (name.isEmpty() && text.isEmpty());
}

//==============================================================================
