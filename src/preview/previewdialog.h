#ifndef PREVIEWDIALOG_H_INCLUDED
#define PREVIEWDIALOG_H_INCLUDED

#include <QPixmap>
#include <map>
#include <vector>
#include <deque>

#include "../settings/settingsmanager.h"
#include "timelineslider.h"

#include <ui_previewdialog.h>

class QEvent;
class QCloseEvent;
class QMoveEvent;
class QResizeEvent;
class QKeyEvent;
class QEvent;
class QStatusBar;
class QMenu;
class QActionGroup;
class QAction;
class VapourSynthScriptProcessor;
class SettingsDialog;
struct VSVideoInfo;
class PreviewAdvancedSettingsDialog;

struct NumberedPixmap
{
	int number;
	QPixmap pixmap;

	NumberedPixmap(int a_number, const QPixmap & a_pixmap);
	bool operator<(const NumberedPixmap & a_other) const;
};

class PreviewDialog : public QDialog
{
	Q_OBJECT

	public:

		PreviewDialog(
			VapourSynthScriptProcessor * a_pVapourSynthScriptProcessor,
			SettingsManager * a_pSettingsManager,
			SettingsDialog * a_pSettingsDialog,
			QWidget * a_pParent = nullptr);
		virtual ~PreviewDialog();

		void previewScript(const QString& a_script,
			const QString& a_scriptName);

		void clear();

	protected:

		void closeEvent(QCloseEvent * a_pEvent) override;

		void moveEvent(QMoveEvent * a_pEvent) override;

		void resizeEvent(QResizeEvent * a_pEvent) override;

		void changeEvent(QEvent * a_pEvent) override;

		void keyPressEvent(QKeyEvent * a_pEvent) override;

	signals:

		void signalWriteLogMessage(int a_messageType,
			const QString & a_message);

		void signalInsertLineIntoScript(const QString& a_line);

	private slots:

		void slotShowFrame(int a_frameNumber);

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

		void slotPreviewAreaMouseOverPoint(float a_normX, float a_normY);

		void slotFrameToClipboard();

		void slotAdvancedSettingsChanged();

		void slotToggleColorPicker(bool a_colorPickerVisible);

		void slotReceivePreviewFrame(int a_frameNumber,
			const QPixmap & a_pixmap);

		void slotFrameQueueStateChanged(size_t a_inQueue, size_t a_inProcess,
			size_t a_maxThreads);

		void slotPlay(bool a_play);

		void slotProcessPlayQueue();

	private:

		void createActionsAndMenus();

		void createStatusBar();

		void setUpZoomPanel();

		void setUpTimeLinePanel();

		void setUpCropPanel();

		void evaluateScript(const QString& a_script,
			const QString& a_scriptName);

		bool showFrame(int a_frameNumber);

		void setPreviewPixmap();

		void recalculateCropMods();

		void resetCropSpinBoxes();

		Ui::PreviewDialog m_ui;

		VapourSynthScriptProcessor * m_pVapourSynthScriptProcessor;

		SettingsManager * m_pSettingsManager;

		SettingsDialog * m_pSettingsDialog;

		PreviewAdvancedSettingsDialog * m_pAdvancedSettingsDialog;

		QStatusBar * m_pStatusBar;
		QLabel * m_pVideoInfoLabel;
		QLabel * m_pFramesInQueueLabel;
		QLabel * m_pFramesInProcessLabel;
		QLabel * m_pMaxThreadsLabel;

		int m_frameExpected;
		int m_frameShown;
		int m_lastFrameRequestedForPlay;

		int m_bigFrameStep;

		QString m_scriptName;

		QPixmap m_framePixmap;

		const VSVideoInfo * m_cpVideoInfo;

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

		std::map<QString, ZoomMode> m_actionIDToZoomModeMap;

		std::map<QString, Qt::TransformationMode> m_actionIDToZoomScaleModeMap;

		std::map<QString, TimeLineSlider::DisplayMode>
			m_actionIDToTimeLineModeMap;

		std::vector<QAction *> m_settableActionsList;

		size_t m_framesInQueue;
		size_t m_framesInProcess;
		size_t m_maxThreads;

		bool m_playing;
		bool m_processingPlayQueue;

		std::deque<NumberedPixmap> m_framePixmapsQueue;
		size_t m_cachedPixmapsLimit;
};

#endif // PREVIEWDIALOG_H_INCLUDED
