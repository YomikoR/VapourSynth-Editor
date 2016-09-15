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

#include <vapoursynth/VapourSynth.h>

#include "../common/helpers.h"
#include "../vapoursynth/vapoursynthscriptprocessor.h"
#include "../settings/settingsdialog.h"
#include "scrollnavigator.h"
#include "timelineslider.h"
#include "preview_advanced_settings_dialog.h"

#include "previewdialog.h"

//==============================================================================

#define BEGIN_CROP_VALUES_CHANGE \
	if(m_changingCropValues) \
		return; \
	m_changingCropValues = true;

#define END_CROP_VALUES_CHANGE \
	m_changingCropValues = false;

//==============================================================================

NumberedPixmap::NumberedPixmap(int a_number, const QPixmap & a_pixmap):
	number(a_number)
	, pixmap(a_pixmap)
{
}

bool NumberedPixmap::operator<(const NumberedPixmap & a_other) const
{
	if(this == &a_other)
		return false;

	if(number < a_other.number)
		return true;

	return false;
}

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
	, m_pAdvancedSettingsDialog(nullptr)
	, m_pStatusBar(nullptr)
	, m_pVideoInfoLabel(nullptr)
	, m_pFramesInQueueLabel(nullptr)
	, m_pFramesInProcessLabel(nullptr)
	, m_pMaxThreadsLabel(nullptr)
	, m_frameExpected(0)
	, m_frameShown(-1)
	, m_lastFrameRequestedForPlay(-1)
	, m_bigFrameStep(10)
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
	, m_pActionAdvancedSettingsDialog(nullptr)
	, m_pActionToggleColorPicker(nullptr)
	, m_pActionPlay(nullptr)
	, m_framesInQueue(0)
	, m_framesInProcess(0)
	, m_maxThreads(0)
	, m_playing(false)
	, m_processingPlayQueue(false)
	, m_cachedPixmapsLimit(120)
{
	m_ui.setupUi(this);
	setWindowIcon(QIcon(":preview.png"));

	m_pAdvancedSettingsDialog = new PreviewAdvancedSettingsDialog(
		m_pSettingsManager, this);

	createActionsAndMenus();

	createStatusBar();

	m_ui.frameNumberSlider->setBigStep(m_bigFrameStep);
	m_ui.frameNumberSlider->setDisplayMode(
		m_pSettingsManager->getTimeLineMode());

	m_ui.frameToClipboardButton->setDefaultAction(m_pActionFrameToClipboard);
	m_ui.saveSnapshotButton->setDefaultAction(m_pActionSaveSnapshot);
	m_ui.advancedSettingsButton->setDefaultAction(
		m_pActionAdvancedSettingsDialog);

	setUpZoomPanel();
	setUpCropPanel();
	setUpTimeLinePanel();

	m_ui.colorPickerButton->setDefaultAction(m_pActionToggleColorPicker);
	m_ui.colorPickerLabel->setVisible(
		m_pSettingsManager->getColorPickerVisible());

	QByteArray newGeometry = m_pSettingsManager->getPreviewDialogGeometry();
	if(!newGeometry.isEmpty())
		restoreGeometry(newGeometry);

	connect(m_pVapourSynthScriptProcessor,
		SIGNAL(signalDistributePixmap(int, const QPixmap &)),
		this, SLOT(slotReceivePreviewFrame(int, const QPixmap &)));
	connect(m_pVapourSynthScriptProcessor,
		SIGNAL(signalFrameQueueStateChanged(size_t, size_t, size_t)),
		this, SLOT(slotFrameQueueStateChanged(size_t, size_t, size_t)));
	connect(m_pAdvancedSettingsDialog, SIGNAL(signalSettingsChanged()),
		m_pVapourSynthScriptProcessor, SLOT(slotSettingsChanged()));
	connect(m_pAdvancedSettingsDialog, SIGNAL(signalSettingsChanged()),
		this, SLOT(slotAdvancedSettingsChanged()));
	connect(m_ui.frameNumberSlider, SIGNAL(signalFrameChanged(int)),
		this, SLOT(slotShowFrame(int)));
	connect(m_ui.expectedFrameNumberSpinBox, SIGNAL(valueChanged(int)),
		this, SLOT(slotShowFrame(int)));
	connect(m_ui.previewArea, SIGNAL(signalSizeChanged()),
		this, SLOT(slotPreviewAreaSizeChanged()));
	connect(m_ui.previewArea, SIGNAL(signalCtrlWheel(QPoint)),
		this, SLOT(slotPreviewAreaCtrlWheel(QPoint)));
	connect(m_ui.previewArea, SIGNAL(signalMouseMiddleButtonReleased()),
		this, SLOT(slotPreviewAreaMouseMiddleButtonReleased()));
	connect(m_ui.previewArea, SIGNAL(signalMouseRightButtonReleased()),
		this, SLOT(slotPreviewAreaMouseRightButtonReleased()));
	connect(m_ui.previewArea, SIGNAL(signalMouseRightButtonReleased()),
		this, SLOT(slotPreviewAreaMouseRightButtonReleased()));
	connect(m_ui.previewArea, SIGNAL(signalMouseOverPoint(float, float)),
		this, SLOT(slotPreviewAreaMouseOverPoint(float, float)));
	connect(m_pSettingsDialog, SIGNAL(signalSettingsChanged()),
		this, SLOT(slotSettingsChanged()));

	slotSettingsChanged();
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
	clear();

	bool initialized =
		m_pVapourSynthScriptProcessor->initialize(a_script, a_scriptName);
	if(!initialized)
	{
		hide();
		return;
	}

	QString title = "Preview - ";
	title += a_scriptName;
	setWindowTitle(title);

	m_cpVideoInfo = m_pVapourSynthScriptProcessor->videoInfo();
	assert(m_cpVideoInfo);

	int lastFrameNumber = m_cpVideoInfo->numFrames - 1;
	m_ui.shownFrameSpinBox->setMaximum(lastFrameNumber);
	m_ui.expectedFrameNumberSpinBox->setMaximum(lastFrameNumber);
	m_ui.frameNumberSlider->setFramesNumber(m_cpVideoInfo->numFrames);
	if(m_cpVideoInfo->fpsDen == 0)
		m_ui.frameNumberSlider->setFPS(0.0);
	else
	{
		m_ui.frameNumberSlider->setFPS((double)m_cpVideoInfo->fpsNum /
			(double)m_cpVideoInfo->fpsDen);
	}

	if(m_frameExpected > lastFrameNumber)
		m_frameExpected = lastFrameNumber;

	QString newVideoInfoString = vsedit::videoInfoString(m_cpVideoInfo);
	m_pVideoInfoLabel->setText(newVideoInfoString);
	m_pVideoInfoLabel->setToolTip(newVideoInfoString);

	resetCropSpinBoxes();

	m_scriptName = a_scriptName;

	if(m_pSettingsManager->getPreviewDialogMaximized())
		showMaximized();
	else
		showNormal();

	slotShowFrame(m_frameExpected);
}

// END OF void PreviewDialog::previewScript(const QString& a_script,
//		const QString& a_scriptName)
//==============================================================================

void PreviewDialog::clear()
{
	m_frameShown = -1;
	m_ui.shownFrameSpinBox->setValue(-1);
	m_framePixmap = QPixmap();
	m_ui.previewArea->setPixmap(QPixmap());
	m_pVideoInfoLabel->clear();
	m_pFramesInQueueLabel->clear();
	m_pFramesInProcessLabel->clear();
	m_pMaxThreadsLabel->clear();
}

// END OF void PreviewDialog::clear()
//==============================================================================

void PreviewDialog::closeEvent(QCloseEvent * a_pEvent)
{
	m_pVapourSynthScriptProcessor->finalize();
	clear();
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

	if(((key == Qt::Key_Left) || (key == Qt::Key_Down)) &&
		(m_frameExpected > 0))
		slotShowFrame(m_frameExpected - 1);
	else if(((key == Qt::Key_Right) || (key == Qt::Key_Up)) &&
		(m_frameExpected < (m_cpVideoInfo->numFrames - 1)))
		slotShowFrame(m_frameExpected + 1);
	else if((key == Qt::Key_PageDown) && (m_frameExpected > 0))
		slotShowFrame(std::max(0, m_frameExpected - m_bigFrameStep));
	else if((key == Qt::Key_PageUp) &&
		(m_frameExpected < (m_cpVideoInfo->numFrames - 1)))
	{
		slotShowFrame(std::min(m_cpVideoInfo->numFrames - 1,
			m_frameExpected + m_bigFrameStep));
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
	if((m_frameExpected == a_frameNumber) && (!m_framePixmap.isNull()))
		return;

	if(m_playing)
		return;

	static bool requestingFrame = false;
	if(requestingFrame)
		return;
	requestingFrame = true;

	m_ui.expectedFrameNumberSpinBox->setValue(a_frameNumber);
	m_ui.frameNumberSlider->setFrame(a_frameNumber);

	bool frameShown = showFrame(a_frameNumber);
	if(frameShown)
		m_frameExpected = a_frameNumber;
	else
	{
		m_ui.expectedFrameNumberSpinBox->setValue(m_frameExpected);
		m_ui.frameNumberSlider->setFrame(m_frameExpected);
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
		snapshotFilePath += QString("/%1.png").arg(m_frameShown);
	}
	else
		snapshotFilePath += QString(" - %1.png").arg(m_frameShown);

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
				m_actionIDToZoomModeMap[pAction->data().toString()];
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
		zoomMode = m_actionIDToZoomModeMap[pSenderAction->data().toString()];
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
				m_actionIDToZoomScaleModeMap[pAction->data().toString()];
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
			m_actionIDToZoomScaleModeMap[pSenderAction->data().toString()];
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
				m_actionIDToTimeLineModeMap[pAction->data().toString()];
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
			m_actionIDToTimeLineModeMap[pSenderAction->data().toString()];
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

	QFont sliderLabelsFont =
		m_pSettingsManager->getTextFormat(TEXT_FORMAT_ID_TIMELINE).font();
	m_ui.frameNumberSlider->setLabelsFont(sliderLabelsFont);
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

void PreviewDialog::slotPreviewAreaMouseOverPoint(float a_normX, float a_normY)
{
	if(!m_ui.colorPickerLabel->isVisible())
		return;

	double value1 = 0.0;
	double value2 = 0.0;
	double value3 = 0.0;

	size_t frameX = 0;
	size_t frameY = 0;

	frameX = (size_t)((float)m_framePixmap.width() * a_normX);
	frameY = (size_t)((float)m_framePixmap.height() * a_normY);

	m_pVapourSynthScriptProcessor->colorAtPoint(frameX, frameY,
		value1, value2, value3);

	QString l1("1");
	QString l2("2");
	QString l3("3");

	int colorFamily = m_cpVideoInfo->format->colorFamily;
	int formatID = m_cpVideoInfo->format->id;

	if(colorFamily == cmGray)
	{
		QString colorString = QString("G:%1").arg(value1);
		m_ui.colorPickerLabel->setText(colorString);
		return;
	}
	else if((colorFamily == cmYUV) || (formatID == pfCompatYUY2))
	{
		l1 = "Y";
		l2 = "U";
		l3 = "V";
	}
	else if((colorFamily == cmRGB) || (formatID == pfCompatBGR32))
	{
		l1 = "R";
		l2 = "G";
		l3 = "B";
	}
	else if(colorFamily == cmYCoCg)
	{
		l1 = "Y";
		l2 = "Co";
		l3 = "Cg";
	}

	QString colorString = QString("%1:%2|%3:%4|%5:%6")
		.arg(l1).arg(value1).arg(l2).arg(value2).arg(l3).arg(value3);
	m_ui.colorPickerLabel->setText(colorString);
}

// END OF void PreviewDialog::slotPreviewAreaMouseOverPoint(float a_normX,
//		float a_normY)
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

void PreviewDialog::slotAdvancedSettingsChanged()
{
	showFrame(m_frameExpected);
}

// END OF void PreviewDialog::slotAdvancedSettingsChanged()
//==============================================================================

void PreviewDialog::slotToggleColorPicker(bool a_colorPickerVisible)
{
	m_ui.colorPickerLabel->setVisible(a_colorPickerVisible);
	m_pSettingsManager->setColorPickerVisible(a_colorPickerVisible);
}

// END OF void PreviewDialog::slotToggleColorPicker(bool a_colorPickerVisible)
//==============================================================================

void PreviewDialog::slotReceivePreviewFrame(int a_frameNumber,
	const QPixmap & a_pixmap)
{
	if(a_pixmap.isNull())
		return;

	if(m_playing)
	{
		NumberedPixmap newPixmap(a_frameNumber, a_pixmap);
		m_framePixmapsQueue.insert(std::upper_bound(m_framePixmapsQueue.begin(),
			m_framePixmapsQueue.end(), newPixmap), newPixmap);
		slotProcessPlayQueue();
	}
	else
	{
		m_framePixmap = a_pixmap;
		setPreviewPixmap();

		m_frameShown = a_frameNumber;
		m_ui.shownFrameSpinBox->setValue(m_frameShown);
	}
}

// END OF void PreviewDialog::slotToggleColorPicker(bool a_colorPickerVisible)
//==============================================================================

void PreviewDialog::slotFrameQueueStateChanged(size_t a_inQueue,
	size_t a_inProcess, size_t a_maxThreads)
{
	m_framesInQueue = a_inQueue;
	m_framesInProcess = a_inProcess;
	m_maxThreads = a_maxThreads;

	m_pFramesInQueueLabel->setText(QString::number(a_inQueue));
	m_pFramesInProcessLabel->setText(QString::number(a_inProcess));
	m_pMaxThreadsLabel->setText(QString::number(a_maxThreads));

	if(m_playing)
		slotProcessPlayQueue();
}

// END OF void PreviewDialog::slotFrameQueueStateChanged(size_t a_inQueue,
//		size_t a_inProcess, size_t a_maxThreads)
//==============================================================================

void PreviewDialog::slotPlay(bool a_play)
{
	m_playing = a_play;
	if(m_playing)
	{
		disconnect(m_ui.frameNumberSlider, SIGNAL(signalFrameChanged(int)),
		this, SLOT(slotShowFrame(int)));
		disconnect(m_ui.expectedFrameNumberSpinBox, SIGNAL(valueChanged(int)),
			this, SLOT(slotShowFrame(int)));
		m_lastFrameRequestedForPlay = m_frameShown;
		slotProcessPlayQueue();
	}
	else
	{
		m_framePixmapsQueue.clear();
		m_pVapourSynthScriptProcessor->flushFrameTicketsQueueForPreview();
		connect(m_ui.frameNumberSlider, SIGNAL(signalFrameChanged(int)),
		this, SLOT(slotShowFrame(int)));
		connect(m_ui.expectedFrameNumberSpinBox, SIGNAL(valueChanged(int)),
			this, SLOT(slotShowFrame(int)));
	}
}

// END OF void PreviewDialog::slotPlay(bool a_play)
//==============================================================================

void PreviewDialog::slotProcessPlayQueue()
{
	if(!m_playing)
		return;

	if(m_processingPlayQueue)
		return;
	m_processingPlayQueue = true;

	while((!m_framePixmapsQueue.empty()) &&
		(m_framePixmapsQueue.front().number ==
		(m_frameShown + 1) % m_cpVideoInfo->numFrames))
	{
		m_framePixmap = m_framePixmapsQueue.front().pixmap;
		setPreviewPixmap();

		m_frameShown = m_framePixmapsQueue.front().number;
		m_ui.shownFrameSpinBox->setValue(m_frameExpected);
		m_frameExpected = m_frameShown;
		m_ui.expectedFrameNumberSpinBox->setValue(m_frameExpected);
		m_ui.frameNumberSlider->setFrame(m_frameExpected);
		m_framePixmapsQueue.pop_front();
	}

	int nextFrame = (m_lastFrameRequestedForPlay + 1) %
		m_cpVideoInfo->numFrames;

	// Each preview request results in two frame requests to synchronize
	// between the preview and the actual output.
	while(((m_framesInQueue + m_framesInProcess) <= (m_maxThreads - 2)) &&
		(m_framePixmapsQueue.size() <= m_cachedPixmapsLimit))
	{
		m_pVapourSynthScriptProcessor->requestPixmapAsync(nextFrame);
		m_lastFrameRequestedForPlay = nextFrame;
		nextFrame = (nextFrame + 1) % m_cpVideoInfo->numFrames;
	}

	m_processingPlayQueue = false;
}

// END OF void PreviewDialog::slotPlay(bool a_play)
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
	m_pPreviewContextMenu->addAction(m_pActionFrameToClipboard);
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
	m_actionIDToZoomModeMap[ACTION_ID_SET_ZOOM_MODE_NO_ZOOM] = ZoomMode::NoZoom;
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
	m_actionIDToZoomModeMap[ACTION_ID_SET_ZOOM_MODE_FIXED_RATIO] =
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
	m_actionIDToZoomModeMap[ACTION_ID_SET_ZOOM_MODE_FIT_TO_FRAME] =
		ZoomMode::FitToFrame;
	addAction(m_pActionSetZoomModeFitToFrame);

//------------------------------------------------------------------------------

	ZoomMode zoomMode = m_pSettingsManager->getZoomMode();
	for(QAction * pAction : m_pActionGroupZoomModes->actions())
	{
		ZoomMode actionZoomMode =
			m_actionIDToZoomModeMap[pAction->data().toString()];
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
	m_actionIDToZoomScaleModeMap[ACTION_ID_SET_ZOOM_SCALE_MODE_NEAREST] =
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
	m_actionIDToZoomScaleModeMap[ACTION_ID_SET_ZOOM_SCALE_MODE_BILINEAR] =
		Qt::SmoothTransformation;
	addAction(m_pActionSetZoomScaleModeBilinear);

//------------------------------------------------------------------------------

	Qt::TransformationMode scaleMode = m_pSettingsManager->getScaleMode();
	for(QAction * pAction : m_pActionGroupZoomScaleModes->actions())
	{
		Qt::TransformationMode actionScaleMode =
			m_actionIDToZoomScaleModeMap[pAction->data().toString()];

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
	m_actionIDToTimeLineModeMap[ACTION_ID_SET_TIMELINE_MODE_TIME] =
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
	m_actionIDToTimeLineModeMap[ACTION_ID_SET_TIMELINE_MODE_FRAMES] =
		TimeLineSlider::DisplayMode::Frames;
	addAction(m_pActionSetTimeLineModeFrames);

//------------------------------------------------------------------------------

	TimeLineSlider::DisplayMode timeLineMode =
		m_pSettingsManager->getTimeLineMode();
	for(QAction * pAction : m_pActionGroupTimeLineModes->actions())
	{
		TimeLineSlider::DisplayMode actionTimeLineMode =
			m_actionIDToTimeLineModeMap[pAction->data().toString()];

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

	m_pActionAdvancedSettingsDialog = new QAction(this);
	m_pActionAdvancedSettingsDialog->setIconText(
		trUtf8("Preview advanced settings"));
	m_pActionAdvancedSettingsDialog->setIcon(QIcon(":settings.png"));
	hotkey = m_pSettingsManager->getHotkey(
		ACTION_ID_ADVANCED_PREVIEW_SETTINGS);
	m_pActionAdvancedSettingsDialog->setShortcut(hotkey);
	m_pActionAdvancedSettingsDialog->setData(
		ACTION_ID_ADVANCED_PREVIEW_SETTINGS);
	m_settableActionsList.push_back(m_pActionAdvancedSettingsDialog);

//------------------------------------------------------------------------------

	m_pActionToggleColorPicker = new QAction(this);
	m_pActionToggleColorPicker->setIconText(trUtf8("Color panel"));
	m_pActionToggleColorPicker->setIcon(QIcon(":color_picker.png"));
	m_pActionToggleColorPicker->setCheckable(true);
	m_pActionToggleColorPicker->setChecked(
		m_pSettingsManager->getColorPickerVisible());
	m_pPreviewContextMenu->addAction(m_pActionToggleColorPicker);
	hotkey = m_pSettingsManager->getHotkey(ACTION_ID_TOGGLE_COLOR_PICKER);
	m_pActionToggleColorPicker->setShortcut(hotkey);
	m_pActionToggleColorPicker->setData(ACTION_ID_TOGGLE_COLOR_PICKER);
	m_settableActionsList.push_back(m_pActionToggleColorPicker);

//------------------------------------------------------------------------------

	m_pActionPlay = new QAction(this);
	m_pActionPlay->setIconText(trUtf8("Play"));
	m_pActionPlay->setIcon(QIcon(":play.png"));
	m_pActionPlay->setCheckable(true);
	m_pActionPlay->setChecked(false);
	hotkey = m_pSettingsManager->getHotkey(ACTION_ID_PLAY);
	m_pActionPlay->setShortcut(hotkey);
	m_pActionPlay->setData(ACTION_ID_PLAY);
	m_settableActionsList.push_back(m_pActionPlay);
	addAction(m_pActionPlay);

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
	connect(m_pActionAdvancedSettingsDialog, SIGNAL(triggered()),
		m_pAdvancedSettingsDialog, SLOT(slotCall()));
	connect(m_pActionToggleColorPicker, SIGNAL(toggled(bool)),
		this, SLOT(slotToggleColorPicker(bool)));
	connect(m_pActionPlay, SIGNAL(toggled(bool)),
		this, SLOT(slotPlay(bool)));

//------------------------------------------------------------------------------

	ActionDataList settableActionsDataList;
	for(QAction * pAction : m_settableActionsList)
		settableActionsDataList.push_back(ActionData(pAction));
	m_pSettingsDialog->addSettableActions(settableActionsDataList);
}

// END OF void PreviewDialog::createActionsAndMenus()
//==============================================================================

void PreviewDialog::createStatusBar()
{
	m_pStatusBar = new QStatusBar(this);
	m_ui.mainLayout->addWidget(m_pStatusBar);

	m_pVideoInfoLabel = new QLabel(m_pStatusBar);
	m_pStatusBar->addPermanentWidget(m_pVideoInfoLabel);

	m_pFramesInQueueLabel = new QLabel(m_pStatusBar);
	m_pStatusBar->addPermanentWidget(m_pFramesInQueueLabel);

	m_pFramesInProcessLabel = new QLabel(m_pStatusBar);
	m_pStatusBar->addPermanentWidget(m_pFramesInProcessLabel);

	m_pMaxThreadsLabel = new QLabel(m_pStatusBar);
	m_pStatusBar->addPermanentWidget(m_pMaxThreadsLabel);
}

// END OF void PreviewDialog::createStatusBar()
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
    m_ui.playButton->setDefaultAction(m_pActionPlay);
    m_ui.timeLineCheckButton->setDefaultAction(m_pActionToggleTimeLinePanel);
    m_ui.timeStepForwardButton->setDefaultAction(m_pActionTimeStepForward);
    m_ui.timeStepBackButton->setDefaultAction(m_pActionTimeStepBack);

    m_ui.playFpsLimitModeComboBox->addItem(trUtf8("No limit"));
    m_ui.playFpsLimitModeComboBox->addItem(trUtf8("From video"));
    m_ui.playFpsLimitModeComboBox->addItem(trUtf8("Custom"));

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

	m_pVapourSynthScriptProcessor->requestPixmapAsync(a_frameNumber);

	// Old blocking way.
//	QPixmap newPixmap = m_pVapourSynthScriptProcessor->pixmap(a_frameNumber);
//	if(newPixmap.isNull())
//		return false;
//
//	m_framePixmap = newPixmap;
//	setPreviewPixmap();

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
