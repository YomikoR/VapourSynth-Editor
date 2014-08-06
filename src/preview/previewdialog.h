#ifndef PREVIEWDIALOG_H_INCLUDED
#define PREVIEWDIALOG_H_INCLUDED

#include <QPixmap>
#include <map>
#include <vector>

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
class SettingsManager;
class SettingsDialog;
struct VSVideoInfo;

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

		void slotFrameToClipboard();

	private:

		void createActionsAndMenus();

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

		QStatusBar * m_pStatusBar;

		int m_currentFrame;

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

		std::map<QString, ZoomMode> m_actionIDToZoomMode;

		std::map<QString, Qt::TransformationMode> m_actionIDToZoomScaleMode;

		std::map<QString, TimeLineSlider::DisplayMode> m_actionIDToTimeLineMode;

		std::vector<QAction *> m_settableActionsList;
};

#endif // PREVIEWDIALOG_H_INCLUDED
