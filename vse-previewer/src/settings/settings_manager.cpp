#include "settings_manager.h"

#include "../helpers.h"

#include <QSettings>
#include <QFileInfo>
#include <QPalette>
#include <QFontMetricsF>

//==============================================================================

const char PREVIEW_DIALOG_GEOMETRY_KEY[] = "prewiew_dialog_geometry";
const char LAST_PREVIEW_SCROLLBAR_POS_X[] = "last_preview_scrollbar_position_x";
const char LAST_PREVIEW_SCROLLBAR_POS_Y[] = "last_preview_scrollbar_position_y";
const char PREVIEW_DIALOG_MAXIMIZED_KEY[] = "preview_dialog_maximized";
const char ADDITIONAL_TITLE_INFO_KEY[] = "show_additional_info_on_title";
const char ZOOM_PANEL_VISIBLE_KEY[] = "zoom_panel_visible";
const char ZOOM_MODE_KEY[] = "zoom_mode";
const char ZOOM_RATIO_KEY[] = "zoom_ratio";
const char SCALE_MODE_KEY[] = "scale_mode";
const char CROP_MODE_KEY[] = "crop_mode";
const char CROP_ZOOM_RATIO_KEY[] = "crop_zoom_ratio";
const char TIMELINE_MODE_KEY[] = "timeline_mode";
const char TIME_STEP_KEY[] = "time_step_mode";
const char COLOR_PICKER_VISIBLE_KEY[] = "color_picker_visible";
const char PLAY_FPS_LIMIT_MODE_KEY[] = "play_fps_limit_mode";
const char PLAY_FPS_LIMIT_KEY[] = "play_fps_limit";
const char LAST_PREVIEW_FRAME_KEY[] = "last_preview_frame";
const char LAST_PREVIEW_TIMESTAMP_KEY[] = "last_preview_timestamp";
const char SYNC_OUTPUT_MODE_KEY[] = "sync_output_node_mode";
const char TIMELINE_PANEL_VISIBLE_KEY[] = "timeline_panel_visible";
const char LAST_SNAPSHOT_EXTENSION_KEY[] = "last_snapshot_extension";
const char PNG_COMPRESSION_LEVEL_KEY[] = "png_compression_level";
const char DEBUG_MESSAGES_KEY[] = "show_debug_messages";
const char SILENT_SNAPSHOT_KEY[] = "silent_snapshot";
const char SNAPSHOT_TEMPLATE_KEY[] = "snapshot_template";

//==============================================================================

const char HOTKEYS_GROUP[] = "hotkeys";

//==============================================================================

const char THEME_GROUP[] = "theme";

//==============================================================================

const char PREVIEW_GROUP[] = "preview";

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
}

SettingsManager::~SettingsManager()
{

}

//==============================================================================

void SettingsManager::initializeStandardActions()
{
	m_standardActions =
	{
		{ACTION_ID_SETTINGS, tr("Additional settings"), QIcon(":settings.png"),
			QKeySequence()},
		{ACTION_ID_ABOUT, tr("About..."), QIcon(), QKeySequence()},
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
			tr("Preview settings"), QIcon(":settings.png"),
			QKeySequence()},
		{ACTION_ID_TOGGLE_COLOR_PICKER, tr("Color panel"),
			QIcon(":color_picker.png"), QKeySequence()},
		{ACTION_ID_PLAY, tr("Play"),
			QIcon(":play.png"), QKeySequence(Qt::Key_Space)},
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
		{ACTION_ID_JUMP_TO_FRAME, tr("Jump to frame..."),
			QIcon(), QKeySequence(Qt::Key_J)},
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

		QPalette defaultPalette;
		defaultFormat.setForeground(
			defaultPalette.color(QPalette::WindowText));
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
	QVariant textFormatValue = valueInGroup(THEME_GROUP, a_textFormatID);
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
	return setValueInGroup(THEME_GROUP, a_textFormatID, vsedit::toByteArray(a_format));
}

//==============================================================================

QColor SettingsManager::getDefaultColor(const QString & a_colorID) const
{
	QColor defaultColor;

	if(a_colorID == COLOR_ID_TIMELINE_BOOKMARKS)
		return Qt::magenta;

	return defaultColor;
}

QColor SettingsManager::getColor(const QString & a_colorID) const
{
	QVariant colorValue = valueInGroup(THEME_GROUP, a_colorID);
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
	return setValueInGroup(THEME_GROUP, a_colorID, a_color.name());
}

//==============================================================================

QByteArray SettingsManager::getPreviewDialogGeometry() const
{
	return valueInGroup(PREVIEW_GROUP,
		PREVIEW_DIALOG_GEOMETRY_KEY).toByteArray();
}

bool SettingsManager::setPreviewDialogGeometry(
	const QByteArray & a_previewDialogGeometry)
{
	return setValueInGroup(PREVIEW_GROUP,
		PREVIEW_DIALOG_GEOMETRY_KEY, a_previewDialogGeometry);
}

//==============================================================================

bool SettingsManager::getPreviewDialogMaximized() const
{
	return valueInGroup(PREVIEW_GROUP,
		PREVIEW_DIALOG_MAXIMIZED_KEY,
		DEFAULT_PREVIEW_DIALOG_MAXIMIZED).toBool();
}

bool SettingsManager::setPreviewDialogMaximized(bool a_previewDialogMaximized)
{
	return setValueInGroup(PREVIEW_GROUP,
		PREVIEW_DIALOG_MAXIMIZED_KEY, a_previewDialogMaximized);
}

bool SettingsManager::getShowAdditionalTitleInfo() const
{
    return valueInGroup(PREVIEW_GROUP,
		ADDITIONAL_TITLE_INFO_KEY, DEFAULT_ADDITIONAL_TITLE_INFO).toBool();
}

bool SettingsManager::setShowAdditionalTitleInfo(bool a_show)
{
    return setValueInGroup(PREVIEW_GROUP,
		ADDITIONAL_TITLE_INFO_KEY, a_show);
}

QPoint SettingsManager::getLastPreviewScrollBarPositions() const
{
	int x = valueInGroup(PREVIEW_GROUP, LAST_PREVIEW_SCROLLBAR_POS_X, 0).toInt();
	int y = valueInGroup(PREVIEW_GROUP, LAST_PREVIEW_SCROLLBAR_POS_Y, 0).toInt();
	return QPoint(x, y);
}

bool SettingsManager::setLastPreviewScrollBarPositions(const QPoint &pos)
{
	int x = pos.x();
	int y = pos.y();
    return setValueInGroup(PREVIEW_GROUP, LAST_PREVIEW_SCROLLBAR_POS_X, x)
		&& setValueInGroup(PREVIEW_GROUP, LAST_PREVIEW_SCROLLBAR_POS_Y, y);
}

//==============================================================================

bool SettingsManager::getZoomPanelVisible() const
{
	return valueInGroup(PREVIEW_GROUP,
		ZOOM_PANEL_VISIBLE_KEY, DEFAULT_ZOOM_PANEL_VISIBLE).toBool();
}

bool SettingsManager::setZoomPanelVisible(bool a_zoomPanelVisible)
{
	return setValueInGroup(PREVIEW_GROUP,
		ZOOM_PANEL_VISIBLE_KEY, a_zoomPanelVisible);
}

//==============================================================================

ZoomMode SettingsManager::getZoomMode() const
{
	return (ZoomMode)valueInGroup(PREVIEW_GROUP,
		ZOOM_MODE_KEY, (int)DEFAULT_ZOOM_MODE).toInt();
}

bool SettingsManager::setZoomMode(ZoomMode a_zoomMode)
{
	return setValueInGroup(PREVIEW_GROUP,
		ZOOM_MODE_KEY, (int)a_zoomMode);
}

//==============================================================================

double SettingsManager::getZoomRatio() const
{
	return valueInGroup(PREVIEW_GROUP,
		ZOOM_RATIO_KEY, DEFAULT_ZOOM_RATIO).toDouble();
}

bool SettingsManager::setZoomRatio(double a_zoomRatio)
{
	return setValueInGroup(PREVIEW_GROUP, ZOOM_RATIO_KEY, a_zoomRatio);
}

//==============================================================================

Qt::TransformationMode SettingsManager::getScaleMode() const
{
	return (Qt::TransformationMode)valueInGroup(PREVIEW_GROUP,
		SCALE_MODE_KEY, (int)DEFAULT_SCALE_MODE).toInt();
}

bool SettingsManager::setScaleMode(Qt::TransformationMode a_scaleMode)
{
	return setValueInGroup(PREVIEW_GROUP, SCALE_MODE_KEY, (int)a_scaleMode);
}

//==============================================================================

CropMode SettingsManager::getCropMode() const
{
	return (CropMode)valueInGroup(PREVIEW_GROUP,
		CROP_MODE_KEY, (int)DEFAULT_CROP_MODE).toInt();
}

bool SettingsManager::setCropMode(CropMode a_cropMode)
{
	return setValueInGroup(PREVIEW_GROUP, CROP_MODE_KEY, (int)a_cropMode);
}

//==============================================================================

int SettingsManager::getCropZoomRatio() const
{
	return valueInGroup(PREVIEW_GROUP,
		CROP_ZOOM_RATIO_KEY, DEFAULT_CROP_ZOOM_RATIO).toInt();
}

bool SettingsManager::setCropZoomRatio(int a_cropZoomRatio)
{
	return setValueInGroup(PREVIEW_GROUP,
		CROP_ZOOM_RATIO_KEY, a_cropZoomRatio);
}

//==============================================================================

TimeLineSlider::DisplayMode SettingsManager::getTimeLineMode() const
{
	return (TimeLineSlider::DisplayMode)valueInGroup(PREVIEW_GROUP,
		TIMELINE_MODE_KEY, (int)DEFAULT_TIMELINE_MODE).toInt();
}

bool SettingsManager::setTimeLineMode(
	TimeLineSlider::DisplayMode a_timeLineMode)
{
	return setValueInGroup(PREVIEW_GROUP,
		TIMELINE_MODE_KEY, (int)a_timeLineMode);
}

//==============================================================================

double SettingsManager::getTimeStep() const
{
	return valueInGroup(PREVIEW_GROUP,
		TIME_STEP_KEY, DEFAULT_TIME_STEP).toDouble();
}

bool SettingsManager::setTimeStep(double a_timeStep)
{
	return setValueInGroup(PREVIEW_GROUP,
		TIME_STEP_KEY, a_timeStep);
}

//==============================================================================

bool SettingsManager::getColorPickerVisible() const
{
	return valueInGroup(PREVIEW_GROUP,
		COLOR_PICKER_VISIBLE_KEY, DEFAULT_COLOR_PICKER_VISIBLE).toBool();
}

bool SettingsManager::setColorPickerVisible(bool a_colorPickerVisible)
{
	return setValueInGroup(PREVIEW_GROUP,
		COLOR_PICKER_VISIBLE_KEY, a_colorPickerVisible);
}

//==============================================================================

PlayFPSLimitMode SettingsManager::getPlayFPSLimitMode() const
{
	return (PlayFPSLimitMode)valueInGroup(PREVIEW_GROUP,
		PLAY_FPS_LIMIT_MODE_KEY, (int)DEFAULT_PLAY_FPS_LIMIT_MODE).toInt();
}

bool SettingsManager::setPlayFPSLimitMode(PlayFPSLimitMode a_mode)
{
	return setValueInGroup(PREVIEW_GROUP,
		PLAY_FPS_LIMIT_MODE_KEY, (int)a_mode);
}

//==============================================================================

double SettingsManager::getPlayFPSLimit() const
{
	return valueInGroup(PREVIEW_GROUP,
		PLAY_FPS_LIMIT_KEY, DEFAULT_PLAY_FPS_LIMIT).toDouble();
}

bool SettingsManager::setPlayFPSLimit(double a_limit)
{
	return setValueInGroup(PREVIEW_GROUP,
		PLAY_FPS_LIMIT_KEY, a_limit);
}

//==============================================================================

int SettingsManager::getLastPreviewFrame() const
{
	return valueInGroup(PREVIEW_GROUP, LAST_PREVIEW_FRAME_KEY,
		DEFAULT_LAST_PREVIEW_FRAME).toInt();
}

bool SettingsManager::setLastPreviewFrame(int a_frameNumber)
{
	return setValueInGroup(PREVIEW_GROUP,
		LAST_PREVIEW_FRAME_KEY, a_frameNumber);
}

qlonglong SettingsManager::getLastPreviewTimestamp() const
{
	return valueInGroup(PREVIEW_GROUP, LAST_PREVIEW_TIMESTAMP_KEY,
		DEFAULT_LAST_PREVIEW_TIMESTAMP).toLongLong();
}

bool SettingsManager::setLastPreviewTimestamp(qlonglong a_ms)
{
	return setValueInGroup(PREVIEW_GROUP,
		LAST_PREVIEW_TIMESTAMP_KEY, a_ms);
}

SyncOutputNodesMode SettingsManager::getSyncOutputMode() const
{
	return (SyncOutputNodesMode)valueInGroup(PREVIEW_GROUP,
		SYNC_OUTPUT_MODE_KEY, (int)DEFAULT_SYNC_OUTPUT_MODE).toInt();
}

bool SettingsManager::setSyncOutputMode(SyncOutputNodesMode a_mode)
{
	return setValueInGroup(PREVIEW_GROUP,
		SYNC_OUTPUT_MODE_KEY, (int)a_mode);
}

//==============================================================================

bool SettingsManager::getTimeLinePanelVisible() const
{
	return valueInGroup(PREVIEW_GROUP,
		TIMELINE_PANEL_VISIBLE_KEY, DEFAULT_TIMELINE_PANEL_VISIBLE).toBool();
}

bool SettingsManager::setTimeLinePanelVisible(bool a_visible)
{
	return setValueInGroup(PREVIEW_GROUP,
		TIMELINE_PANEL_VISIBLE_KEY, a_visible);
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

bool SettingsManager::getShowDebugMessages() const
{
    return value(DEBUG_MESSAGES_KEY, DEFAULT_DEBUG_MESSAGES).toBool();
}

bool SettingsManager::setShowDebugMessages(bool a_debug)
{
    return setValue(DEBUG_MESSAGES_KEY, a_debug);
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
