#include "settings_manager.h"

#include "../helpers.h"

#include <QSettings>
#include <QFileInfo>
#include <QPalette>
#include <QFontMetricsF>
#include <QRegExp>

//==============================================================================

const char MAIN_WINDOW_GEOMETRY_KEY[] = "main_window_geometry";
const char PREVIEW_DIALOG_GEOMETRY_KEY[] = "prewiew_dialog_geometry";
const char JOB_SERVER_WATCHER_GEOMETRY_KEY[] = "job_server_watcher_geometry";
const char MAIN_WINDOW_MAXIMIZED_KEY[] = "main_window_maximized";
const char PREVIEW_DIALOG_MAXIMIZED_KEY[] = "preview_dialog_maximized";
const char JOB_SERVER_WATCHER_MAXIMIZED_KEY[] = "job_server_watcher_maximized";
const char JOBS_HEADER_STATE_KEY[] = "jobs_header_state";
const char AUTO_LOAD_LAST_SCRIPT_KEY[] = "auto_load_last_script";
const char ZOOM_PANEL_VISIBLE_KEY[] = "zoom_panel_visible";
const char ZOOM_MODE_KEY[] = "zoom_mode";
const char ZOOM_RATIO_KEY[] = "zoom_ratio";
const char SCALE_MODE_KEY[] = "scale_mode";
const char CROP_MODE_KEY[] = "crop_mode";
const char CROP_ZOOM_RATIO_KEY[] = "crop_zoom_ratio";
const char PROMPT_TO_SAVE_CHANGES_KEY[] = "prompt_to_save_changes";
const char RECENT_FILES_LIST_KEY[] = "recent_files_list";
const char MAX_RECENT_FILES_NUMBER_KEY[] = "max_recent_files_number";
const char CHARACTERS_TYPED_TO_START_COMPLETION_KEY[] =
	"characters_typed_to_start_completion";
const char TIMELINE_MODE_KEY[] = "timeline_mode";
const char TIME_STEP_KEY[] = "time_step_mode";
const char COLOR_PICKER_VISIBLE_KEY[] = "color_picker_visible";
const char PLAY_FPS_LIMIT_MODE_KEY[] = "play_fps_limit_mode";
const char PLAY_FPS_LIMIT_KEY[] = "play_fps_limit";
const char USE_SPACES_AS_TAB_KEY[] = "use_spaces_as_tab";
const char SPACES_IN_TAB_KEY[] = "spaces_in_tab";
const char REMEMBER_LAST_PREVIEW_FRAME_KEY[] = "remember_last_preview_frame";
const char LAST_PREVIEW_FRAME_KEY[] = "last_preview_frame";
const char NEW_SCRIPT_TEMPLATE_KEY[] = "new_script_template";
const char HIGHLIGHT_SELECTION_MATCHES_KEY[] = "highlight_selection_matches";
const char HIGHLIGHT_SELECTION_MATCHES_MIN_LENGTH_KEY[] =
	"highlight_selection_matches_min_length";
const char TIMELINE_PANEL_VISIBLE_KEY[] = "timeline_panel_visible";
const char ALWAYS_KEEP_CURRENT_FRAME_KEY[] = "always_keep_current_frame";
const char LAST_SNAPSHOT_EXTENSION_KEY[] = "last_snapshot_extension";
const char PNG_COMPRESSION_LEVEL_KEY[] = "png_compression_level";
const char RELOAD_BEFORE_EXECUTION_KEY[] = "reload_before_execution";
const char DARK_MODE_KEY[] = "dark_mode";
const char SILENT_SNAPSHOT_KEY[] = "silent_snapshot";
const char SNAPSHOT_TEMPLATE_KEY[] = "snapshot_template";

//==============================================================================

const char HOTKEYS_GROUP[] = "hotkeys";

//==============================================================================

const char THEME_GROUP[] = "theme";

//==============================================================================

const char DARK_THEME_GROUP[] = "dark_theme";

//==============================================================================

const char PREVIEWER_GROUP[] = "previewer";

//==============================================================================

const char CODE_SNIPPETS_GROUP[] = "code_snippets";

//==============================================================================

const char DROP_FILE_TEMPLATES_GROUP[] = "drop_file_templates";

const char DROP_FILE_CATEGORY_MASK_LIST_KEY[] = "mask_list";
const char DROP_FILE_CATEGORY_SOURCE_TEMPLATE_KEY[] = "template";

//==============================================================================

const char LOGS_GROUP[] = "logs";
const char LOG_STYLES_GROUP[] = "styles";

const char LOG_STYLE_TITLE_KEY[] = "title";
const char LOG_STYLE_TEXT_FORMAT_KEY[] = "text_format";
const char LOG_STYLE_IS_ALIAS_KEY[] = "is_alias";
const char LOG_STYLE_ORIGINAL_STYLE_NAME_KEY[] = "original_style_name";
const char LOG_STYLE_IS_VISIBLE_KEY[] = "is_visible";

//==============================================================================

SettingsManager::SettingsManager(QObject * a_pParent) :
	SettingsManagerCore(a_pParent)
{
	initializeStandardActions();
	m_bInDarkMode = getDarkMode();
}

SettingsManager::~SettingsManager()
{

}

//==============================================================================

void SettingsManager::initializeStandardActions()
{
	m_standardActions =
	{
		{ACTION_ID_NEW_SCRIPT, tr("New script"), QIcon(":new.png"),
			QKeySequence::New},
		{ACTION_ID_OPEN_SCRIPT, tr("Open script"), QIcon(":load.png"),
			QKeySequence::Open},
		{ACTION_ID_SAVE_SCRIPT, tr("Save script"), QIcon(":save.png"),
			QKeySequence::Save},
		{ACTION_ID_SAVE_SCRIPT_AS, tr("Save script as..."),
			QIcon(":save_as.png"), QKeySequence::SaveAs},
		{ACTION_ID_EXIT, tr("Exit"), QIcon(":exit.png"),
			QKeySequence::Quit},
		{ACTION_ID_DUPLICATE_SELECTION, tr("Duplicate selection or line"),
			QIcon(), QKeySequence(Qt::CTRL | Qt::Key_D)},
		{ACTION_ID_COMMENT_SELECTION, tr("Comment lines"), QIcon(),
			QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C)},
		{ACTION_ID_UNCOMMENT_SELECTION, tr("Uncomment lines"), QIcon(),
			QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_X)},
		{ACTION_ID_REPLACE_TAB_WITH_SPACES,
			tr("Replace Tab characters with spaces"), QIcon(),
			QKeySequence()},
		{ACTION_ID_TEMPLATES, tr("Snippets and templates"), QIcon(),
			QKeySequence()},
		{ACTION_ID_SETTINGS, tr("Settings"), QIcon(":settings.png"),
			QKeySequence()},
		{ACTION_ID_PREVIEW, tr("Preview"), QIcon(":preview.png"),
			QKeySequence(Qt::Key_F5)},
		{ACTION_ID_CHECK_SCRIPT, tr("Check script"), QIcon(":check.png"),
			QKeySequence(Qt::Key_F6)},
		{ACTION_ID_BENCHMARK, tr("Benchmark"), QIcon(":benchmark.png"),
			QKeySequence(Qt::Key_F7)},
		{ACTION_ID_CLI_ENCODE, tr("Encode video"),
			QIcon(":film_save.png"), QKeySequence(Qt::Key_F8)},
		{ACTION_ID_ENQUEUE_ENCODE_JOB, tr("Enqueue encode job"),
			QIcon(":jobs.png"), QKeySequence(Qt::Key_F9)},
		{ACTION_ID_JOBS, tr("Jobs"),
			QIcon(":jobs.png"), QKeySequence(Qt::Key_F10)},
		{ACTION_ID_ABOUT, tr("About..."), QIcon(), QKeySequence()},
		{ACTION_ID_AUTOCOMPLETE, tr("Autocomplete"), QIcon(),
			QKeySequence(Qt::CTRL | Qt::Key_Space)},
		{ACTION_ID_FRAME_TO_CLIPBOARD, tr("Copy frame to clipboard"),
			QIcon(":image_to_clipboard.png"), QKeySequence(Qt::Key_X)},
		{ACTION_ID_SAVE_SNAPSHOT, tr("Save snapshot"),
			QIcon(":snapshot.png"), QKeySequence(Qt::Key_S)},
		{ACTION_ID_TOGGLE_ZOOM_PANEL, tr("Show zoom panel"),
			QIcon(":zoom.png"), QKeySequence(Qt::Key_Z)},
		{ACTION_ID_SET_ZOOM_MODE_NO_ZOOM, tr("Zoom: No zoom"),
			QIcon(":zoom_no_zoom.png"), QKeySequence(
			Qt::ALT | Qt::Key_1)},
		{ACTION_ID_SET_ZOOM_MODE_FIXED_RATIO, tr("Zoom: Fixed ratio"),
			QIcon(":zoom_fixed_ratio.png"), QKeySequence(
			Qt::ALT | Qt::Key_2)},
		{ACTION_ID_SET_ZOOM_MODE_FIT_TO_FRAME, tr("Zoom: Fit to frame"),
			QIcon(":zoom_fit_to_frame.png"), QKeySequence(
			Qt::ALT | Qt::Key_3)},
		{ACTION_ID_SET_ZOOM_SCALE_MODE_NEAREST, tr("Scale: Nearest"),
			QIcon(), QKeySequence()},
		{ACTION_ID_SET_ZOOM_SCALE_MODE_BILINEAR, tr("Scale: Bilinear"),
			QIcon(), QKeySequence()},
		{ACTION_ID_TOGGLE_CROP_PANEL, tr("Crop assistant"),
			QIcon(":crop.png"), QKeySequence(Qt::Key_C)},
		{ACTION_ID_PASTE_CROP_SNIPPET_INTO_SCRIPT,
			tr("Paste crop snippet into script"), QIcon(":paste.png"),
			QKeySequence()},
		{ACTION_ID_TOGGLE_TIMELINE_PANEL, tr("Show timeline panel"),
			QIcon(":timeline.png"), QKeySequence(Qt::Key_T)},
		{ACTION_ID_SET_TIMELINE_MODE_TIME, tr("Timeline mode: Time"),
			QIcon(":timeline.png"), QKeySequence()},
		{ACTION_ID_SET_TIMELINE_MODE_FRAMES, tr("Timeline mode: Frames"),
			QIcon(":timeline_frames.png"), QKeySequence()},
		{ACTION_ID_TIME_STEP_FORWARD, tr("Time: step forward"),
			QIcon(":time_forward.png"),
			QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Right)},
		{ACTION_ID_TIME_STEP_BACK, tr("Time: step back"),
			QIcon(":time_back.png"),
			QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Left)},
		{ACTION_ID_ADVANCED_PREVIEW_SETTINGS,
			tr("Preview advanced settings"), QIcon(":settings.png"),
			QKeySequence()},
		{ACTION_ID_TOGGLE_COLOR_PICKER, tr("Color panel"),
			QIcon(":color_picker.png"), QKeySequence()},
		{ACTION_ID_PLAY, tr("Play"), QIcon(":play.png"), QKeySequence()},
		{ACTION_ID_TIMELINE_LOAD_CHAPTERS, tr("Load chapters"),
			QIcon(":load.png"), QKeySequence()},
		{ACTION_ID_TIMELINE_CLEAR_BOOKMARKS, tr("Clear bookmarks"),
			QIcon(":timeline_bookmark.png"), QKeySequence()},
		{ACTION_ID_TIMELINE_BOOKMARK_CURRENT_FRAME,
			tr("Bookmark current frame"),
			QIcon(":timeline_bookmark_add.png"),
			QKeySequence(Qt::CTRL | Qt::Key_B)},
		{ACTION_ID_TIMELINE_UNBOOKMARK_CURRENT_FRAME,
			tr("Unbookmark current frame"),
			QIcon(":timeline_bookmark_remove.png"),
			QKeySequence(Qt::CTRL | Qt::Key_U)},
		{ACTION_ID_TIMELINE_GO_TO_PREVIOUS_BOOKMARK,
			tr("Go to previous bookmark"),
			QIcon(":timeline_bookmark_previous.png"),
			QKeySequence(Qt::CTRL | Qt::Key_Left)},
		{ACTION_ID_TIMELINE_GO_TO_NEXT_BOOKMARK,
			tr("Go to next bookmark"),
			QIcon(":timeline_bookmark_next.png"),
			QKeySequence(Qt::CTRL | Qt::Key_Right)},
		{ACTION_ID_PASTE_SHOWN_FRAME_NUMBER_INTO_SCRIPT,
			tr("Paste shown frame number into script"), QIcon(),
			QKeySequence()},
		{ACTION_ID_MOVE_TEXT_BLOCK_UP, tr("Move text block up"), QIcon(),
			QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Up)},
		{ACTION_ID_MOVE_TEXT_BLOCK_DOWN, tr("Move text block down"),
			QIcon(), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Down)},
		{ACTION_ID_TOGGLE_COMMENT, tr("Toggle comment"),
			QIcon(), QKeySequence(Qt::CTRL | Qt::Key_Slash)},
		{ACTION_ID_SHUTDOWN_SERVER_AND_EXIT, tr("Shutdown server and exit"),
			QIcon(":exit.png"), QKeySequence()},
		{ACTION_ID_SET_TRUSTED_CLIENTS_ADDRESSES,
			tr("Set trusted clients addresses"), QIcon(), QKeySequence()},
		{ACTION_ID_TOGGLE_FRAME_PROPS, tr("Toggle frame properties panel"),
			QIcon(), QKeySequence(Qt::Key_P)},
		{ACTION_ID_SET_OUTPUT_INDEX_0, tr("Switch to output index 0"),
			QIcon(), QKeySequence(Qt::Key_0)},
		{ACTION_ID_SET_OUTPUT_INDEX_1, tr("Switch to output index 1"),
			QIcon(), QKeySequence(Qt::Key_1)},
		{ACTION_ID_SET_OUTPUT_INDEX_2, tr("Switch to output index 2"),
			QIcon(), QKeySequence(Qt::Key_2)},
		{ACTION_ID_SET_OUTPUT_INDEX_3, tr("Switch to output index 3"),
			QIcon(), QKeySequence(Qt::Key_3)},
		{ACTION_ID_SET_OUTPUT_INDEX_4, tr("Switch to output index 4"),
			QIcon(), QKeySequence(Qt::Key_4)},
		{ACTION_ID_SET_OUTPUT_INDEX_5, tr("Switch to output index 5"),
			QIcon(), QKeySequence(Qt::Key_5)},
		{ACTION_ID_SET_OUTPUT_INDEX_6, tr("Switch to output index 6"),
			QIcon(), QKeySequence(Qt::Key_6)},
		{ACTION_ID_SET_OUTPUT_INDEX_7, tr("Switch to output index 7"),
			QIcon(), QKeySequence(Qt::Key_7)},
		{ACTION_ID_SET_OUTPUT_INDEX_8, tr("Switch to output index 8"),
			QIcon(), QKeySequence(Qt::Key_8)},
		{ACTION_ID_SET_OUTPUT_INDEX_9, tr("Switch to output index 9"),
			QIcon(), QKeySequence(Qt::Key_9)},
		{ACTION_ID_SET_OUTPUT_INDEX_10, tr("Switch to output index 10"),
			QIcon(), QKeySequence()},
		{ACTION_ID_SET_OUTPUT_INDEX_11, tr("Switch to output index 11"),
			QIcon(), QKeySequence()},
		{ACTION_ID_SET_OUTPUT_INDEX_12, tr("Switch to output index 12"),
			QIcon(), QKeySequence()},
		{ACTION_ID_SET_OUTPUT_INDEX_13, tr("Switch to output index 13"),
			QIcon(), QKeySequence()},
		{ACTION_ID_SET_OUTPUT_INDEX_14, tr("Switch to output index 14"),
			QIcon(), QKeySequence()},
		{ACTION_ID_SET_OUTPUT_INDEX_15, tr("Switch to output index 15"),
			QIcon(), QKeySequence()},
		{ACTION_ID_SET_OUTPUT_INDEX_16, tr("Switch to output index 16"),
			QIcon(), QKeySequence()},
		{ACTION_ID_SET_OUTPUT_INDEX_17, tr("Switch to output index 17"),
			QIcon(), QKeySequence()},
		{ACTION_ID_SET_OUTPUT_INDEX_18, tr("Switch to output index 18"),
			QIcon(), QKeySequence()},
		{ACTION_ID_SET_OUTPUT_INDEX_19, tr("Switch to output index 19"),
			QIcon(), QKeySequence()},
	};
}

std::vector<StandardAction> SettingsManager::getStandardActions() const
{
	return m_standardActions;
}

QAction * SettingsManager::createStandardAction(const QString & a_actionID,
	QObject * a_pParent)
{
	StandardAction actionToFind;
	actionToFind.id = a_actionID;

	std::vector<StandardAction>::const_iterator it = std::find(
		m_standardActions.begin(), m_standardActions.end(), actionToFind);
	if(it == m_standardActions.end())
		return nullptr;

	QKeySequence hotkey = getHotkey(it->id);

	QAction * pAction = new QAction(it->icon, it->title, a_pParent);
	pAction->setData(it->id);
	pAction->setShortcut(hotkey);

	vsedit::disableFontKerning(pAction);

	return pAction;
}

QKeySequence SettingsManager::getDefaultHotkey(const QString & a_actionID) const
{
	StandardAction actionToFind;
	actionToFind.id = a_actionID;

	std::vector<StandardAction>::const_iterator it = std::find(
		m_standardActions.begin(), m_standardActions.end(), actionToFind);
	if(it != m_standardActions.end())
		return it->hotkey;

	return QKeySequence();
}

QKeySequence SettingsManager::getHotkey(const QString & a_actionID) const
{
	QSettings settings(m_settingsFilePath, QSettings::IniFormat);
	settings.beginGroup(HOTKEYS_GROUP);
	if(!settings.contains(a_actionID))
		return getDefaultHotkey(a_actionID);

	QKeySequence hotkey =
		settings.value(a_actionID).value<QKeySequence>();
	return hotkey;
}

bool SettingsManager::setHotkey(const QString & a_actionID,
	const QKeySequence & a_hotkey)
{
	return setValueInGroup(HOTKEYS_GROUP, a_actionID, a_hotkey);
}

//==============================================================================

QTextCharFormat SettingsManager::getDefaultTextFormat(
	const QString & a_textFormatID) const
{
	// Standard "Icecream" theme

	QTextCharFormat defaultFormat;

	if(a_textFormatID == TEXT_FORMAT_ID_COMMON_SCRIPT_TEXT)
	{
		QFont commonScriptFont = defaultFormat.font();
#ifdef Q_OS_MACOS
		commonScriptFont.setFamily("menlo");
#else
		commonScriptFont.setFamily("monospace");
#endif
		commonScriptFont.setStyleHint(QFont::Monospace);
		commonScriptFont.setFixedPitch(true);
		commonScriptFont.setKerning(false);
		commonScriptFont.setPointSize(12);
		defaultFormat.setFont(commonScriptFont);

		QColor defaultColor = getColor(COLOR_ID_TEXT_BACKGROUND);
		int refColor = 255 - (defaultColor.red() + defaultColor.green() +
			defaultColor.blue() + 64) * 0.25;
		defaultFormat.setForeground(QColor(refColor, refColor, refColor));
	}
	else if(a_textFormatID == TEXT_FORMAT_ID_KEYWORD)
	{
		defaultFormat.setForeground(QColor("#0EAA95"));
		defaultFormat.setFontWeight(QFont::Bold);
	}
	else if(a_textFormatID == TEXT_FORMAT_ID_OPERATOR)
	{
		defaultFormat.setForeground(QColor("#b9672a"));
	}
	else if(a_textFormatID == TEXT_FORMAT_ID_STRING)
	{
		defaultFormat.setForeground(QColor("#a500bc"));
	}
	else if(a_textFormatID == TEXT_FORMAT_ID_NUMBER)
	{
		defaultFormat.setForeground(QColor("#3f8300"));
	}
	else if(a_textFormatID == TEXT_FORMAT_ID_COMMENT)
	{
		defaultFormat.setForeground(QColor("#387000"));
	}
	else if(a_textFormatID == TEXT_FORMAT_ID_VS_CORE)
	{
		defaultFormat.setForeground(QColor("#0673E0"));
		defaultFormat.setFontWeight(QFont::Bold);
	}
	else if(a_textFormatID == TEXT_FORMAT_ID_VS_NAMESPACE)
	{
		defaultFormat.setForeground(QColor("#0673E0"));
		defaultFormat.setFontWeight(QFont::Bold);
	}
	else if(a_textFormatID == TEXT_FORMAT_ID_VS_FUNCTION)
	{
		defaultFormat.setForeground(QColor("#0673E0"));
		defaultFormat.setFontWeight(QFont::Bold);
	}
	else if(a_textFormatID == TEXT_FORMAT_ID_VS_ARGUMENT)
	{
		defaultFormat.setForeground(QColor("#a500bc"));
	}
	else if(a_textFormatID == TEXT_FORMAT_ID_TIMELINE)
	{
		QFont timelineLabelsFont = defaultFormat.font();
		timelineLabelsFont.setFamily(QString("Digital Mini"));

		QFontMetricsF metrics(timelineLabelsFont);
		qreal factor = (qreal)DEFAULT_TIMELINE_LABELS_HEIGHT /
			metrics.tightBoundingRect("9").height();
		qreal currentFontSize = timelineLabelsFont.pointSizeF();
		timelineLabelsFont.setPointSizeF(currentFontSize * factor);

		defaultFormat.setFont(timelineLabelsFont);
	}

	return defaultFormat;
}

QTextCharFormat SettingsManager::getTextFormat(const QString & a_textFormatID)
	const
{
	QVariant textFormatValue = valueInGroup(inDarkMode() ? DARK_THEME_GROUP
		: THEME_GROUP, a_textFormatID);
	if(textFormatValue.isNull())
		return getDefaultTextFormat(a_textFormatID);
	return vsedit::fromByteArray<QTextCharFormat>(
		textFormatValue.toByteArray());
}

bool SettingsManager::setTextFormat(const QString & a_textFormatID,
	const QTextCharFormat & a_format)
{
	// Only record to settings if it doesn't match the default
	QTextCharFormat defaultFmt = getTextFormat(a_textFormatID);
	if(defaultFmt == a_format) return true;
	return setValueInGroup(inDarkMode() ? DARK_THEME_GROUP
		: THEME_GROUP, a_textFormatID, vsedit::toByteArray(a_format));
}

//==============================================================================

QColor SettingsManager::getDefaultColor(const QString & a_colorID) const
{
	QColor defaultColor;

	QPalette defaultPalette;

	if(a_colorID == COLOR_ID_TEXT_BACKGROUND)
	{
#ifdef Q_OS_WIN
		return inDarkMode() ? QColor(16, 16, 24) : QColor(255, 255, 255);
#else
		return defaultPalette.color(QPalette::Active, QPalette::Base);
#endif
	}

	if(a_colorID == COLOR_ID_ACTIVE_LINE)
	{
		defaultColor = getColor(COLOR_ID_TEXT_BACKGROUND);
		int refColor = (defaultColor.red() + defaultColor.green() +
			defaultColor.blue() + 128) * 0.25;
		return QColor(refColor, refColor, refColor);
	}

	if(a_colorID == COLOR_ID_SELECTION_MATCHES)
		return QColor("#FFCCFF");

	if(a_colorID == COLOR_ID_TIMELINE_BOOKMARKS)
		return Qt::magenta;

	return defaultColor;
}

QColor SettingsManager::getColor(const QString & a_colorID) const
{
	QVariant colorValue = valueInGroup(inDarkMode() ? DARK_THEME_GROUP
		: THEME_GROUP, a_colorID);
	if(colorValue.isNull())
		return getDefaultColor(a_colorID);
	return QColor(colorValue.toString());
}

bool SettingsManager::setColor(const QString & a_colorID,
	const QColor & a_color)
{
	// Only record to settings if it doesn't match the default
	QColor defaultColor = getDefaultColor(a_colorID);
	if(defaultColor == a_color) return true;
	return setValueInGroup(inDarkMode() ? DARK_THEME_GROUP
		: THEME_GROUP, a_colorID, a_color.name());
}

//==============================================================================

QString SettingsManager::getLastUsedPath() const
{
	QStringList recentFilesList = getRecentFilesList();
	if(!recentFilesList.isEmpty())
		return recentFilesList.first();

	return QString();
}

bool SettingsManager::setLastUsedPath(const QString& a_lastUsedPath)
{
	return addToRecentFilesList(a_lastUsedPath);
}

//==============================================================================

QByteArray SettingsManager::getMainWindowGeometry() const
{
	return value(MAIN_WINDOW_GEOMETRY_KEY).toByteArray();
}

bool SettingsManager::setMainWindowGeometry(
	const QByteArray & a_mainWindowGeometry)
{
	return setValue(MAIN_WINDOW_GEOMETRY_KEY, a_mainWindowGeometry);
}

//==============================================================================

bool SettingsManager::getMainWindowMaximized() const
{
	return value(MAIN_WINDOW_MAXIMIZED_KEY,
		DEFAULT_MAIN_WINDOW_MAXIMIZED).toBool();
}

bool SettingsManager::setMainWindowMaximized(bool a_mainWindowMaximized)
{
	return setValue(MAIN_WINDOW_MAXIMIZED_KEY, a_mainWindowMaximized);
}

//==============================================================================

QByteArray SettingsManager::getPreviewDialogGeometry() const
{
	return value(PREVIEW_DIALOG_GEOMETRY_KEY).toByteArray();
}

bool SettingsManager::setPreviewDialogGeometry(
	const QByteArray & a_previewDialogGeometry)
{
	return setValue(PREVIEW_DIALOG_GEOMETRY_KEY, a_previewDialogGeometry);
}

//==============================================================================

bool SettingsManager::getPreviewDialogMaximized() const
{
	return value(PREVIEW_DIALOG_MAXIMIZED_KEY,
		DEFAULT_PREVIEW_DIALOG_MAXIMIZED).toBool();
}

bool SettingsManager::setPreviewDialogMaximized(bool a_previewDialogMaximized)
{
	return setValue(PREVIEW_DIALOG_MAXIMIZED_KEY, a_previewDialogMaximized);
}

//==============================================================================

QByteArray SettingsManager::getJobServerWatcherGeometry() const
{
	return value(JOB_SERVER_WATCHER_GEOMETRY_KEY).toByteArray();
}

bool SettingsManager::setJobServerWatcherGeometry(
	const QByteArray & a_geometry)
{
	return setValue(JOB_SERVER_WATCHER_GEOMETRY_KEY, a_geometry);
}

//==============================================================================

bool SettingsManager::getJobServerWatcherMaximized() const
{
	return value(JOB_SERVER_WATCHER_MAXIMIZED_KEY,
		DEFAULT_JOB_SERVER_WATCHER_MAXIMIZED).toBool();
}

bool SettingsManager::setJobServerWatcherMaximized(bool a_maximized)
{
	return setValue(JOB_SERVER_WATCHER_MAXIMIZED_KEY, a_maximized);
}

//==============================================================================

QByteArray SettingsManager::getJobsHeaderState() const
{
	return value(JOBS_HEADER_STATE_KEY).toByteArray();
}

bool SettingsManager::setJobsHeaderState(const QByteArray & a_headerState)
{
	return setValue(JOBS_HEADER_STATE_KEY, a_headerState);
}

//==============================================================================

bool SettingsManager::getAutoLoadLastScript() const
{
	return value(AUTO_LOAD_LAST_SCRIPT_KEY, DEFAULT_AUTO_LOAD_LAST_SCRIPT)
		.toBool();
}

bool SettingsManager::setAutoLoadLastScript(bool a_autoLoadLastScript)
{
	return setValue(AUTO_LOAD_LAST_SCRIPT_KEY, a_autoLoadLastScript);
}

//==============================================================================

bool SettingsManager::getZoomPanelVisible() const
{
	return value(ZOOM_PANEL_VISIBLE_KEY, DEFAULT_ZOOM_PANEL_VISIBLE).toBool();
}

bool SettingsManager::setZoomPanelVisible(bool a_zoomPanelVisible)
{
	return setValue(ZOOM_PANEL_VISIBLE_KEY, a_zoomPanelVisible);
}

//==============================================================================

ZoomMode SettingsManager::getZoomMode() const
{
	return (ZoomMode)value(ZOOM_MODE_KEY, (int)DEFAULT_ZOOM_MODE).toInt();
}

bool SettingsManager::setZoomMode(ZoomMode a_zoomMode)
{
	return setValue(ZOOM_MODE_KEY, (int)a_zoomMode);
}

//==============================================================================

double SettingsManager::getZoomRatio() const
{
	return value(ZOOM_RATIO_KEY, DEFAULT_ZOOM_RATIO).toDouble();
}

bool SettingsManager::setZoomRatio(double a_zoomRatio)
{
	return setValue(ZOOM_RATIO_KEY, a_zoomRatio);
}

//==============================================================================

Qt::TransformationMode SettingsManager::getScaleMode() const
{
	return (Qt::TransformationMode)value(SCALE_MODE_KEY,
		(int)DEFAULT_SCALE_MODE).toInt();
}

bool SettingsManager::setScaleMode(Qt::TransformationMode a_scaleMode)
{
	return setValue(SCALE_MODE_KEY, (int)a_scaleMode);
}

//==============================================================================

CropMode SettingsManager::getCropMode() const
{
	return (CropMode)value(CROP_MODE_KEY, (int)DEFAULT_CROP_MODE).toInt();
}

bool SettingsManager::setCropMode(CropMode a_cropMode)
{
	return setValue(CROP_MODE_KEY, (int)a_cropMode);
}

//==============================================================================

int SettingsManager::getCropZoomRatio() const
{
	return value(CROP_ZOOM_RATIO_KEY, DEFAULT_CROP_ZOOM_RATIO).toInt();
}

bool SettingsManager::setCropZoomRatio(int a_cropZoomRatio)
{
	return setValue(CROP_ZOOM_RATIO_KEY, a_cropZoomRatio);
}

//==============================================================================

bool SettingsManager::getPromptToSaveChanges() const
{
	return value(PROMPT_TO_SAVE_CHANGES_KEY,
		DEFAULT_PROMPT_TO_SAVE_CHANGES).toBool();
}

bool SettingsManager::setPromptToSaveChanges(bool a_prompt)
{
	return setValue(PROMPT_TO_SAVE_CHANGES_KEY, a_prompt);
}

//==============================================================================

QStringList SettingsManager::getRecentFilesList() const
{
	return value(RECENT_FILES_LIST_KEY).toStringList();
}

bool SettingsManager::addToRecentFilesList(const QString & a_filePath)
{
	QFileInfo fileInfo(a_filePath);
	QString canonicalPath = fileInfo.canonicalFilePath();
	QStringList recentFilesList = getRecentFilesList();
	recentFilesList.removeAll(canonicalPath);
	recentFilesList.prepend(canonicalPath);
	unsigned int maxRecentFilesNumber = getMaxRecentFilesNumber();
	while((unsigned int)recentFilesList.size() > maxRecentFilesNumber)
		recentFilesList.removeLast();
	return setValue(RECENT_FILES_LIST_KEY, recentFilesList);
}

//==============================================================================

unsigned int SettingsManager::getMaxRecentFilesNumber() const
{
	return value(MAX_RECENT_FILES_NUMBER_KEY,
		DEFAULT_MAX_RECENT_FILES_NUMBER).toUInt();
}

bool SettingsManager::setMaxRecentFilesNumber(
	unsigned int a_maxRecentFilesNumber)
{
	return setValue(MAX_RECENT_FILES_NUMBER_KEY, a_maxRecentFilesNumber);
}

//==============================================================================

int SettingsManager::getCharactersTypedToStartCompletion() const
{
	return value(CHARACTERS_TYPED_TO_START_COMPLETION_KEY,
		DEFAULT_CHARACTERS_TYPED_TO_START_COMPLETION).toInt();
}

bool SettingsManager::setCharactersTypedToStartCompletion(
	int a_charactersNumber)
{
	return setValue(CHARACTERS_TYPED_TO_START_COMPLETION_KEY,
		a_charactersNumber);
}

//==============================================================================

TimeLineSlider::DisplayMode SettingsManager::getTimeLineMode() const
{
	return (TimeLineSlider::DisplayMode)value(TIMELINE_MODE_KEY,
		(int)DEFAULT_TIMELINE_MODE).toInt();
}

bool SettingsManager::setTimeLineMode(
	TimeLineSlider::DisplayMode a_timeLineMode)
{
	return setValue(TIMELINE_MODE_KEY, (int)a_timeLineMode);
}

//==============================================================================

double SettingsManager::getTimeStep() const
{
	return value(TIME_STEP_KEY, DEFAULT_TIME_STEP).toDouble();
}

bool SettingsManager::setTimeStep(double a_timeStep)
{
	return setValue(TIME_STEP_KEY, a_timeStep);
}

//==============================================================================

bool SettingsManager::getColorPickerVisible() const
{
	return value(COLOR_PICKER_VISIBLE_KEY,
		DEFAULT_COLOR_PICKER_VISIBLE).toBool();
}

bool SettingsManager::setColorPickerVisible(bool a_colorPickerVisible)
{
	return setValue(COLOR_PICKER_VISIBLE_KEY, a_colorPickerVisible);
}

//==============================================================================

PlayFPSLimitMode SettingsManager::getPlayFPSLimitMode() const
{
	return (PlayFPSLimitMode)value(PLAY_FPS_LIMIT_MODE_KEY,
		(int)DEFAULT_PLAY_FPS_LIMIT_MODE).toInt();
}

bool SettingsManager::setPlayFPSLimitMode(PlayFPSLimitMode a_mode)
{
	return setValue(PLAY_FPS_LIMIT_MODE_KEY, (int)a_mode);
}

//==============================================================================

double SettingsManager::getPlayFPSLimit() const
{
	return value(PLAY_FPS_LIMIT_KEY, DEFAULT_PLAY_FPS_LIMIT).toDouble();
}

bool SettingsManager::setPlayFPSLimit(double a_limit)
{
	return setValue(PLAY_FPS_LIMIT_KEY, a_limit);
}

//==============================================================================

bool SettingsManager::getUseSpacesAsTab() const
{
	return value(USE_SPACES_AS_TAB_KEY, DEFAULT_USE_SPACES_AS_TAB).toBool();
}

bool SettingsManager::setUseSpacesAsTab(bool a_value)
{
	return setValue(USE_SPACES_AS_TAB_KEY, a_value);
}

//==============================================================================

int SettingsManager::getSpacesInTab() const
{
	return value(SPACES_IN_TAB_KEY, DEFAULT_SPACES_IN_TAB).toInt();
}

bool SettingsManager::setSpacesInTab(int a_spacesNumber)
{
	return setValue(SPACES_IN_TAB_KEY, a_spacesNumber);
}

//==============================================================================

QString SettingsManager::getTabText() const
{
	QString text = "\t";
	bool useSpacesAsTab = getUseSpacesAsTab();
	if(useSpacesAsTab)
	{
		int spacesInTab = getSpacesInTab();
		text.fill(' ', spacesInTab);
	}
	return text;
}

//==============================================================================

bool SettingsManager::getRememberLastPreviewFrame() const
{
	return value(REMEMBER_LAST_PREVIEW_FRAME_KEY,
		DEFAULT_REMEMBER_LAST_PREVIEW_FRAME).toBool();
}

bool SettingsManager::setRememberLastPreviewFrame(bool a_remember)
{
	return setValue(REMEMBER_LAST_PREVIEW_FRAME_KEY, a_remember);
}

//==============================================================================

int SettingsManager::getLastPreviewFrame(bool a_inPreviewer) const
{
	if(a_inPreviewer)
	{
		return valueInGroup(PREVIEWER_GROUP, LAST_PREVIEW_FRAME_KEY,
			DEFAULT_LAST_PREVIEW_FRAME).toInt();
	}
	return value(LAST_PREVIEW_FRAME_KEY, DEFAULT_LAST_PREVIEW_FRAME).toInt();
}

bool SettingsManager::setLastPreviewFrame(int a_frameNumber, bool a_inPreviewer)
{
	if(a_inPreviewer)
	{
		return setValueInGroup(PREVIEWER_GROUP,
			LAST_PREVIEW_FRAME_KEY, a_frameNumber);
	}
	return setValue(LAST_PREVIEW_FRAME_KEY, a_frameNumber);
}

//==============================================================================

QString SettingsManager::getDefaultNewScriptTemplate()
{
	return QString(
		"import vapoursynth as vs\n"
		"core = vs.core\n"
	);
}

QString SettingsManager::getNewScriptTemplate()
{
	return value(NEW_SCRIPT_TEMPLATE_KEY,
		getDefaultNewScriptTemplate()).toString();
}

bool SettingsManager::setNewScriptTemplate(const QString & a_text)
{
	return setValue(NEW_SCRIPT_TEMPLATE_KEY, a_text);
}

//==============================================================================

std::vector<CodeSnippet> SettingsManager::getAllCodeSnippets() const
{
	QSettings settings(m_settingsFilePath, QSettings::IniFormat);
	settings.beginGroup(CODE_SNIPPETS_GROUP);

	std::vector<CodeSnippet> snippets;

	QStringList snippetNames = settings.childKeys();
	for(const QString & snippetName : snippetNames)
	{
		CodeSnippet snippet(snippetName);
		snippet.text = settings.value(snippetName).toString();
		snippets.push_back(snippet);
	}

	return snippets;
}

CodeSnippet SettingsManager::getCodeSnippet(const QString & a_name) const
{
	QSettings settings(m_settingsFilePath, QSettings::IniFormat);
	settings.beginGroup(CODE_SNIPPETS_GROUP);

	CodeSnippet snippet;

	if(!settings.contains(a_name))
		return snippet;

	snippet.name = a_name;
	snippet.text = valueInGroup(CODE_SNIPPETS_GROUP, a_name).toString();

	return snippet;
}

bool SettingsManager::saveCodeSnippet(const CodeSnippet & a_snippet)
{
	return setValueInGroup(CODE_SNIPPETS_GROUP, a_snippet.name, a_snippet.text);
}

bool SettingsManager::deleteCodeSnippet(const QString & a_name)
{
	return deleteValueInGroup(CODE_SNIPPETS_GROUP, a_name);
}

//==============================================================================

std::vector<DropFileCategory> SettingsManager::getAllDropFileTemplates() const
{
	QSettings settings(m_settingsFilePath, QSettings::IniFormat);
	settings.beginGroup(DROP_FILE_TEMPLATES_GROUP);

	std::vector<DropFileCategory> categories;

	QStringList categoryNames = settings.childGroups();
	for(const QString & categoryName : categoryNames)
	{
		settings.beginGroup(categoryName);

		DropFileCategory category;
		category.name = categoryName;
		category.maskList =
			settings.value(DROP_FILE_CATEGORY_MASK_LIST_KEY).toStringList();
		category.sourceTemplate =
			settings.value(DROP_FILE_CATEGORY_SOURCE_TEMPLATE_KEY).toString();
		categories.push_back(category);

		settings.endGroup();
	}

	return categories;
}

bool SettingsManager::setDropFileTemplates(
	const std::vector<DropFileCategory> & a_categories)
{
	QSettings settings(m_settingsFilePath, QSettings::IniFormat);

	settings.remove(DROP_FILE_TEMPLATES_GROUP);
	settings.beginGroup(DROP_FILE_TEMPLATES_GROUP);

	for(const DropFileCategory & category : a_categories)
	{
		settings.beginGroup(category.name);
		settings.setValue(DROP_FILE_CATEGORY_MASK_LIST_KEY, category.maskList);
		settings.setValue(DROP_FILE_CATEGORY_SOURCE_TEMPLATE_KEY,
			category.sourceTemplate);
		settings.endGroup();
	}

	settings.sync();
	bool success = (QSettings::NoError == settings.status());
	return success;
}

QString SettingsManager::getDropFileTemplate(const QString & a_filePath) const
{
	QSettings settings(m_settingsFilePath, QSettings::IniFormat);
	settings.beginGroup(DROP_FILE_TEMPLATES_GROUP);

	QRegExp matcher;
	matcher.setPatternSyntax(QRegExp::Wildcard);
	matcher.setCaseSensitivity(Qt::CaseInsensitive);

	std::vector<DropFileCategory> categories = getAllDropFileTemplates();

	for(const DropFileCategory & category : categories)
	{
		for(const QString & mask : category.maskList)
		{
			matcher.setPattern(mask);
			if(matcher.exactMatch(a_filePath))
				return category.sourceTemplate;
		}
	}

	return QString(DEFAULT_DROP_FILE_TEMPLATE);
}

//==============================================================================

bool SettingsManager::getHighlightSelectionMatches() const
{
	return value(HIGHLIGHT_SELECTION_MATCHES_KEY,
		DEFAULT_HIGHLIGHT_SELECTION_MATCHES).toBool();
}

bool SettingsManager::setHighlightSelectionMatches(bool a_highlight)
{
	return setValue(HIGHLIGHT_SELECTION_MATCHES_KEY, a_highlight);
}

//==============================================================================

int SettingsManager::getHighlightSelectionMatchesMinLength() const
{
	return value(HIGHLIGHT_SELECTION_MATCHES_MIN_LENGTH_KEY,
		DEFAULT_HIGHLIGHT_SELECTION_MATCHES_MIN_LENGTH).toInt();
}

bool SettingsManager::setHighlightSelectionMatchesMinLength(int a_length)
{
	return setValue(HIGHLIGHT_SELECTION_MATCHES_MIN_LENGTH_KEY, a_length);
}

//==============================================================================

bool SettingsManager::getTimeLinePanelVisible() const
{
	return value(TIMELINE_PANEL_VISIBLE_KEY,
		DEFAULT_TIMELINE_PANEL_VISIBLE).toBool();
}

bool SettingsManager::setTimeLinePanelVisible(bool a_visible)
{
	return setValue(TIMELINE_PANEL_VISIBLE_KEY, a_visible);
}

//==============================================================================

bool SettingsManager::getAlwaysKeepCurrentFrame() const
{
	return value(ALWAYS_KEEP_CURRENT_FRAME_KEY,
		DEFAULT_ALWAYS_KEEP_CURRENT_FRAME).toBool();
}

bool SettingsManager::setAlwaysKeepCurrentFrame(bool a_keep)
{
	return setValue(ALWAYS_KEEP_CURRENT_FRAME_KEY, a_keep);
}

//==============================================================================

std::vector<TextBlockStyle> SettingsManager::getLogStyles(
	const QString & a_logName) const
{
	std::vector<TextBlockStyle> styles;

	if(a_logName.isEmpty())
		return styles;

	QSettings settings(m_settingsFilePath, QSettings::IniFormat);
	settings.beginGroup(LOGS_GROUP);

	QStringList logNames = settings.childGroups();
	if(!logNames.contains(a_logName))
		return styles;

	settings.beginGroup(a_logName);
	settings.beginGroup(LOG_STYLES_GROUP);

	QStringList styleNames = settings.childGroups();
	for(const QString & styleName : styleNames)
	{
		settings.beginGroup(styleName);

		TextBlockStyle style;
		style.name = styleName;
		style.title = settings.value(LOG_STYLE_TITLE_KEY).toString();
		if(settings.contains(LOG_STYLE_TEXT_FORMAT_KEY))
			style.textFormat = qvariant_cast<QTextFormat>(
				settings.value(LOG_STYLE_TEXT_FORMAT_KEY)).toCharFormat();
		style.isAlias = settings.value(LOG_STYLE_IS_ALIAS_KEY, false).toBool();
		style.originalStyleName =
			settings.value(LOG_STYLE_ORIGINAL_STYLE_NAME_KEY).toString();
		style.isVisible =
			settings.value(LOG_STYLE_IS_VISIBLE_KEY, true).toBool();

		styles.push_back(style);

		settings.endGroup();
	}

	return styles;
}

bool SettingsManager::setLogStyles(const QString & a_logName,
	const std::vector<TextBlockStyle> a_styles)
{
	if(a_logName.isEmpty())
		return false;

	if(a_styles.empty())
		return false;

	QSettings settings(m_settingsFilePath, QSettings::IniFormat);
	settings.beginGroup(LOGS_GROUP);
	settings.beginGroup(a_logName);
	settings.remove(LOG_STYLES_GROUP);
	settings.beginGroup(LOG_STYLES_GROUP);
	for(const TextBlockStyle & style : a_styles)
	{
		settings.beginGroup(style.name);
		settings.setValue(LOG_STYLE_TITLE_KEY, style.title);
		settings.setValue(LOG_STYLE_TEXT_FORMAT_KEY, style.textFormat);
		settings.setValue(LOG_STYLE_IS_ALIAS_KEY, style.isAlias);
		settings.setValue(LOG_STYLE_ORIGINAL_STYLE_NAME_KEY,
			style.originalStyleName);
		settings.setValue(LOG_STYLE_IS_VISIBLE_KEY, style.isVisible);
		settings.endGroup();
	}

	settings.sync();
	bool success = (QSettings::NoError == settings.status());
	return success;
}

//==============================================================================

QString SettingsManager::getLastSnapshotExtension() const
{
	return value(LAST_SNAPSHOT_EXTENSION_KEY,
		DEFAULT_LAST_SNAPSHOT_EXTENSION).toString();
}

bool SettingsManager::setLastSnapshotExtension(const QString & a_extension)
{
	return setValue(LAST_SNAPSHOT_EXTENSION_KEY, a_extension);
}

int SettingsManager::getPNGSnapshotCompressionLevel() const
{
	return value(PNG_COMPRESSION_LEVEL_KEY,
		DEFAULT_PNG_COMPRESSION_LEVEL).toInt();
}

bool SettingsManager::setPNGSnapshotCompressionLevel(int a_level)
{
	return setValue(PNG_COMPRESSION_LEVEL_KEY, a_level);
}

bool SettingsManager::getReloadBeforeExecution() const
{
	return value(RELOAD_BEFORE_EXECUTION_KEY,
		DEFAULT_RELOAD_BEFORE_EXECUTION).toBool();
}

bool SettingsManager::setReloadBeforeExecution(bool a_reload)
{
	return setValue(RELOAD_BEFORE_EXECUTION_KEY, a_reload);
}

bool SettingsManager::getDarkMode() const
{
#ifdef Q_OS_WIN
	return value(DARK_MODE_KEY, DEFAULT_DARK_MODE).toBool();
#else
	return false;
#endif
}

bool SettingsManager::setDarkMode(bool a_dark)
{
#ifdef Q_OS_WIN
	return setValue(DARK_MODE_KEY, a_dark);
#else
	return setValue(DARK_MODE_KEY, false);
#endif
}

bool SettingsManager::getSilentSnapshot() const
{
	return value(SILENT_SNAPSHOT_KEY, DEFAULT_SILENT_SNAPSHOT).toBool();
}

bool SettingsManager::setSilentSnapshot(bool a_set)
{
	return setValue(SILENT_SNAPSHOT_KEY, a_set);
}

QString SettingsManager::getSnapshotTemplate() const
{
	return value(SNAPSHOT_TEMPLATE_KEY, DEFAULT_SNAPSHOT_TEMPLATE).toString();
}

bool SettingsManager::setSnapshotTemplate(const QString & a_template)
{
	return setValue(SNAPSHOT_TEMPLATE_KEY, a_template);
}

//==============================================================================
