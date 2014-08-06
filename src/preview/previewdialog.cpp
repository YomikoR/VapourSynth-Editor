#include <QEvent>
#include <QCloseEvent>
#include <QMoveEvent>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QStatusBar>
#include <QLabel>
#include <QToolTip>
#include <QCursor>
#include <QStandardPaths>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollBar>
#include <QPoint>
#include <QMenu>
#include <QActionGroup>
#include <QAction>
#include <QByteArray>
#include <QClipboard>
#include <cassert>
#include <algorithm>
#include <cmath>

#include "../common/helpers.h"
#include "../vapoursynth/vapoursynthscriptprocessor.h"
#include "../settings/settingsdialog.h"
#include "scrollnavigator.h"
#include "timelineslider.h"

#include "previewdialog.h"

//==============================================================================

#define BEGIN_CROP_VALUES_CHANGE \
	if(m_changingCropValues) \
		return; \
	m_changingCropValues = true;

#define END_CROP_VALUES_CHANGE \
	m_changingCropValues = false;

//==============================================================================

PreviewDialog::PreviewDialog(
	VapourSynthScriptProcessor * a_pVapourSynthScriptProcessor,
	SettingsManager * a_pSettingsManager,
	SettingsDialog * a_pSettingsDialog,  QWidget * a_pParent) :
	QDialog(a_pParent, (Qt::WindowFlags)0
		| Qt::Window
		| Qt::CustomizeWindowHint
		| Qt::WindowMinimizeButtonHint
		| Qt::WindowMaximizeButtonHint
		| Qt::WindowCloseButtonHint)
	, m_pVapourSynthScriptProcessor(a_pVapourSynthScriptProcessor)
	, m_pSettingsManager(a_pSettingsManager)
	, m_pSettingsDialog(a_pSettingsDialog)
	, m_pStatusBar(nullptr)
	, m_currentFrame(0)
	, m_bigFrameStep(10)
	, m_scriptName()
	, m_framePixmap()
	, m_cpVideoInfo(nullptr)
	, m_changingCropValues(false)
	, m_pPreviewContextMenu(nullptr)
	, m_pActionFrameToClipboard(nullptr)
	, m_pActionSaveSnapshot(nullptr)
	, m_pActionToggleZoomPanel(nullptr)
	, m_pMenuZoomModes(nullptr)
	, m_pActionGroupZoomModes(nullptr)
	, m_pActionSetZoomModeNoZoom(nullptr)
	, m_pActionSetZoomModeFixedRatio(nullptr)
	, m_pActionSetZoomModeFitToFrame(nullptr)
	, m_pMenuZoomScaleModes(nullptr)
	, m_pActionGroupZoomScaleModes(nullptr)
	, m_pActionSetZoomScaleModeNearest(nullptr)
	, m_pActionSetZoomScaleModeBilinear(nullptr)
	, m_pActionToggleCropPanel(nullptr)
	, m_pActionToggleTimeLinePanel(nullptr)
	, m_pMenuTimeLineModes(nullptr)
	, m_pActionGroupTimeLineModes(nullptr)
	, m_pActionSetTimeLineModeTime(nullptr)
	, m_pActionSetTimeLineModeFrames(nullptr)
	, m_pActionTimeStepForward(nullptr)
	, m_pActionTimeStepBack(nullptr)
	, m_pActionPasteCropSnippetIntoScript(nullptr)
	, m_actionIDToZoomMode()
	, m_actionIDToZoomScaleMode()
	, m_actionIDToTimeLineMode()
	, m_settableActionsList()
{
	m_ui.setupUi(this);
	setWindowIcon(QIcon(":preview.png"));

	createActionsAndMenus();

	m_ui.frameNumberSlider->setBigStep(m_bigFrameStep);
	m_ui.frameNumberSlider->setDisplayMode(
		m_pSettingsManager->getTimeLineMode());

	m_ui.frameToClipboardButton->setDefaultAction(m_pActionFrameToClipboard);
	m_ui.saveSnapshotButton->setDefaultAction(m_pActionSaveSnapshot);

	setUpZoomPanel();
	setUpCropPanel();
	setUpTimeLinePanel();

	m_pStatusBar = new QStatusBar(this);
	m_ui.mainLayout->addWidget(m_pStatusBar);

	QByteArray newGeometry = m_pSettingsManager->getPreviewDialogGeometry();
	if(!newGeometry.isEmpty())
		restoreGeometry(newGeometry);

	connect(m_ui.frameNumberSlider, SIGNAL(signalFrameChanged(int)),
		this, SLOT(slotShowFrame(int)));
	connect(m_ui.frameNumberSpinBox, SIGNAL(valueChanged(int)),
		this, SLOT(slotShowFrame(int)));
	connect(m_ui.previewArea, SIGNAL(signalSizeChanged()),
		this, SLOT(slotPreviewAreaSizeChanged()));
	connect(m_ui.previewArea, SIGNAL(signalCtrlWheel(QPoint)),
		this, SLOT(slotPreviewAreaCtrlWheel(QPoint)));
	connect(m_ui.previewArea, SIGNAL(signalMouseMiddleButtonReleased()),
		this, SLOT(slotPreviewAreaMouseMiddleButtonReleased()));
	connect(m_ui.previewArea, SIGNAL(signalMouseRightButtonReleased()),
		this, SLOT(slotPreviewAreaMouseRightButtonReleased()));
	connect(m_pSettingsDialog, SIGNAL(signalSettingsChanged()),
		this, SLOT(slotSettingsChanged()));
}

// END OF PreviewDialog::PreviewDialog(SettingsManager * a_pSettingsManager,
//		QWidget * a_pParent, Qt::WindowFlags a_flags)
//==============================================================================

PreviewDialog::~PreviewDialog()
{
	m_pVapourSynthScriptProcessor->finalize();
}

// END OF PreviewDialog::~PreviewDialog()
//==============================================================================

void PreviewDialog::previewScript(const QString& a_script,
	const QString& a_scriptName)
{
	m_ui.cropCheckButton->setChecked(false);

	bool initialized =
		m_pVapourSynthScriptProcessor->initialize(a_script, a_scriptName);
	if(!initialized)
		return;

	QString title = "Preview - ";
	title += a_scriptName;
	setWindowTitle(title);

	m_cpVideoInfo = m_pVapourSynthScriptProcessor->videoInfo();
	assert(m_cpVideoInfo);

	int lastFrameNumber = m_cpVideoInfo->numFrames - 1;
	m_ui.frameNumberSpinBox->setMaximum(lastFrameNumber);
	m_ui.frameNumberSlider->setFramesNumber(m_cpVideoInfo->numFrames);
	if(m_cpVideoInfo->fpsDen == 0)
		m_ui.frameNumberSlider->setFPS(0.0);
	else
	{
		m_ui.frameNumberSlider->setFPS((double)m_cpVideoInfo->fpsNum /
			(double)m_cpVideoInfo->fpsDen);
	}

	if(m_currentFrame > lastFrameNumber)
		m_currentFrame = lastFrameNumber;

	QString newVideoInfoString = vsedit::videoInfoString(m_cpVideoInfo);
	m_pStatusBar->showMessage(newVideoInfoString);
	m_pStatusBar->setToolTip(newVideoInfoString);

	resetCropSpinBoxes();

	m_scriptName = a_scriptName;

	if(m_pSettingsManager->getPreviewDialogMaximized())
		showMaximized();
	else
		showNormal();

	slotShowFrame(m_currentFrame);
}

// END OF void PreviewDialog::previewScript(const QString& a_script,
//		const QString& a_scriptName)
//==============================================================================

void PreviewDialog::closeEvent(QCloseEvent * a_pEvent)
{
	m_pVapourSynthScriptProcessor->finalize();
	QDialog::closeEvent(a_pEvent);
}

// END OF void PreviewDialog::closeEvent(QCloseEvent * a_pEvent)
//==============================================================================

void PreviewDialog::moveEvent(QMoveEvent * a_pEvent)
{
	QDialog::moveEvent(a_pEvent);
	QApplication::processEvents();
	if(!isMaximized())
		m_pSettingsManager->setPreviewDialogGeometry(saveGeometry());
}

// END OF void PreviewDialog::moveEvent(QMoveEvent * a_pEvent)
//==============================================================================

void PreviewDialog::resizeEvent(QResizeEvent * a_pEvent)
{
	QDialog::resizeEvent(a_pEvent);
	QApplication::processEvents();
	if(!isMaximized())
		m_pSettingsManager->setPreviewDialogGeometry(saveGeometry());
}

// END OF void PreviewDialog::resizeEvent(QResizeEvent * a_pEvent)
//==============================================================================

void PreviewDialog::changeEvent(QEvent * a_pEvent)
{
	QDialog::changeEvent(a_pEvent);
	if(a_pEvent->type() == QEvent::WindowStateChange)
	{
		if(isMaximized())
			m_pSettingsManager->setPreviewDialogMaximized(true);
		else
			m_pSettingsManager->setPreviewDialogMaximized(false);
	}
}

// END OF void PreviewDialog::changeEvent(QEvent * a_pEvent)
//==============================================================================

void PreviewDialog::keyPressEvent(QKeyEvent * a_pEvent)
{
	Qt::KeyboardModifiers modifiers = a_pEvent->modifiers();

	if(modifiers != Qt::NoModifier)
	{
		QDialog::keyPressEvent(a_pEvent);
		return;
	}

	if(!m_pVapourSynthScriptProcessor->isInitialized())
	{
		QDialog::keyPressEvent(a_pEvent);
		return;
	}
	assert(m_cpVideoInfo);

	int key = a_pEvent->key();

	if((key == Qt::Key_Left) || (key == Qt::Key_Down))
		slotShowFrame(std::max(0, m_currentFrame - 1));
	else if((key == Qt::Key_Right) || (key == Qt::Key_Up))
	{
		slotShowFrame(std::min(m_cpVideoInfo->numFrames - 1,
			m_currentFrame + 1));
	}
	else if(key == Qt::Key_PageDown)
		slotShowFrame(std::max(0, m_currentFrame - m_bigFrameStep));
	else if(key == Qt::Key_PageUp)
	{
		slotShowFrame(std::min(m_cpVideoInfo->numFrames - 1,
			m_currentFrame + m_bigFrameStep));
	}
	else if(key == Qt::Key_Home)
		slotShowFrame(0);
	else if(key == Qt::Key_End)
		slotShowFrame(m_cpVideoInfo->numFrames - 1);
	else if(key == Qt::Key_Escape)
		close();
	else
		QDialog::keyPressEvent(a_pEvent);
}

// END OF void PreviewDialog::keyPressEvent(QKeyEvent * a_pEvent)
//==============================================================================

void PreviewDialog::slotShowFrame(int a_frameNumber)
{
	static bool requestingFrame = false;
	if(requestingFrame)
		return;
	requestingFrame = true;

	m_ui.frameNumberSpinBox->setValue(a_frameNumber);
	m_ui.frameNumberSlider->setFrame(a_frameNumber);

	bool frameShown = showFrame(a_frameNumber);
	if(frameShown)
		m_currentFrame = a_frameNumber;
	else
	{
		m_ui.frameNumberSpinBox->setValue(m_currentFrame);
		m_ui.frameNumberSlider->setFrame(m_currentFrame);
	}

	requestingFrame = false;
}
// END OF void PreviewDialog::slotFrameNumberSliderMoved(int a_position)
//==============================================================================

void PreviewDialog::slotSaveSnapshot()
{
	QString snapshotFilePath = m_scriptName;
	if(snapshotFilePath.isEmpty())
	{
		snapshotFilePath =
			QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
		snapshotFilePath += QString("/%1.png").arg(m_currentFrame);
	}
	else
		snapshotFilePath += QString(" - %1.png").arg(m_currentFrame);

	snapshotFilePath = QFileDialog::getSaveFileName(this,
		trUtf8("Save frame as image"),
		snapshotFilePath,
		trUtf8("PNG image (*.png);;All files (*)"));

	if(!snapshotFilePath.isEmpty())
	{
		bool success = m_framePixmap.save(snapshotFilePath, "PNG");
		if(!success)
		{
			QMessageBox::critical(this, trUtf8("Image save error"),
				trUtf8("Error while saving image ") + snapshotFilePath);
		}
	}
}

// END OF void PreviewDialog::slotSaveSnapshot()
//==============================================================================

void PreviewDialog::slotToggleZoomPanelVisible(bool a_zoomPanelVisible)
{
	m_ui.zoomPanel->setVisible(a_zoomPanelVisible);
	m_pSettingsManager->setZoomPanelVisible(a_zoomPanelVisible);
}

// END OF void PreviewDialog::slotToggleZoomPanelVisible(
//		bool a_zoomPanelVisible)
//==============================================================================

void PreviewDialog::slotZoomModeChanged()
{
	static bool changingZoomMode = false;
	if(changingZoomMode)
		return;
	changingZoomMode = true;

	ZoomMode zoomMode = (ZoomMode)m_ui.zoomModeComboBox->currentData().toInt();

	QObject * pSender = sender();
	if(pSender == m_ui.zoomModeComboBox)
	{

		for(QAction * pAction : m_pActionGroupZoomModes->actions())
		{
			ZoomMode actionZoomMode =
				m_actionIDToZoomMode[pAction->data().toString()];
			if(actionZoomMode == zoomMode)
			{
				pAction->setChecked(true);
				break;
			}
		}
	}
	else
	{
		// If signal wasn't sent by combo box - presume it was sent by action.
		QAction * pSenderAction = qobject_cast<QAction *>(pSender);
		zoomMode = m_actionIDToZoomMode[pSenderAction->data().toString()];
		int zoomModeIndex = m_ui.zoomModeComboBox->findData((int)zoomMode);
		m_ui.zoomModeComboBox->setCurrentIndex(zoomModeIndex);
	}

	setPreviewPixmap();
	bool fixedRatio(zoomMode == ZoomMode::FixedRatio);
	m_ui.zoomRatioSpinBox->setEnabled(fixedRatio);
	bool noZoom = (zoomMode == ZoomMode::NoZoom);
	m_ui.scaleModeComboBox->setEnabled(!noZoom);
	m_pMenuZoomScaleModes->setEnabled(!noZoom);
	m_pSettingsManager->setZoomMode(zoomMode);

	changingZoomMode = false;
}

// END OF void PreviewDialog::slotZoomModeChanged()
//==============================================================================

void PreviewDialog::slotZoomRatioChanged(double a_zoomRatio)
{
	setPreviewPixmap();
	m_pSettingsManager->setZoomRatio(a_zoomRatio);
}

// END OF void PreviewDialog::slotZoomRateChanged(double a_zoomRatio)
//==============================================================================

void PreviewDialog::slotScaleModeChanged()
{
	static bool changingScaleMode = false;
	if(changingScaleMode)
		return;
	changingScaleMode = true;

	Qt::TransformationMode scaleMode = (Qt::TransformationMode)
		m_ui.scaleModeComboBox->currentData().toInt();

	QObject * pSender = sender();
	if(pSender == m_ui.scaleModeComboBox)
	{
		for(QAction * pAction : m_pActionGroupZoomScaleModes->actions())
		{
			Qt::TransformationMode actionScaleMode =
				m_actionIDToZoomScaleMode[pAction->data().toString()];
			if(actionScaleMode == scaleMode)
			{
				pAction->setChecked(true);
				break;
			}
		}
	}
	else
	{
		// If signal wasn't sent by combo box - presume it was sent by action.
		QAction * pSenderAction = qobject_cast<QAction *>(pSender);
		scaleMode =
			m_actionIDToZoomScaleMode[pSenderAction->data().toString()];
		int scaleModeIndex = m_ui.scaleModeComboBox->findData((int)scaleMode);
		m_ui.scaleModeComboBox->setCurrentIndex(scaleModeIndex);
	}

	setPreviewPixmap();
	m_pSettingsManager->setScaleMode(scaleMode);

	changingScaleMode = false;
}

// END OF void PreviewDialog::slotScaleModeChanged()
//==============================================================================

void PreviewDialog::slotToggleCropPanelVisible(bool a_cropPanelVisible)
{
	m_ui.cropPanel->setVisible(a_cropPanelVisible);
	setPreviewPixmap();
}

// END OF void PreviewDialog::slotToggleCropPanelVisible(
//		bool a_cropPanelVisible)
//==============================================================================

void PreviewDialog::slotCropModeChanged()
{
	CropMode cropMode = (CropMode)m_ui.cropModeComboBox->currentData().toInt();
	if(cropMode == CropMode::Absolute)
	{
		m_ui.cropWidthSpinBox->setEnabled(true);
		m_ui.cropHeightSpinBox->setEnabled(true);
		m_ui.cropRightSpinBox->setEnabled(false);
		m_ui.cropBottomSpinBox->setEnabled(false);
	}
	else
	{
		m_ui.cropWidthSpinBox->setEnabled(false);
		m_ui.cropHeightSpinBox->setEnabled(false);
		m_ui.cropRightSpinBox->setEnabled(true);
		m_ui.cropBottomSpinBox->setEnabled(true);
	}

	m_pSettingsManager->setCropMode(cropMode);
}

// END OF void PreviewDialog::slotCropModeChanged()
//==============================================================================

void PreviewDialog::slotCropLeftValueChanged(int a_value)
{
	BEGIN_CROP_VALUES_CHANGE

	int remainder = m_cpVideoInfo->width - a_value;
	m_ui.cropWidthSpinBox->setMaximum(remainder);
	m_ui.cropRightSpinBox->setMaximum(remainder - 1);

	CropMode cropMode = (CropMode)m_ui.cropModeComboBox->currentData().toInt();
	if(cropMode == CropMode::Absolute)
	{
		if(m_ui.cropWidthSpinBox->value() > remainder)
			m_ui.cropWidthSpinBox->setValue(remainder);
		m_ui.cropRightSpinBox->setValue(remainder -
			m_ui.cropWidthSpinBox->value());
	}
	else
	{
		if(m_ui.cropRightSpinBox->value() > remainder - 1)
			m_ui.cropRightSpinBox->setValue(remainder - 1);
		m_ui.cropWidthSpinBox->setValue(remainder -
			m_ui.cropRightSpinBox->value());
	}

	recalculateCropMods();

	setPreviewPixmap();

	m_ui.previewArea->slotScrollLeft();

	END_CROP_VALUES_CHANGE
}

// END OF void PreviewDialog::slotCropLeftValueChanged(int a_value)
//==============================================================================

void PreviewDialog::slotCropTopValueChanged(int a_value)
{
	BEGIN_CROP_VALUES_CHANGE

	int remainder = m_cpVideoInfo->height - a_value;
	m_ui.cropHeightSpinBox->setMaximum(remainder);
	m_ui.cropBottomSpinBox->setMaximum(remainder - 1);

	CropMode cropMode = (CropMode)m_ui.cropModeComboBox->currentData().toInt();
	if(cropMode == CropMode::Absolute)
	{
		if(m_ui.cropHeightSpinBox->value() > remainder)
			m_ui.cropHeightSpinBox->setValue(remainder);
		m_ui.cropBottomSpinBox->setValue(remainder -
			m_ui.cropHeightSpinBox->value());
	}
	else
	{
		if(m_ui.cropBottomSpinBox->value() > remainder - 1)
			m_ui.cropBottomSpinBox->setValue(remainder - 1);
		m_ui.cropHeightSpinBox->setValue(remainder -
			m_ui.cropBottomSpinBox->value());
	}

	recalculateCropMods();

	setPreviewPixmap();

	m_ui.previewArea->slotScrollTop();

	END_CROP_VALUES_CHANGE
}

// END OF void PreviewDialog::slotCropTopValueChanged(int a_value)
//==============================================================================

void PreviewDialog::slotCropWidthValueChanged(int a_value)
{
	BEGIN_CROP_VALUES_CHANGE

	m_ui.cropRightSpinBox->setValue(m_cpVideoInfo->width -
		m_ui.cropLeftSpinBox->value() - a_value);

	recalculateCropMods();

	setPreviewPixmap();

	m_ui.previewArea->slotScrollRight();

	END_CROP_VALUES_CHANGE
}

// END OF void PreviewDialog::slotCropWidthValueChanged(int a_value)
//==============================================================================

void PreviewDialog::slotCropHeightValueChanged(int a_value)
{
	BEGIN_CROP_VALUES_CHANGE

	m_ui.cropBottomSpinBox->setValue(m_cpVideoInfo->height -
		m_ui.cropTopSpinBox->value() - a_value);

	recalculateCropMods();

	setPreviewPixmap();

	m_ui.previewArea->slotScrollBottom();

	END_CROP_VALUES_CHANGE
}

// END OF void PreviewDialog::slotCropHeightValueChanged(int a_value)
//==============================================================================

void PreviewDialog::slotCropRightValueChanged(int a_value)
{
	BEGIN_CROP_VALUES_CHANGE

	m_ui.cropWidthSpinBox->setValue(m_cpVideoInfo->width -
		m_ui.cropLeftSpinBox->value() - a_value);

	recalculateCropMods();

	setPreviewPixmap();

	m_ui.previewArea->slotScrollRight();

	END_CROP_VALUES_CHANGE
}

// END OF void PreviewDialog::slotCropRightValueChanged(int a_value)
//==============================================================================

void PreviewDialog::slotCropBottomValueChanged(int a_value)
{
	BEGIN_CROP_VALUES_CHANGE

	m_ui.cropHeightSpinBox->setValue(m_cpVideoInfo->height -
		m_ui.cropTopSpinBox->value() - a_value);

	recalculateCropMods();

	setPreviewPixmap();

	m_ui.previewArea->slotScrollBottom();

	END_CROP_VALUES_CHANGE
}

// END OF void PreviewDialog::slotCropBottomValueChanged(int a_value)
//==============================================================================

void PreviewDialog::slotCropZoomRatioValueChanged(int a_cropZoomRatio)
{
	m_pSettingsManager->setCropZoomRatio(a_cropZoomRatio);
	setPreviewPixmap();
}

// END OF void PreviewDialog::slotCropZoomRatioValueChanged(int a_cropZoomRatio)
//==============================================================================

void PreviewDialog::slotPasteCropSnippetIntoScript()
{
	if(!m_ui.cropPanel->isVisible())
		return;

	CropMode cropMode = (CropMode)m_ui.cropModeComboBox->currentData().toInt();
	if(cropMode == CropMode::Absolute)
	{
		QString cropString = QString("***CLIP*** = core.std.CropAbs"
			"(***CLIP***, x=%1, y=%2, width=%3, height=%4)")
			.arg(m_ui.cropLeftSpinBox->value())
			.arg(m_ui.cropTopSpinBox->value())
			.arg(m_ui.cropWidthSpinBox->value())
			.arg(m_ui.cropHeightSpinBox->value());
		emit signalInsertLineIntoScript(cropString);
	}
	else
	{
		QString cropString = QString("***CLIP*** = core.std.CropRel"
			"(***CLIP***, left=%1, top=%2, right=%3, bottom=%4)")
			.arg(m_ui.cropLeftSpinBox->value())
			.arg(m_ui.cropTopSpinBox->value())
			.arg(m_ui.cropRightSpinBox->value())
			.arg(m_ui.cropBottomSpinBox->value());
		emit signalInsertLineIntoScript(cropString);
	}
}

// END OF void PreviewDialog::slotPasteCropSnippetIntoScript()
//==============================================================================

void PreviewDialog::slotToggleTimeLinePanelVisible(bool a_timeLinePanelVisible)
{
	m_ui.timeLinePanel->setVisible(a_timeLinePanelVisible);
}

// END OF void PreviewDialog::slotPasteCropSnippetIntoScript()
//==============================================================================

void PreviewDialog::slotTimeLineModeChanged()
{
	static bool changingTimeLineMode = false;
	if(changingTimeLineMode)
		return;
	changingTimeLineMode = true;

	TimeLineSlider::DisplayMode timeLineMode = (TimeLineSlider::DisplayMode)
		m_ui.timeLineModeComboBox->currentData().toInt();

	QObject * pSender = sender();
	if(pSender == m_ui.timeLineModeComboBox)
	{
		for(QAction * pAction : m_pActionGroupTimeLineModes->actions())
		{
			TimeLineSlider::DisplayMode actionTimeLineMode =
				m_actionIDToTimeLineMode[pAction->data().toString()];
			if(actionTimeLineMode == timeLineMode)
			{
				pAction->setChecked(true);
				break;
			}
		}
	}
	else
	{
		// If signal wasn't sent by combo box - presume it was sent by action.
		QAction * pSenderAction = qobject_cast<QAction *>(pSender);
		timeLineMode =
			m_actionIDToTimeLineMode[pSenderAction->data().toString()];
		int timeLineModeIndex = m_ui.timeLineModeComboBox->findData(
			(int)timeLineMode);
		m_ui.timeLineModeComboBox->setCurrentIndex(timeLineModeIndex);
	}

	m_ui.frameNumberSlider->setDisplayMode(timeLineMode);
	m_pSettingsManager->setTimeLineMode(timeLineMode);

	changingTimeLineMode = false;
}

// END OF void PreviewDialog::slotTimeLineModeChanged()
//==============================================================================

void PreviewDialog::slotTimeStepChanged(const QTime & a_time)
{
	m_pSettingsManager->setTimeStep(vsedit::qtimeToSeconds(a_time));
}

// END OF void PreviewDialog::slotTimeStepChanged(const QTime & a_time)
//==============================================================================

void PreviewDialog::slotTimeStepForward()
{
	double step = vsedit::qtimeToSeconds(m_ui.timeStepEdit->time());
	m_ui.frameNumberSlider->slotStepBySeconds(step);
}

// END OF void PreviewDialog::slotTimeStepForward()
//==============================================================================

void PreviewDialog::slotTimeStepBack()
{
	double step = vsedit::qtimeToSeconds(m_ui.timeStepEdit->time());
	m_ui.frameNumberSlider->slotStepBySeconds(-step);
}

// END OF void PreviewDialog::slotTimeStepBack()
//==============================================================================

void PreviewDialog::slotSettingsChanged()
{
	QKeySequence hotkey;
	for(QAction * pAction : m_settableActionsList)
	{
		hotkey = m_pSettingsManager->getHotkey(pAction->data().toString());
		pAction->setShortcut(hotkey);
	}
}

// END OF void PreviewDialog::slotSettingsChanged()
//==============================================================================

void PreviewDialog::slotPreviewAreaSizeChanged()
{
	ZoomMode zoomMode = (ZoomMode)m_ui.zoomModeComboBox->currentData().toInt();
	if(zoomMode == ZoomMode::FitToFrame)
		setPreviewPixmap();
}

// END OF void PreviewDialog::slotPreviewAreaSizeChanged()
//==============================================================================

void PreviewDialog::slotPreviewAreaCtrlWheel(QPoint a_angleDelta)
{
	ZoomMode zoomMode = (ZoomMode)m_ui.zoomModeComboBox->currentData().toInt();
	int deltaY = a_angleDelta.y();

	if(m_ui.cropCheckButton->isChecked())
	{
		if(deltaY > 0)
			m_ui.cropZoomRatioSpinBox->stepBy(1);
		else if(deltaY < 0)
			m_ui.cropZoomRatioSpinBox->stepBy(-1);
	}
	else if(zoomMode == ZoomMode::FixedRatio)
	{
		if(deltaY > 0)
			m_ui.zoomRatioSpinBox->stepBy(1);
		else if(deltaY < 0)
			m_ui.zoomRatioSpinBox->stepBy(-1);
	}
}

// END OF void PreviewDialog::slotPreviewAreaCtrlWheel(QPoint a_angleDelta)
//==============================================================================

void PreviewDialog::slotPreviewAreaMouseMiddleButtonReleased()
{
	if(m_ui.cropCheckButton->isChecked())
		return;

	int zoomModeIndex = m_ui.zoomModeComboBox->currentIndex();
	zoomModeIndex++;
	if(zoomModeIndex >= m_ui.zoomModeComboBox->count())
		zoomModeIndex = 0;
	m_ui.zoomModeComboBox->setCurrentIndex(zoomModeIndex);
}

// END OF void PreviewDialog::slotPreviewAreaMouseMiddleButtonReleased()
//==============================================================================

void PreviewDialog::slotPreviewAreaMouseRightButtonReleased()
{
	m_pPreviewContextMenu->exec(QCursor::pos());
}

// END OF void PreviewDialog::slotPreviewAreaMouseRightButtonReleased()
//==============================================================================

void PreviewDialog::slotFrameToClipboard()
{
	if(m_framePixmap.isNull())
		return;

	QClipboard * pClipboard = QApplication::clipboard();
	pClipboard->setPixmap(m_framePixmap);
}

// END OF void PreviewDialog::slotFrameToClipboard()
//==============================================================================

void PreviewDialog::createActionsAndMenus()
{
	QKeySequence hotkey;

//------------------------------------------------------------------------------

	m_pPreviewContextMenu = new QMenu(this);

//------------------------------------------------------------------------------

	m_pActionFrameToClipboard = new QAction(this);
	m_pActionFrameToClipboard->setIconText(
		trUtf8("Copy frame to clipboard"));
	m_pActionFrameToClipboard->setIcon(QIcon(":image_to_clipboard.png"));
	m_pPreviewContextMenu->addAction(m_pActionSaveSnapshot);
	hotkey = m_pSettingsManager->getHotkey(ACTION_ID_FRAME_TO_CLIPBOARD);
	m_pActionFrameToClipboard->setShortcut(hotkey);
	m_pActionFrameToClipboard->setData(ACTION_ID_FRAME_TO_CLIPBOARD);
	m_settableActionsList.push_back(m_pActionFrameToClipboard);

//------------------------------------------------------------------------------

	m_pActionSaveSnapshot = new QAction(this);
	m_pActionSaveSnapshot->setIconText(trUtf8("Save snapshot"));
	m_pActionSaveSnapshot->setIcon(QIcon(":snapshot.png"));
	m_pPreviewContextMenu->addAction(m_pActionSaveSnapshot);
	hotkey = m_pSettingsManager->getHotkey(ACTION_ID_SAVE_SNAPSHOT);
	m_pActionSaveSnapshot->setShortcut(hotkey);
	m_pActionSaveSnapshot->setData(ACTION_ID_SAVE_SNAPSHOT);
	m_settableActionsList.push_back(m_pActionSaveSnapshot);

//------------------------------------------------------------------------------

	m_pActionToggleZoomPanel = new QAction(this);
	m_pActionToggleZoomPanel->setIconText(trUtf8("Show zoom panel"));
	m_pActionToggleZoomPanel->setIcon(QIcon(":zoom.png"));
	m_pActionToggleZoomPanel->setCheckable(true);
	m_pActionToggleZoomPanel->setChecked(
		m_pSettingsManager->getZoomPanelVisible());
	m_pPreviewContextMenu->addAction(m_pActionToggleZoomPanel);
	hotkey = m_pSettingsManager->getHotkey(ACTION_ID_TOGGLE_ZOOM_PANEL);
	m_pActionToggleZoomPanel->setShortcut(hotkey);
	m_pActionToggleZoomPanel->setData(ACTION_ID_TOGGLE_ZOOM_PANEL);
	m_settableActionsList.push_back(m_pActionToggleZoomPanel);

//------------------------------------------------------------------------------

	m_pMenuZoomModes = new QMenu(m_pPreviewContextMenu);
	m_pMenuZoomModes->setTitle(trUtf8("Zoom mode"));
	m_pPreviewContextMenu->addMenu(m_pMenuZoomModes);

//------------------------------------------------------------------------------

	m_pActionGroupZoomModes = new QActionGroup(this);

//------------------------------------------------------------------------------

	m_pActionSetZoomModeNoZoom = new QAction(this);
	m_pActionSetZoomModeNoZoom->setIconText(trUtf8("Zoom: No zoom"));
	m_pActionSetZoomModeNoZoom->setIcon(QIcon(":zoom_no_zoom.png"));
	m_pActionSetZoomModeNoZoom->setCheckable(true);
	m_pActionSetZoomModeNoZoom->setActionGroup(m_pActionGroupZoomModes);
	m_pMenuZoomModes->addAction(m_pActionSetZoomModeNoZoom);
	hotkey = m_pSettingsManager->getHotkey(ACTION_ID_SET_ZOOM_MODE_NO_ZOOM);
	m_pActionSetZoomModeNoZoom->setShortcut(hotkey);
	m_pActionSetZoomModeNoZoom->setData(ACTION_ID_SET_ZOOM_MODE_NO_ZOOM);
	m_settableActionsList.push_back(m_pActionSetZoomModeNoZoom);
	m_actionIDToZoomMode[ACTION_ID_SET_ZOOM_MODE_NO_ZOOM] = ZoomMode::NoZoom;
	addAction(m_pActionSetZoomModeNoZoom);

//------------------------------------------------------------------------------

	m_pActionSetZoomModeFixedRatio = new QAction(this);
	m_pActionSetZoomModeFixedRatio->setIconText(trUtf8("Zoom: Fixed ratio"));
	m_pActionSetZoomModeFixedRatio->setIcon(QIcon(":zoom_fixed_ratio.png"));
	m_pActionSetZoomModeFixedRatio->setCheckable(true);
	m_pActionSetZoomModeFixedRatio->setActionGroup(m_pActionGroupZoomModes);
	m_pMenuZoomModes->addAction(m_pActionSetZoomModeFixedRatio);
	hotkey = m_pSettingsManager->getHotkey(
		ACTION_ID_SET_ZOOM_MODE_FIXED_RATIO);
	m_pActionSetZoomModeFixedRatio->setShortcut(hotkey);
	m_pActionSetZoomModeFixedRatio->setData(
		ACTION_ID_SET_ZOOM_MODE_FIXED_RATIO);
	m_settableActionsList.push_back(m_pActionSetZoomModeFixedRatio);
	m_actionIDToZoomMode[ACTION_ID_SET_ZOOM_MODE_FIXED_RATIO] =
		ZoomMode::FixedRatio;
	addAction(m_pActionSetZoomModeFixedRatio);

//------------------------------------------------------------------------------

	m_pActionSetZoomModeFitToFrame = new QAction(this);
	m_pActionSetZoomModeFitToFrame->setIconText(trUtf8("Zoom: Fit to frame"));
	m_pActionSetZoomModeFitToFrame->setIcon(QIcon(":zoom_fit_to_frame.png"));
	m_pActionSetZoomModeFitToFrame->setCheckable(true);
	m_pActionSetZoomModeFitToFrame->setActionGroup(m_pActionGroupZoomModes);
	m_pMenuZoomModes->addAction(m_pActionSetZoomModeFitToFrame);
	hotkey = m_pSettingsManager->getHotkey(
		ACTION_ID_SET_ZOOM_MODE_FIT_TO_FRAME);
	m_pActionSetZoomModeFitToFrame->setShortcut(hotkey);
	m_pActionSetZoomModeFitToFrame->setData(
		ACTION_ID_SET_ZOOM_MODE_FIT_TO_FRAME);
	m_settableActionsList.push_back(m_pActionSetZoomModeFitToFrame);
	m_actionIDToZoomMode[ACTION_ID_SET_ZOOM_MODE_FIT_TO_FRAME] =
		ZoomMode::FitToFrame;
	addAction(m_pActionSetZoomModeFitToFrame);

//------------------------------------------------------------------------------

	ZoomMode zoomMode = m_pSettingsManager->getZoomMode();
	for(QAction * pAction : m_pActionGroupZoomModes->actions())
	{
		ZoomMode actionZoomMode =
			m_actionIDToZoomMode[pAction->data().toString()];
		if(actionZoomMode == zoomMode)
		{
			pAction->setChecked(true);
			break;
		}
	}

//------------------------------------------------------------------------------

	m_pMenuZoomScaleModes = new QMenu(m_pPreviewContextMenu);
	m_pMenuZoomScaleModes->setTitle(trUtf8("Zoom scale mode"));
	m_pPreviewContextMenu->addMenu(m_pMenuZoomScaleModes);

//------------------------------------------------------------------------------

	m_pActionGroupZoomScaleModes = new QActionGroup(this);

//------------------------------------------------------------------------------

	m_pActionSetZoomScaleModeNearest = new QAction(this);
	m_pActionSetZoomScaleModeNearest->setIconText(trUtf8("Scale: Nearest"));
	m_pActionSetZoomScaleModeNearest->setCheckable(true);
	m_pActionSetZoomScaleModeNearest->setActionGroup(
		m_pActionGroupZoomScaleModes);
	m_pMenuZoomScaleModes->addAction(m_pActionSetZoomScaleModeNearest);
	hotkey = m_pSettingsManager->getHotkey(
		ACTION_ID_SET_ZOOM_SCALE_MODE_NEAREST);
	m_pActionSetZoomScaleModeNearest->setShortcut(hotkey);
	m_pActionSetZoomScaleModeNearest->setData(
		ACTION_ID_SET_ZOOM_SCALE_MODE_NEAREST);
	m_settableActionsList.push_back(m_pActionSetZoomScaleModeNearest);
	m_actionIDToZoomScaleMode[ACTION_ID_SET_ZOOM_SCALE_MODE_NEAREST] =
		Qt::FastTransformation;
	addAction(m_pActionSetZoomScaleModeNearest);

//------------------------------------------------------------------------------

	m_pActionSetZoomScaleModeBilinear = new QAction(this);
	m_pActionSetZoomScaleModeBilinear->setIconText(trUtf8("Scale: Bilinear"));
	m_pActionSetZoomScaleModeBilinear->setCheckable(true);
	m_pActionSetZoomScaleModeBilinear->setActionGroup(
		m_pActionGroupZoomScaleModes);
	m_pMenuZoomScaleModes->addAction(m_pActionSetZoomScaleModeBilinear);
	hotkey = m_pSettingsManager->getHotkey(
		ACTION_ID_SET_ZOOM_SCALE_MODE_BILINEAR);
	m_pActionSetZoomScaleModeBilinear->setShortcut(hotkey);
	m_pActionSetZoomScaleModeBilinear->setData(
		ACTION_ID_SET_ZOOM_SCALE_MODE_BILINEAR);
	m_settableActionsList.push_back(m_pActionSetZoomScaleModeBilinear);
	m_actionIDToZoomScaleMode[ACTION_ID_SET_ZOOM_SCALE_MODE_BILINEAR] =
		Qt::SmoothTransformation;
	addAction(m_pActionSetZoomScaleModeBilinear);

//------------------------------------------------------------------------------

	Qt::TransformationMode scaleMode = m_pSettingsManager->getScaleMode();
	for(QAction * pAction : m_pActionGroupZoomScaleModes->actions())
	{
		Qt::TransformationMode actionScaleMode =
			m_actionIDToZoomScaleMode[pAction->data().toString()];

		if(actionScaleMode == scaleMode)
		{
			pAction->setChecked(true);
			break;
		}
	}

//------------------------------------------------------------------------------

	bool noZoom = (zoomMode == ZoomMode::NoZoom);
	m_pMenuZoomScaleModes->setEnabled(!noZoom);

//------------------------------------------------------------------------------

	m_pActionToggleCropPanel = new QAction(this);
	m_pActionToggleCropPanel->setIconText(trUtf8("Crop assistant"));
	m_pActionToggleCropPanel->setIcon(QIcon(":crop.png"));
	m_pActionToggleCropPanel->setCheckable(true);
	m_pPreviewContextMenu->addAction(m_pActionToggleCropPanel);
	hotkey = m_pSettingsManager->getHotkey(ACTION_ID_TOGGLE_CROP_PANEL);
	m_pActionToggleCropPanel->setShortcut(hotkey);
	m_pActionToggleCropPanel->setData(ACTION_ID_TOGGLE_CROP_PANEL);
	m_settableActionsList.push_back(m_pActionToggleCropPanel);

//------------------------------------------------------------------------------

	m_pActionToggleTimeLinePanel = new QAction(this);
	m_pActionToggleTimeLinePanel->setIconText(trUtf8("Show timeline panel"));
	m_pActionToggleTimeLinePanel->setIcon(QIcon(":timeline.png"));
	m_pActionToggleTimeLinePanel->setCheckable(true);
	m_pPreviewContextMenu->addAction(m_pActionToggleTimeLinePanel);
	hotkey = m_pSettingsManager->getHotkey(ACTION_ID_TOGGLE_TIMELINE_PANEL);
	m_pActionToggleTimeLinePanel->setShortcut(hotkey);
	m_pActionToggleTimeLinePanel->setData(ACTION_ID_TOGGLE_TIMELINE_PANEL);
	m_settableActionsList.push_back(m_pActionToggleTimeLinePanel);

//------------------------------------------------------------------------------

	m_pMenuTimeLineModes= new QMenu(m_pPreviewContextMenu);
	m_pMenuTimeLineModes->setTitle(trUtf8("Timeline display mode"));
	m_pPreviewContextMenu->addMenu(m_pMenuTimeLineModes);

//------------------------------------------------------------------------------

	m_pActionGroupTimeLineModes = new QActionGroup(this);

//------------------------------------------------------------------------------


	m_pActionSetTimeLineModeTime = new QAction(this);
	m_pActionSetTimeLineModeTime->setIconText(trUtf8("Timeline mode: Time"));
	m_pActionSetTimeLineModeTime->setIcon(QIcon(":timeline.png"));
	m_pActionSetTimeLineModeTime->setCheckable(true);
	m_pActionSetTimeLineModeTime->setActionGroup(
		m_pActionGroupTimeLineModes);
	m_pMenuTimeLineModes->addAction(m_pActionSetTimeLineModeTime);
	hotkey = m_pSettingsManager->getHotkey(
		ACTION_ID_SET_TIMELINE_MODE_TIME);
	m_pActionSetTimeLineModeTime->setShortcut(hotkey);
	m_pActionSetTimeLineModeTime->setData(
		ACTION_ID_SET_TIMELINE_MODE_TIME);
	m_settableActionsList.push_back(m_pActionSetTimeLineModeTime);
	m_actionIDToTimeLineMode[ACTION_ID_SET_TIMELINE_MODE_TIME] =
		TimeLineSlider::DisplayMode::Time;
	addAction(m_pActionSetTimeLineModeTime);

//------------------------------------------------------------------------------

	m_pActionSetTimeLineModeFrames = new QAction(this);
	m_pActionSetTimeLineModeFrames->setIconText(
		trUtf8("Timeline mode: Frames"));
	m_pActionSetTimeLineModeFrames->setIcon(QIcon(":timeline_frames.png"));
	m_pActionSetTimeLineModeFrames->setCheckable(true);
	m_pActionSetTimeLineModeFrames->setActionGroup(
		m_pActionGroupTimeLineModes);
	m_pMenuTimeLineModes->addAction(m_pActionSetTimeLineModeFrames);
	hotkey = m_pSettingsManager->getHotkey(
		ACTION_ID_SET_TIMELINE_MODE_FRAMES);
	m_pActionSetTimeLineModeFrames->setShortcut(hotkey);
	m_pActionSetTimeLineModeFrames->setData(
		ACTION_ID_SET_TIMELINE_MODE_FRAMES);
	m_settableActionsList.push_back(m_pActionSetTimeLineModeFrames);
	m_actionIDToTimeLineMode[ACTION_ID_SET_TIMELINE_MODE_FRAMES] =
		TimeLineSlider::DisplayMode::Frames;
	addAction(m_pActionSetTimeLineModeFrames);

//------------------------------------------------------------------------------

	TimeLineSlider::DisplayMode timeLineMode =
		m_pSettingsManager->getTimeLineMode();
	for(QAction * pAction : m_pActionGroupTimeLineModes->actions())
	{
		TimeLineSlider::DisplayMode actionTimeLineMode =
			m_actionIDToTimeLineMode[pAction->data().toString()];

		if(actionTimeLineMode == timeLineMode)
		{
			pAction->setChecked(true);
			break;
		}
	}

//------------------------------------------------------------------------------

	m_pActionTimeStepForward = new QAction(this);
	m_pActionTimeStepForward->setIconText(trUtf8("Time: step forward"));
	m_pActionTimeStepForward->setIcon(QIcon(":time_forward.png"));
	hotkey = m_pSettingsManager->getHotkey(ACTION_ID_TIME_STEP_FORWARD);
	m_pActionTimeStepForward->setShortcut(hotkey);
	m_pActionTimeStepForward->setData(ACTION_ID_TIME_STEP_FORWARD);
	m_settableActionsList.push_back(m_pActionTimeStepForward);
	addAction(m_pActionTimeStepForward);

//------------------------------------------------------------------------------

	m_pActionTimeStepBack = new QAction(this);
	m_pActionTimeStepBack->setIconText(trUtf8("Time: step back"));
	m_pActionTimeStepBack->setIcon(QIcon(":time_back.png"));
	hotkey = m_pSettingsManager->getHotkey(ACTION_ID_TIME_STEP_BACK);
	m_pActionTimeStepBack->setShortcut(hotkey);
	m_pActionTimeStepBack->setData(ACTION_ID_TIME_STEP_BACK);
	m_settableActionsList.push_back(m_pActionTimeStepBack);
	addAction(m_pActionTimeStepBack);

//------------------------------------------------------------------------------

	m_pActionPasteCropSnippetIntoScript = new QAction(this);
	m_pActionPasteCropSnippetIntoScript->setIconText(
		trUtf8("Paste crop snippet into script"));
	m_pActionPasteCropSnippetIntoScript->setIcon(QIcon(":paste.png"));
	hotkey = m_pSettingsManager->getHotkey(
		ACTION_ID_PASTE_CROP_SNIPPET_INTO_SCRIPT);
	m_pActionPasteCropSnippetIntoScript->setShortcut(hotkey);
	m_pActionPasteCropSnippetIntoScript->setData(
		ACTION_ID_PASTE_CROP_SNIPPET_INTO_SCRIPT);
	m_settableActionsList.push_back(m_pActionPasteCropSnippetIntoScript);

//------------------------------------------------------------------------------

	connect(m_pActionFrameToClipboard, SIGNAL(triggered()),
		this, SLOT(slotFrameToClipboard()));
	connect(m_pActionSaveSnapshot, SIGNAL(triggered()),
		this, SLOT(slotSaveSnapshot()));
	connect(m_pActionToggleZoomPanel, SIGNAL(toggled(bool)),
		this, SLOT(slotToggleZoomPanelVisible(bool)));
	connect(m_pActionSetZoomModeNoZoom, SIGNAL(toggled(bool)),
		this, SLOT(slotZoomModeChanged()));
	connect(m_pActionSetZoomModeFixedRatio, SIGNAL(toggled(bool)),
		this, SLOT(slotZoomModeChanged()));
	connect(m_pActionSetZoomModeFitToFrame, SIGNAL(toggled(bool)),
		this, SLOT(slotZoomModeChanged()));
	connect(m_pActionSetZoomScaleModeNearest, SIGNAL(toggled(bool)),
		this, SLOT(slotScaleModeChanged()));
	connect(m_pActionSetZoomScaleModeBilinear, SIGNAL(toggled(bool)),
		this, SLOT(slotScaleModeChanged()));
	connect(m_pActionToggleCropPanel, SIGNAL(toggled(bool)),
		this, SLOT(slotToggleCropPanelVisible(bool)));
	connect(m_pActionToggleTimeLinePanel, SIGNAL(toggled(bool)),
		this, SLOT(slotToggleTimeLinePanelVisible(bool)));
	connect(m_pActionSetTimeLineModeTime, SIGNAL(toggled(bool)),
		this, SLOT(slotTimeLineModeChanged()));
	connect(m_pActionSetTimeLineModeFrames, SIGNAL(toggled(bool)),
		this, SLOT(slotTimeLineModeChanged()));
	connect(m_pActionTimeStepForward, SIGNAL(triggered()),
		this, SLOT(slotTimeStepForward()));
	connect(m_pActionTimeStepBack, SIGNAL(triggered()),
		this, SLOT(slotTimeStepBack()));
	connect(m_pActionPasteCropSnippetIntoScript, SIGNAL(triggered()),
		this, SLOT(slotPasteCropSnippetIntoScript()));

//------------------------------------------------------------------------------

	ActionDataList settableActionsDataList;
	for(QAction * pAction : m_settableActionsList)
		settableActionsDataList.push_back(ActionData(pAction));
	m_pSettingsDialog->addSettableActions(settableActionsDataList);
}

// END OF void PreviewDialog::createActionsAndMenus()
//==============================================================================

void PreviewDialog::setUpZoomPanel()
{
	m_ui.zoomRatioSpinBox->setLocale(QLocale("C"));

	m_ui.zoomCheckButton->setDefaultAction(m_pActionToggleZoomPanel);

	m_ui.zoomModeComboBox->addItem(QIcon(":zoom_no_zoom.png"),
		trUtf8("No zoom"), (int)ZoomMode::NoZoom);
	m_ui.zoomModeComboBox->addItem(QIcon(":zoom_fixed_ratio.png"),
		trUtf8("Fixed ratio"), (int)ZoomMode::FixedRatio);
	m_ui.zoomModeComboBox->addItem(QIcon(":zoom_fit_to_frame.png"),
		trUtf8("Fit to frame"), (int)ZoomMode::FitToFrame);

	ZoomMode zoomMode = m_pSettingsManager->getZoomMode();
	int comboIndex = m_ui.zoomModeComboBox->findData((int)zoomMode);
	if(comboIndex != -1)
		m_ui.zoomModeComboBox->setCurrentIndex(comboIndex);
	bool fixedRatio(zoomMode == ZoomMode::FixedRatio);
	m_ui.zoomRatioSpinBox->setEnabled(fixedRatio);

	double zoomRatio = m_pSettingsManager->getZoomRatio();
	m_ui.zoomRatioSpinBox->setValue(zoomRatio);

	m_ui.scaleModeComboBox->addItem(trUtf8("Nearest"),
		(int)Qt::FastTransformation);
	m_ui.scaleModeComboBox->addItem(trUtf8("Bilinear"),
		(int)Qt::SmoothTransformation);
	bool noZoom = (zoomMode == ZoomMode::NoZoom);
	m_ui.scaleModeComboBox->setEnabled(!noZoom);

	Qt::TransformationMode scaleMode = m_pSettingsManager->getScaleMode();
	comboIndex = m_ui.scaleModeComboBox->findData((int)scaleMode);
	if(comboIndex != -1)
		m_ui.scaleModeComboBox->setCurrentIndex(comboIndex);

	m_ui.zoomPanel->setVisible(m_pSettingsManager->getZoomPanelVisible());

	connect(m_ui.zoomModeComboBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(slotZoomModeChanged()));
	connect(m_ui.zoomRatioSpinBox, SIGNAL(valueChanged(double)),
		this, SLOT(slotZoomRatioChanged(double)));
	connect(m_ui.scaleModeComboBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(slotScaleModeChanged()));
}

// END OF void PreviewDialog::setUpZoomPanel()
//==============================================================================

void PreviewDialog::setUpTimeLinePanel()
{
    m_ui.timeLinePanel->setVisible(false);
    m_ui.timeLineCheckButton->setDefaultAction(m_pActionToggleTimeLinePanel);
    m_ui.timeStepForwardButton->setDefaultAction(m_pActionTimeStepForward);
    m_ui.timeStepBackButton->setDefaultAction(m_pActionTimeStepBack);

    double timeStep = m_pSettingsManager->getTimeStep();
    m_ui.timeStepEdit->setTime(vsedit::secondsToQTime(timeStep));

    m_ui.timeLineModeComboBox->addItem(QIcon(":timeline.png"), trUtf8("Time"),
		(int)TimeLineSlider::DisplayMode::Time);
    m_ui.timeLineModeComboBox->addItem(QIcon(":timeline_frames.png"),
		trUtf8("Frames"), (int)TimeLineSlider::DisplayMode::Frames);

	TimeLineSlider::DisplayMode timeLineMode =
		m_pSettingsManager->getTimeLineMode();
	int comboIndex = m_ui.timeLineModeComboBox->findData((int)timeLineMode);
	if(comboIndex != -1)
		m_ui.timeLineModeComboBox->setCurrentIndex(comboIndex);

	connect(m_ui.timeStepEdit, SIGNAL(timeChanged(const QTime &)),
		this, SLOT(slotTimeStepChanged(const QTime &)));
	connect(m_ui.timeLineModeComboBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(slotTimeLineModeChanged()));
}

// END OF void PreviewDialog::setUpTimeLinePanel()
//==============================================================================

void PreviewDialog::setUpCropPanel()
{
	m_ui.cropCheckButton->setDefaultAction(m_pActionToggleCropPanel);

	m_ui.cropModeComboBox->addItem(trUtf8("Absolute"), (int)CropMode::Absolute);
	m_ui.cropModeComboBox->addItem(trUtf8("Relative"), (int)CropMode::Relative);
	CropMode cropMode = m_pSettingsManager->getCropMode();
	int cropModeIndex = m_ui.cropModeComboBox->findData((int)cropMode);
	m_ui.cropModeComboBox->setCurrentIndex(cropModeIndex);
	slotCropModeChanged();

	m_ui.cropZoomRatioSpinBox->setValue(m_pSettingsManager->getCropZoomRatio());
	m_ui.cropPasteToScriptButton->setDefaultAction(
		m_pActionPasteCropSnippetIntoScript);
	m_ui.cropPanel->setVisible(false);

	connect(m_ui.cropModeComboBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(slotCropModeChanged()));
	connect(m_ui.cropLeftSpinBox, SIGNAL(valueChanged(int)),
		this, SLOT(slotCropLeftValueChanged(int)));
	connect(m_ui.cropTopSpinBox, SIGNAL(valueChanged(int)),
		this, SLOT(slotCropTopValueChanged(int)));
	connect(m_ui.cropWidthSpinBox, SIGNAL(valueChanged(int)),
		this, SLOT(slotCropWidthValueChanged(int)));
	connect(m_ui.cropHeightSpinBox, SIGNAL(valueChanged(int)),
		this, SLOT(slotCropHeightValueChanged(int)));
	connect(m_ui.cropRightSpinBox, SIGNAL(valueChanged(int)),
		this, SLOT(slotCropRightValueChanged(int)));
	connect(m_ui.cropBottomSpinBox, SIGNAL(valueChanged(int)),
		this, SLOT(slotCropBottomValueChanged(int)));
	connect(m_ui.cropZoomRatioSpinBox, SIGNAL(valueChanged(int)),
		this, SLOT(slotCropZoomRatioValueChanged(int)));
}

// END OF void PreviewDialog::setUpCropPanel()
//==============================================================================

bool PreviewDialog::showFrame(int a_frameNumber)
{
	if(!m_pVapourSynthScriptProcessor->isInitialized())
		return false;

	QPixmap newPixmap = m_pVapourSynthScriptProcessor->pixmap(a_frameNumber);
	if(newPixmap.isNull())
		return false;

	m_framePixmap = newPixmap;
	setPreviewPixmap();
	return true;
}

// END OF bool PreviewDialog::showFrame(int a_frameNumber)
//==============================================================================

void PreviewDialog::setPreviewPixmap()
{
	if(m_ui.cropPanel->isVisible())
	{
		int cropLeft = m_ui.cropLeftSpinBox->value();
		int cropTop = m_ui.cropTopSpinBox->value();
		int cropWidth = m_ui.cropWidthSpinBox->value();
		int cropHeight = m_ui.cropHeightSpinBox->value();
		QPixmap croppedPixmap = m_framePixmap.copy(cropLeft, cropTop,
			cropWidth, cropHeight);
		int ratio = m_ui.cropZoomRatioSpinBox->value();

		if(ratio == 1)
		{
			m_ui.previewArea->setPixmap(croppedPixmap);
			return;
		}

		QPixmap zoomedPixmap = croppedPixmap.scaled(
			croppedPixmap.width() * ratio, croppedPixmap.height() * ratio,
			Qt::KeepAspectRatio, Qt::FastTransformation);
		m_ui.previewArea->setPixmap(zoomedPixmap);
		return;
	}

	ZoomMode zoomMode = (ZoomMode)m_ui.zoomModeComboBox->currentData().toInt();
	if(zoomMode == ZoomMode::NoZoom)
	{
		m_ui.previewArea->setPixmap(m_framePixmap);
		return;
	}

	QPixmap previewPixmap;
	int frameWidth = 0;
	int frameHeight = 0;
	Qt::TransformationMode scaleMode = (Qt::TransformationMode)
		m_ui.scaleModeComboBox->currentData().toInt();

	if(zoomMode == ZoomMode::FixedRatio)
	{
		double ratio = m_ui.zoomRatioSpinBox->value();
		frameWidth = m_cpVideoInfo->width * ratio;
		frameHeight = m_cpVideoInfo->height * ratio;
	}
	else
	{
		QRect previewRect = m_ui.previewArea->geometry();
		int cropSize = m_ui.previewArea->frameWidth() * 2;
		frameWidth = previewRect.width() - cropSize;
		frameHeight = previewRect.height() - cropSize;
	}

	previewPixmap = m_framePixmap.scaled(frameWidth, frameHeight,
		Qt::KeepAspectRatio, scaleMode);
	m_ui.previewArea->setPixmap(previewPixmap);
}

// END OF bool void PreviewDialog::setPreviewPixmap()
//==============================================================================

void PreviewDialog::recalculateCropMods()
{
	QSpinBox * cropSpinBoxes[] = {m_ui.cropLeftSpinBox, m_ui.cropTopSpinBox,
		m_ui.cropWidthSpinBox, m_ui.cropHeightSpinBox, m_ui.cropRightSpinBox,
		m_ui.cropBottomSpinBox};

	for(QSpinBox * pSpinBox : cropSpinBoxes)
	{
		int value = pSpinBox->value();
		if(value == 0)
			pSpinBox->setSuffix("");
		else
		{
			int sizeMod = vsedit::mod(value);
			pSpinBox->setSuffix(QString(" |%1|").arg(sizeMod));
		}
	}
}

// END OF void PreviewDialog::recalculateCropMods()
//==============================================================================

void PreviewDialog::resetCropSpinBoxes()
{
	BEGIN_CROP_VALUES_CHANGE

	m_ui.cropLeftSpinBox->setMaximum(m_cpVideoInfo->width - 1);
	m_ui.cropLeftSpinBox->setValue(0);
	m_ui.cropTopSpinBox->setMaximum(m_cpVideoInfo->height - 1);
	m_ui.cropTopSpinBox->setValue(0);

	m_ui.cropWidthSpinBox->setMaximum(m_cpVideoInfo->width);
	m_ui.cropWidthSpinBox->setValue(m_cpVideoInfo->width);
	m_ui.cropHeightSpinBox->setMaximum(m_cpVideoInfo->height);
	m_ui.cropHeightSpinBox->setValue(m_cpVideoInfo->height);

	m_ui.cropRightSpinBox->setMaximum(m_cpVideoInfo->width - 1);
	m_ui.cropRightSpinBox->setValue(0);
	m_ui.cropBottomSpinBox->setMaximum(m_cpVideoInfo->height - 1);
	m_ui.cropBottomSpinBox->setValue(0);

	recalculateCropMods();

	END_CROP_VALUES_CHANGE
}

// END OF void PreviewDialog::resetCropSpinBoxes()
//==============================================================================
