#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include "settings_manager_core.h"
#include "settings_definitions.h"

#include <QByteArray>
#include <QTextCharFormat>
#include <QColor>
#include <QAction>

//==============================================================================

class SettingsManager : public SettingsManagerCore
{
public:

	SettingsManager(QObject * a_pParent);
	virtual ~SettingsManager();

	//----------------------------------------------------------------------

	QAction * createStandardAction(const QString & a_actionID,
		QObject * a_pParent);

	std::vector<StandardAction> getStandardActions() const;

	QKeySequence getDefaultHotkey(const QString & a_actionID) const;

	QKeySequence getHotkey(const QString & a_actionID) const;

	bool setHotkey(const QString & a_actionID,
		const QKeySequence & a_hotkey);

	//----------------------------------------------------------------------

	QTextCharFormat getDefaultTextFormat(const QString & a_textFormatID)
		const;

	QTextCharFormat getTextFormat(const QString & a_textFormatID) const;

	bool setTextFormat(const QString & a_textFormatID,
		const QTextCharFormat & a_format);

	//----------------------------------------------------------------------

	QColor getDefaultColor(const QString & a_colorID) const;

	QColor getColor(const QString & a_colorID) const;

	bool setColor(const QString & a_colorID, const QColor & a_color);

	//----------------------------------------------------------------------

	QByteArray getPreviewDialogGeometry() const;

	bool setPreviewDialogGeometry(const QByteArray & a_previewDialogGeometry);

	bool getPreviewDialogMaximized() const;

	bool setPreviewDialogMaximized(bool a_previewDialogMaximized);

	QPoint getLastPreviewScrollBarPositions() const;

	bool setLastPreviewScrollBarPositions(const QPoint &pos);

	bool getZoomPanelVisible() const;

	bool setZoomPanelVisible(bool a_zoomPanelVisible);

	ZoomMode getZoomMode() const;

	bool setZoomMode(ZoomMode a_zoomMode);

	double getZoomRatio() const;

	bool setZoomRatio(double a_zoomRatio);

	Qt::TransformationMode getScaleMode() const;

	bool setScaleMode(Qt::TransformationMode a_scaleMode);

	CropMode getCropMode() const;

	bool setCropMode(CropMode a_cropMode);

	int getCropZoomRatio() const;

	bool setCropZoomRatio(int a_cropZoomRatio);

	TimeLineSlider::DisplayMode getTimeLineMode() const;

	bool setTimeLineMode(TimeLineSlider::DisplayMode a_timeLineMode);

	double getTimeStep() const;

	bool setTimeStep(double a_timeStep);

	bool getColorPickerVisible() const;

	bool setColorPickerVisible(bool a_colorPickerVisible);

	PlayFPSLimitMode getPlayFPSLimitMode() const;

	bool setPlayFPSLimitMode(PlayFPSLimitMode a_mode);

	double getPlayFPSLimit() const;

	bool setPlayFPSLimit(double a_limit);

	int getLastPreviewFrame() const;

	bool setLastPreviewFrame(int a_frameNumber);

	bool getTimeLinePanelVisible() const;

	bool setTimeLinePanelVisible(bool a_visible);

	QString getLastSnapshotExtension() const;

	bool setLastSnapshotExtension(const QString & a_extension);

	int getPNGSnapshotCompressionLevel() const;

	bool setPNGSnapshotCompressionLevel(int a_level);

	bool getShowDebugMessages() const;

	bool setShowDebugMessages(bool a_debug);

	bool getSilentSnapshot() const;

	bool setSilentSnapshot(bool a_set);

	QString getSnapshotTemplate() const;

	bool setSnapshotTemplate(const QString & a_template);

private:

	void initializeStandardActions();

	std::vector<StandardAction> m_standardActions;

};

//==============================================================================

#endif // SETTINGSMANAGER_H
