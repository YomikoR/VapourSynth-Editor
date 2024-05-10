#ifndef PREVIEWDIALOG_H_INCLUDED
#define PREVIEWDIALOG_H_INCLUDED

#include <ui_preview_dialog.h>

#include "../vapoursynth/vs_script_processor_dialog.h"
#include "../settings/settings_definitions.h"
#include "../chrono.h"

#include <QPixmap>
#include <QTextEdit>
#include <QIcon>
#include <QToolButton>
#include <QAudioSink>
#include <QIODevice>
#include <map>
#include <vector>
#include <chrono>

class QEvent;
class QMoveEvent;
class QResizeEvent;
class QKeyEvent;
class QMenu;
class QActionGroup;
class QAction;
class QTimer;
class SettingsManager;
class SettingsDialog;
class PreviewAdvancedSettingsDialog;
class VSNodeInfo;
class FramePropsPanel;

extern const char TIMELINE_BOOKMARKS_FILE_SUFFIX[];

class PreviewDialog : public VSScriptProcessorDialog
{
	Q_OBJECT

	struct AudioFrame
	{
		int number;
		int outputIndex;
		QByteArray data;

		AudioFrame();
		AudioFrame(int a_number, int a_outputIndex, QByteArray a_data);
		bool valid() const { return !!data.size(); }
		bool operator==(const AudioFrame & a_other) const;
	};

public:

	PreviewDialog(SettingsManager * a_pSettingsManager,
		VSScriptLibrary * a_pVSScriptLibrary,
		bool a_enableAudioPlayback = true,
		QWidget * a_pParent = nullptr);
	virtual ~PreviewDialog();

	virtual void setScriptName(const QString & a_scriptName) override;

	void previewScript(const QString& a_script,
		const QString& a_scriptName, int a_number = -1);

	bool busy() const
	{
		return VSScriptProcessorDialog::busy(m_outputIndex);
	}

signals:

protected slots:

	virtual void slotReceiveFrame(int a_frameNumber, int a_outputIndex,
		const VSFrame * a_cpOutputFrame,
		const VSFrame * a_cpPreviewFrame) override;

	virtual void slotFrameRequestDiscarded(int a_frameNumber,
		int a_outputIndex, const QString & a_reason) override;

	void slotShowFrame(int a_frameNumber, bool a_refreshCache = true);

	void slotSaveSnapshot();

	void slotToggleZoomPanelVisible(bool a_zoomPanelVisible);

	void slotZoomModeChanged();

	void slotZoomRatioChanged(double a_zoomRatio);

	void slotScaleModeChanged();

	void slotToggleCropPanelVisible(bool a_cropPanelVisible);

	void slotCropModeChanged();

	void slotCropLeftValueChanged(int a_value);

	void slotCropTopValueChanged(int a_value);

	void slotCropWidthValueChanged(int a_value);

	void slotCropHeightValueChanged(int a_value);

	void slotCropRightValueChanged(int a_value);

	void slotCropBottomValueChanged(int a_value);

	void slotCropZoomRatioValueChanged(int a_cropZoomRatio);

	void slotCallAdvancedSettingsDialog();

	void slotToggleTimeLinePanelVisible(bool a_timeLinePanelVisible);

	void slotTimeLineModeChanged();

	void slotTimeStepChanged(const QTime & a_time);

	void slotTimeStepForward();

	void slotTimeStepBack();

	void slotSettingsChanged();

	void slotPreviewAreaSizeChanged();

	void slotPreviewAreaCtrlWheel(QPoint a_angleDelta);

	void slotPreviewAreaMouseMiddleButtonReleased();

	void slotPreviewAreaMouseRightButtonReleased();

	void slotPreviewAreaMouseOverPoint(double a_pX, double a_pY);

	void slotFrameToClipboard();

	void slotAdvancedSettingsChanged();

	void slotToggleColorPicker(bool a_colorPickerVisible);

	void slotSetPlayFPSLimit();

	void slotPlay(bool a_play);

	void slotProcessPlayQueue();
	void slotProcessAudioPlayQueue();

	void slotLoadChapters();
	void slotClearBookmarks();
	void slotBookmarkCurrentFrame();
	void slotUnbookmarkCurrentFrame();
	void slotGoToPreviousBookmark();
	void slotGoToNextBookmark();

	void slotSaveGeometry();

	void slotJumpToFrame();

	void slotToggleFrameProps();

	void slotSwitchOutputIndex(int a_outputIndex);

	void slotSwitchOutputIndex0() { slotSwitchOutputIndex(0); }
	void slotSwitchOutputIndex1() { slotSwitchOutputIndex(1); }
	void slotSwitchOutputIndex2() { slotSwitchOutputIndex(2); }
	void slotSwitchOutputIndex3() { slotSwitchOutputIndex(3); }
	void slotSwitchOutputIndex4() { slotSwitchOutputIndex(4); }
	void slotSwitchOutputIndex5() { slotSwitchOutputIndex(5); }
	void slotSwitchOutputIndex6() { slotSwitchOutputIndex(6); }
	void slotSwitchOutputIndex7() { slotSwitchOutputIndex(7); }
	void slotSwitchOutputIndex8() { slotSwitchOutputIndex(8); }
	void slotSwitchOutputIndex9() { slotSwitchOutputIndex(9); }
	void slotSwitchOutputIndex10() { slotSwitchOutputIndex(10); }
	void slotSwitchOutputIndex11() { slotSwitchOutputIndex(11); }
	void slotSwitchOutputIndex12() { slotSwitchOutputIndex(12); }
	void slotSwitchOutputIndex13() { slotSwitchOutputIndex(13); }
	void slotSwitchOutputIndex14() { slotSwitchOutputIndex(14); }
	void slotSwitchOutputIndex15() { slotSwitchOutputIndex(15); }
	void slotSwitchOutputIndex16() { slotSwitchOutputIndex(16); }
	void slotSwitchOutputIndex17() { slotSwitchOutputIndex(17); }
	void slotSwitchOutputIndex18() { slotSwitchOutputIndex(18); }
	void slotSwitchOutputIndex19() { slotSwitchOutputIndex(19); }

	void slotAbout();
	void slotCallSettingsDialog();

protected:

	friend class FramePropsPanel;

	virtual void stopAndCleanUp() override;

	void moveEvent(QMoveEvent * a_pEvent) override;

	void resizeEvent(QResizeEvent * a_pEvent) override;

	void changeEvent(QEvent * a_pEvent) override;

	void closeEvent(QCloseEvent * a_pEvent) override;

	void keyPressEvent(QKeyEvent * a_pEvent) override;

	void createActionsAndMenus();

	void setUpZoomPanel();

	void setUpTimeLinePanel();

	void setUpCropPanel();

	bool requestShowFrame(int a_frameNumber);

	void setPreviewPixmap();

	void recalculateCropMods();

	void resetCropSpinBoxes();

	qlonglong frameToTimestamp(int a_frame);
	int timestampToFrame(qlonglong a_timestamp);

	void setExpectedFrame(int a_frame);

	bool useTimestamps() const;

	void setCurrentFrame(const VSFrame * a_cpOutputFrame,
		const VSFrame * a_cpPreviewFrame);

	void updateFrameProps(bool a_forced);

	double valueAtPoint(size_t a_x, size_t a_y, int a_plane);

	void previewValueAtPoint(size_t a_x, size_t a_y, int a_ret[]);

	QPixmap pixmapFromRGB(const VSFrame * a_cpFrame);

	void setTitle();

	void saveTimelineBookmarks();
	void loadTimelineBookmarks();

	void saveGeometryDelayed();

	QPoint loadLastScrollBarPositions() const;
	void saveLastScrollBarPositions();

	void setAudioOutput();
	void stopAudioOutput();
	void playAudioFrame();

	QByteArray readAudioFrame(const VSFrame * a_cpFrame);

	Ui::PreviewDialog m_ui;

	PreviewAdvancedSettingsDialog * m_pAdvancedSettingsDialog;

	SettingsDialog * m_pSettingsDialog;

	int64_t m_fpsNum = 0;
	int64_t m_fpsDen = 0;

	int m_frameExpected;
	qlonglong m_frameTimestampExpected;
	int m_frameShown;
	int m_lastFrameRequestedForPlay;

	int m_bigFrameStep;

	const VSFrame * m_cpFrame;
	const VSFrame * m_cpPreviewFrame;
	QPixmap m_framePixmap;

	bool m_changingCropValues;

	QMenu * m_pPreviewContextMenu;
	QAction * m_pActionFrameToClipboard;
	QAction * m_pActionSaveSnapshot;
	QAction * m_pActionToggleZoomPanel;
	QMenu * m_pMenuZoomModes;
	QActionGroup * m_pActionGroupZoomModes;
	QAction * m_pActionSetZoomModeNoZoom;
	QAction * m_pActionSetZoomModeFixedRatio;
	QAction * m_pActionSetZoomModeFitToFrame;
	QMenu * m_pMenuZoomScaleModes;
	QActionGroup * m_pActionGroupZoomScaleModes;
	QAction * m_pActionSetZoomScaleModeNearest;
	QAction * m_pActionSetZoomScaleModeBilinear;
	QAction * m_pActionToggleCropPanel;
	QAction * m_pActionToggleTimeLinePanel;
	QMenu * m_pMenuTimeLineModes;
	QActionGroup * m_pActionGroupTimeLineModes;
	QAction * m_pActionSetTimeLineModeTime;
	QAction * m_pActionSetTimeLineModeFrames;
	QAction * m_pActionTimeStepForward;
	QAction * m_pActionTimeStepBack;
	QAction * m_pActionPasteCropSnippetIntoScript;
	QAction * m_pActionAdvancedSettingsDialog;
	QAction * m_pActionSettingsDialog;
	QAction * m_pActionToggleColorPicker;
	QAction * m_pActionPlay;
	QAction * m_pActionLoadChapters;
	QAction * m_pActionClearBookmarks;
	QAction * m_pActionBookmarkCurrentFrame;
	QAction * m_pActionUnbookmarkCurrentFrame;
	QAction * m_pActionGoToPreviousBookmark;
	QAction * m_pActionGoToNextBookmark;
	QAction * m_pActionJumpToFrame;
	QAction * m_pActionToggleFramePropsPanel;
	QAction * m_pActionSwitchToOutputIndex0;
	QAction * m_pActionSwitchToOutputIndex1;
	QAction * m_pActionSwitchToOutputIndex2;
	QAction * m_pActionSwitchToOutputIndex3;
	QAction * m_pActionSwitchToOutputIndex4;
	QAction * m_pActionSwitchToOutputIndex5;
	QAction * m_pActionSwitchToOutputIndex6;
	QAction * m_pActionSwitchToOutputIndex7;
	QAction * m_pActionSwitchToOutputIndex8;
	QAction * m_pActionSwitchToOutputIndex9;
	QAction * m_pActionSwitchToOutputIndex10;
	QAction * m_pActionSwitchToOutputIndex11;
	QAction * m_pActionSwitchToOutputIndex12;
	QAction * m_pActionSwitchToOutputIndex13;
	QAction * m_pActionSwitchToOutputIndex14;
	QAction * m_pActionSwitchToOutputIndex15;
	QAction * m_pActionSwitchToOutputIndex16;
	QAction * m_pActionSwitchToOutputIndex17;
	QAction * m_pActionSwitchToOutputIndex18;
	QAction * m_pActionSwitchToOutputIndex19;
	QAction * m_pActionAbout;

	std::map<QString, ZoomMode> m_actionIDToZoomModeMap;

	std::map<QString, Qt::TransformationMode> m_actionIDToZoomScaleModeMap;

	std::map<QString, TimeLineSlider::DisplayMode>
		m_actionIDToTimeLineModeMap;

	std::vector<QAction *> m_settableActionsList;

	bool m_playing;
	bool m_processingPlayQueue;
	bool m_nativePlaybackRate;
	double m_secondsBetweenFrames;
	hr_time_point m_lastFrameShowTime;
	QTimer * m_pPlayTimer;
	QIcon m_iconPlay;
	QIcon m_iconPause;
	QIcon m_iconPreview;

	QTimer * m_pGeometrySaveTimer;
	QByteArray m_windowGeometry;

	bool m_detailedTitle;

	qreal m_devicePixelRatio;

	FramePropsPanel * m_pFramePropsPanel;

	// fake buttons to hold hotkeys
	QToolButton * m_fakeButton1 = nullptr;
	QToolButton * m_fakeButton2 = nullptr;

	// audio
	bool m_audioPlaybackEnabled;
	bool m_currentIsAudio;
	QAudioSink * m_pAudioSink = nullptr;
	QIODevice * m_pAudioIODevice = nullptr;
	std::map<int, AudioFrame> m_audioCache;
	QTimer * m_pAudioPlayTimer = nullptr;
	double m_audioVolume = 1.0;
};

class FramePropsPanel: public QTextEdit
{
	Q_OBJECT

public:
	FramePropsPanel(SettingsManager * a_pSettingsManager,
		PreviewDialog * a_pFakeParent);

	void setVisible(bool visible) override;

	void keyPressEvent(QKeyEvent * a_pEvent) override;

public slots:
	void slotHide() { setVisible(false); }

private:
	PreviewDialog * m_pFakeParent;
	QAction * m_pActionHide;
	int m_widgetWidth = 400;
	int m_widgetHeight = 400;

	void setHideAction(SettingsManager * a_pSettingsManager);
};

#endif // PREVIEWDIALOG_H_INCLUDED
