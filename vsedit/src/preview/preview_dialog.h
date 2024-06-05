#ifndef PREVIEWDIALOG_H_INCLUDED
#define PREVIEWDIALOG_H_INCLUDED

#include <ui_preview_dialog.h>

#include "../vapoursynth/vs_script_processor_dialog.h"
#include "../../../common-src/settings/settings_definitions.h"
#include "../../../common-src/chrono.h"

#include <QPixmap>
#include <QTextEdit>
#include <QIcon>
#ifdef Q_OS_WIN // AUDIO
#include <QAudioSink>
#include <QIODevice>
#endif
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

#ifdef Q_OS_WIN // AUDIO
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
#endif

public:

	PreviewDialog(SettingsManager * a_pSettingsManager,
		VSScriptLibrary * a_pVSScriptLibrary, bool a_inPreviewer = false,
		QWidget * a_pParent = nullptr);
	virtual ~PreviewDialog();

	virtual void setScriptName(const QString & a_scriptName) override;

	void previewScript(const QString& a_script,
		const QString& a_scriptName);

	bool busy() const
	{
		return VSScriptProcessorDialog::busy(m_outputIndex);
	}

signals:

	void signalPasteIntoScriptAtNewLine(const QString& a_line);
	void signalPasteIntoScriptAtCursor(const QString& a_line);

public slots:

	void slotScriptTextChanged();

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

	void slotPasteCropSnippetIntoScript();

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
#ifdef Q_OS_WIN // AUDIO
	void slotProcessAudioPlayQueue();
#endif

	void slotLoadChapters();
	void slotClearBookmarks();
	void slotBookmarkCurrentFrame();
	void slotUnbookmarkCurrentFrame();
	void slotGoToPreviousBookmark();
	void slotGoToNextBookmark();

	void slotPasteShownFrameNumberIntoScript();

	void slotSaveGeometry();

	void slotJumpToFrame();

	void slotToggleFrameProps();

	void slotSwitchOutputIndex(int a_outputIndex);

	void setOutputIndex(int a_index);

#define SLOT_SWITCH_OUTPUT_INDEX(a) \
	void slotSwitchOutputIndex##a() { setOutputIndex(a); }

	SLOT_SWITCH_OUTPUT_INDEX(0)
	SLOT_SWITCH_OUTPUT_INDEX(1)
	SLOT_SWITCH_OUTPUT_INDEX(2)
	SLOT_SWITCH_OUTPUT_INDEX(3)
	SLOT_SWITCH_OUTPUT_INDEX(4)
	SLOT_SWITCH_OUTPUT_INDEX(5)
	SLOT_SWITCH_OUTPUT_INDEX(6)
	SLOT_SWITCH_OUTPUT_INDEX(7)
	SLOT_SWITCH_OUTPUT_INDEX(8)
	SLOT_SWITCH_OUTPUT_INDEX(9)
	SLOT_SWITCH_OUTPUT_INDEX(10)
	SLOT_SWITCH_OUTPUT_INDEX(11)
	SLOT_SWITCH_OUTPUT_INDEX(12)
	SLOT_SWITCH_OUTPUT_INDEX(13)
	SLOT_SWITCH_OUTPUT_INDEX(14)
	SLOT_SWITCH_OUTPUT_INDEX(15)
	SLOT_SWITCH_OUTPUT_INDEX(16)
	SLOT_SWITCH_OUTPUT_INDEX(17)
	SLOT_SWITCH_OUTPUT_INDEX(18)
	SLOT_SWITCH_OUTPUT_INDEX(19)

#undef SLOT_SWITCH_OUTPUT_INDEX

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

#ifdef Q_OS_WIN // AUDIO
	void setAudioOutput();
	void stopAudioOutput();
	void playAudioFrame();

	QByteArray readAudioFrame(const VSFrame * a_cpFrame);
#endif

	Ui::PreviewDialog m_ui;

	PreviewAdvancedSettingsDialog * m_pAdvancedSettingsDialog;

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
	QAction * m_pActionToggleColorPicker;
	QAction * m_pActionPlay;
	QAction * m_pActionLoadChapters;
	QAction * m_pActionClearBookmarks;
	QAction * m_pActionBookmarkCurrentFrame;
	QAction * m_pActionUnbookmarkCurrentFrame;
	QAction * m_pActionGoToPreviousBookmark;
	QAction * m_pActionGoToNextBookmark;
	QAction * m_pActionPasteShownFrameNumberIntoScript;
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

	bool m_alwaysKeepCurrentFrame;

	QTimer * m_pGeometrySaveTimer;
	QByteArray m_windowGeometry;

	qreal m_devicePixelRatio;

	FramePropsPanel * m_pFramePropsPanel;

	bool m_toChangeTitle;
	bool m_scriptTextChanged = false;

	bool m_inPreviewer;

#ifdef Q_OS_WIN // AUDIO
	bool m_currentIsAudio;
	QAudioSink * m_pAudioSink = nullptr;
	QIODevice * m_pAudioIODevice = nullptr;
	std::map<int, AudioFrame> m_audioCache;
	QTimer * m_pAudioPlayTimer = nullptr;
	double m_audioVolume = 1.0;
#endif
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
