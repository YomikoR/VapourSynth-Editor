#include "preview_dialog.h"

#include "../../../common-src/helpers.h"
#include "../../../common-src/libp2p/p2p_api.h"
#include "../../../common-src/vapoursynth/vapoursynth_script_processor.h"
#include "../../../common-src/settings/settings_manager.h"
#include "../settings/settings_dialog.h"
#include "scroll_navigator.h"
#include "../../../common-src/timeline_slider/timeline_slider.h"
#include "preview_advanced_settings_dialog.h"
#include "zoom_ratio_spinbox.h"

#include <vapoursynth/VapourSynth4.h>
#include <vapoursynth/VSHelper4.h>

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
#include <QTimer>
#include <QImageWriter>
#include <QFileInfo>
#include <QRegExp>
#include <QScreen>
#include <QWindow>
#include <algorithm>
#include <cmath>

//==============================================================================

#define BEGIN_CROP_VALUES_CHANGE \
	if(m_changingCropValues) \
		return; \
	m_changingCropValues = true;

#define END_CROP_VALUES_CHANGE \
	m_changingCropValues = false;

//==============================================================================

const char TIMELINE_BOOKMARKS_FILE_SUFFIX[] = ".bookmarks";

//==============================================================================

PreviewDialog::PreviewDialog(SettingsManager * a_pSettingsManager,
	VSScriptLibrary * a_pVSScriptLibrary, QWidget * a_pParent) :
	VSScriptProcessorDialog(a_pSettingsManager, a_pVSScriptLibrary, a_pParent)
	, m_pAdvancedSettingsDialog(nullptr)
	, m_frameExpected(0)
	, m_frameShown(-1)
	, m_lastFrameRequestedForPlay(-1)
	, m_bigFrameStep(10)
	, m_cpFrame(nullptr)
	, m_cpPreviewFrame(nullptr)
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
	, m_pActionLoadChapters(nullptr)
	, m_pActionClearBookmarks(nullptr)
	, m_pActionBookmarkCurrentFrame(nullptr)
	, m_pActionUnbookmarkCurrentFrame(nullptr)
	, m_pActionGoToPreviousBookmark(nullptr)
	, m_pActionGoToNextBookmark(nullptr)
	, m_pActionPasteShownFrameNumberIntoScript(nullptr)
	, m_pActionSwitchToOutputIndex0(nullptr)
	, m_pActionSwitchToOutputIndex1(nullptr)
	, m_pActionSwitchToOutputIndex2(nullptr)
	, m_pActionSwitchToOutputIndex3(nullptr)
	, m_pActionSwitchToOutputIndex4(nullptr)
	, m_pActionSwitchToOutputIndex5(nullptr)
	, m_pActionSwitchToOutputIndex6(nullptr)
	, m_pActionSwitchToOutputIndex7(nullptr)
	, m_pActionSwitchToOutputIndex8(nullptr)
	, m_pActionSwitchToOutputIndex9(nullptr)
	, m_playing(false)
	, m_processingPlayQueue(false)
	, m_nativePlaybackRate(false)
	, m_secondsBetweenFrames(0)
	, m_pPlayTimer(nullptr)
	, m_alwaysKeepCurrentFrame(DEFAULT_ALWAYS_KEEP_CURRENT_FRAME)
	, m_pGeometrySaveTimer(nullptr)
	, m_devicePixelRatio(-1)
	, m_toChangeTitle(false)
{
	vsedit::disableFontKerning(this);
	m_ui.setupUi(this);
	setWindowIcon(QIcon(":preview.png"));

	m_iconPlay = QIcon(":play.png");
	m_iconPause = QIcon(":pause.png");

	m_pAdvancedSettingsDialog = new PreviewAdvancedSettingsDialog(
		m_pSettingsManager, this);

	m_pPlayTimer = new QTimer(this);
	m_pPlayTimer->setTimerType(Qt::PreciseTimer);
	m_pPlayTimer->setSingleShot(true);

	createActionsAndMenus();

	createStatusBar();
	m_pStatusBarWidget->setColorPickerVisible(
		m_pSettingsManager->getColorPickerVisible());

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

	m_pGeometrySaveTimer = new QTimer(this);
	m_pGeometrySaveTimer->setInterval(DEFAULT_WINDOW_GEOMETRY_SAVE_DELAY);
	connect(m_pGeometrySaveTimer, &QTimer::timeout,
		this, &PreviewDialog::slotSaveGeometry);

	m_windowGeometry = m_pSettingsManager->getPreviewDialogGeometry();
	if(!m_windowGeometry.isEmpty())
		restoreGeometry(m_windowGeometry);

	connect(m_pAdvancedSettingsDialog, SIGNAL(signalSettingsChanged()),
		this, SLOT(slotAdvancedSettingsChanged()));
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
	connect(m_ui.previewArea, SIGNAL(signalMouseOverPoint(double, double)),
		this, SLOT(slotPreviewAreaMouseOverPoint(double, double)));
	connect(m_pPlayTimer, SIGNAL(timeout()),
		this, SLOT(slotProcessPlayQueue()));

	slotSettingsChanged();

	bool rememberLastPreviewFrame =
		m_pSettingsManager->getRememberLastPreviewFrame();
	if(rememberLastPreviewFrame)
	{
		m_frameExpected = m_pSettingsManager->getLastPreviewFrame();
		setScriptName(m_pSettingsManager->getLastUsedPath());
	}
}

// END OF PreviewDialog::PreviewDialog(SettingsManager * a_pSettingsManager,
//		VSScriptLibrary * a_pVSScriptLibrary, QWidget * a_pParent)
//==============================================================================

PreviewDialog::~PreviewDialog()
{
	if(m_pGeometrySaveTimer->isActive())
	{
		m_pGeometrySaveTimer->stop();
		slotSaveGeometry();
	}
}

// END OF PreviewDialog::~PreviewDialog()
//==============================================================================

void PreviewDialog::setScriptName(const QString & a_scriptName)
{
	VSScriptProcessorDialog::setScriptName(a_scriptName);
	setTitle();
}

// END OF void PreviewDialog::setScriptName(const QString & a_scriptName)
//==============================================================================

void PreviewDialog::previewScript(const QString& a_script,
	const QString& a_scriptName)
{
	QString previousScript = script();
	QString previousScriptName = scriptName();

	stopAndCleanUp();

	bool initialized = initialize(a_script, a_scriptName);
	if(!initialized)
		return;

	setTitle();

	int lastFrameNumber = m_cpVideoInfo[m_outputIndex]->numFrames - 1;
	m_ui.frameNumberSpinBox->setMaximum(lastFrameNumber);
	m_ui.frameNumberSlider->setFramesNumber(
		m_cpVideoInfo[m_outputIndex]->numFrames);
	if(m_cpVideoInfo[m_outputIndex]->fpsDen == 0)
		m_ui.frameNumberSlider->setFPS(0.0);
	else
	{
		m_ui.frameNumberSlider->setFPS(
			(double)m_cpVideoInfo[m_outputIndex]->fpsNum /
			(double)m_cpVideoInfo[m_outputIndex]->fpsDen);
	}

	bool scriptChanged = ((previousScript != a_script) &&
		(previousScriptName != a_scriptName));

	if(scriptChanged && (!m_alwaysKeepCurrentFrame))
	{
		m_frameExpected = 0;
		m_ui.previewArea->setPixmap(QPixmap());
	}

	if(m_frameExpected > lastFrameNumber)
		m_frameExpected = lastFrameNumber;

	resetCropSpinBoxes();

	slotSetPlayFPSLimit();

	setScriptName(a_scriptName);

	loadTimelineBookmarks();

	if(m_pSettingsManager->getPreviewDialogMaximized())
		showMaximized();
	else
		showNormal();

	slotShowFrame(m_frameExpected);
}

// END OF void PreviewDialog::previewScript(const QString& a_script,
//		const QString& a_scriptName)
//==============================================================================

void PreviewDialog::stopAndCleanUp()
{
	slotPlay(false);

	if(m_ui.cropCheckButton->isChecked())
		m_ui.cropCheckButton->click();

	bool rememberLastPreviewFrame =
		m_pSettingsManager->getRememberLastPreviewFrame();
	if(rememberLastPreviewFrame && (!scriptName().isEmpty()) &&
		(m_frameShown > -1))
		m_pSettingsManager->setLastPreviewFrame(m_frameShown);

	m_frameShown = -1;
	m_framePixmap = QPixmap();
	// Replace shown image with a blank one of the same dimension:
	// -helps to keep the scrolling position when refreshing the script;
	// -leaves the image blank on sudden error;
	// -creates a blinking effect indicating the script is being refreshed.
	int pixmapWidth = m_ui.previewArea->pixmapWidth();
	int pixmapHeight = m_ui.previewArea->pixmapHeight();
	QPixmap blackPixmap(pixmapWidth, pixmapHeight);
	blackPixmap.fill(Qt::black);
	m_ui.previewArea->setPixmap(blackPixmap);

	if(m_cpFrame)
	{
		Q_ASSERT(m_cpVSAPI);
		m_cpVSAPI->freeFrame(m_cpFrame);
		m_cpFrame = nullptr;
	}

	if(m_cpPreviewFrame)
	{
		Q_ASSERT(m_cpVSAPI);
		m_cpVSAPI->freeFrame(m_cpPreviewFrame);
		m_cpPreviewFrame = nullptr;
	}

	VSScriptProcessorDialog::stopAndCleanUp();
}

// END OF void PreviewDialog::stopAndCleanUp()
//==============================================================================

void PreviewDialog::moveEvent(QMoveEvent * a_pEvent)
{
	QDialog::moveEvent(a_pEvent);
	saveGeometryDelayed();
}

// END OF void PreviewDialog::moveEvent(QMoveEvent * a_pEvent)
//==============================================================================

void PreviewDialog::resizeEvent(QResizeEvent * a_pEvent)
{
	QDialog::resizeEvent(a_pEvent);
	saveGeometryDelayed();
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
	Q_ASSERT(m_cpVideoInfo[m_outputIndex]);

	int key = a_pEvent->key();

	if(((key == Qt::Key_Left) || (key == Qt::Key_Down)) &&
		(m_frameExpected > 0))
		slotShowFrame(m_frameExpected - 1);
	else if(((key == Qt::Key_Right) || (key == Qt::Key_Up)) &&
		(m_frameExpected < (m_cpVideoInfo[m_outputIndex]->numFrames - 1)))
		slotShowFrame(m_frameExpected + 1);
	else if((key == Qt::Key_PageDown) && (m_frameExpected > 0))
		slotShowFrame(std::max(0, m_frameExpected - m_bigFrameStep));
	else if((key == Qt::Key_PageUp) &&
		(m_frameExpected < (m_cpVideoInfo[m_outputIndex]->numFrames - 1)))
	{
		slotShowFrame(std::min(m_cpVideoInfo[m_outputIndex]->numFrames - 1,
			m_frameExpected + m_bigFrameStep));
	}
	else if(key == Qt::Key_Home)
		slotShowFrame(0);
	else if(key == Qt::Key_End)
		slotShowFrame(m_cpVideoInfo[m_outputIndex]->numFrames - 1);
	else if(key == Qt::Key_Escape)
		close();
	else
		QDialog::keyPressEvent(a_pEvent);
}

// END OF void PreviewDialog::keyPressEvent(QKeyEvent * a_pEvent)
//==============================================================================

void PreviewDialog::slotReceiveFrame(int a_frameNumber, int a_outputIndex,
	const VSFrame * a_cpOutputFrame,
	const VSFrame * a_cpPreviewFrame)
{
	if(!a_cpOutputFrame)
		return;

	Q_ASSERT(m_cpVSAPI);
	const VSFrame * cpOutputFrame =
		m_cpVSAPI->addFrameRef(a_cpOutputFrame);
	const VSFrame * cpPreviewFrame =
		m_cpVSAPI->addFrameRef(a_cpPreviewFrame);

	if(m_playing)
	{
		Frame newFrame(a_frameNumber, a_outputIndex,
			cpOutputFrame, cpPreviewFrame);
		m_framesCache[m_outputIndex].push_back(newFrame);
		slotProcessPlayQueue();
	}
	else
	{
		setCurrentFrame(cpOutputFrame, cpPreviewFrame);
		m_frameShown = a_frameNumber;
		if(m_frameShown == m_frameExpected)
			m_ui.frameStatusLabel->setPixmap(m_readyPixmap);
	}
}

// END OF void PreviewDialog::slotReceiveFrame(int a_frameNumber,
//		int a_outputIndex, const VSFrame * a_cpOutputFrame,
//		const VSFrame * a_cpPreviewFrame)
//==============================================================================

void PreviewDialog::slotFrameRequestDiscarded(int a_frameNumber,
	int a_outputIndex, const QString & a_reason)
{
	(void)a_outputIndex;
	(void)a_reason;

	if(m_playing)
	{
		slotPlay(false);
	}
	else
	{
		if(a_frameNumber != m_frameExpected)
			return;

		if(m_frameShown == -1)
		{
			if(m_frameExpected == 0)
			{
				// Nowhere to roll back
				m_ui.frameNumberSlider->setFrame(0);
				m_ui.frameNumberSpinBox->setValue(0);
				m_ui.frameStatusLabel->setPixmap(m_errorPixmap);
			}
			else
				slotShowFrame(0);
			return;
		}

		m_frameExpected = m_frameShown;
		m_ui.frameNumberSlider->setFrame(m_frameShown);
		m_ui.frameNumberSpinBox->setValue(m_frameShown);
		m_ui.frameStatusLabel->setPixmap(m_readyPixmap);
	}
}

// END OF void PreviewDialog::slotFrameRequestDiscarded(int a_frameNumber,
//		int a_outputIndex, const QString & a_reason)
//==============================================================================

void PreviewDialog::slotShowFrame(int a_frameNumber)
{
	if((m_frameShown == a_frameNumber) && (!m_framePixmap.isNull()))
		return;

	if(m_playing)
		return;

	static bool requestingFrame = false;
	if(requestingFrame)
		return;
	requestingFrame = true;

	m_ui.frameNumberSpinBox->setValue(a_frameNumber);
	m_ui.frameNumberSlider->setFrame(a_frameNumber);

	bool requested = requestShowFrame(a_frameNumber);
	if(requested)
	{
		m_frameExpected = a_frameNumber;
		m_ui.frameStatusLabel->setPixmap(m_busyPixmap);
	}
	else
	{
		m_ui.frameNumberSpinBox->setValue(m_frameExpected);
		m_ui.frameNumberSlider->setFrame(m_frameExpected);
	}

	requestingFrame = false;
}
// END OF void PreviewDialog::slotShowFrame(int a_frameNumber)
//==============================================================================

void PreviewDialog::slotSaveSnapshot()
{
	if((m_frameShown < 0) || m_framePixmap.isNull())
		return;

	std::map<QString, QString> extensionToFilterMap =
	{
		{"png", tr("PNG image (*.png)")},
	};

	QString fileExtension = m_pSettingsManager->getLastSnapshotExtension();

	QList<QByteArray> supportedFormats = QImageWriter::supportedImageFormats();
	bool webpSupported = (supportedFormats.indexOf("webp") > -1);

	if(webpSupported)
		extensionToFilterMap["webp"] = tr("WebP image (*.webp)");

	QString currScriptName = scriptName();
	bool currScriptNotSaved = currScriptName.isEmpty();

	// Parse the template
	QString snapshotTemplate = m_pSettingsManager->getSnapshotTemplate();
	if(!currScriptNotSaved)
	{
		std::vector<vsedit::VariableToken> variables =
		{
			{"{f}", tr("script file path"),
				[&]()
				{
					return currScriptName;
				}
			},

			{"{d}", tr("script file directory"),
				[&]()
				{
					QFileInfo file(currScriptName);
					return QDir::toNativeSeparators(file.path());
				}
			},

			{"{n}", tr("script file name"),
				[&]()
				{
					QFileInfo file(currScriptName);
					return file.completeBaseName();
				}
			},

			{"{o}", tr("output index"),
				[&]()
				{
					return QString::number(m_outputIndex);
				}
			},

			{"{i}", tr("frame number"),
				[&]()
				{
					return QString::number(m_frameShown);
				}
			},

			{"{t}", tr("timestamp"),
				[&]()
				{
					auto fpsden = m_cpVideoInfo[m_outputIndex]->fpsDen;
					auto fpsnum = m_cpVideoInfo[m_outputIndex]->fpsNum;
					if(fpsden == 0 || fpsnum == 0)
						return QString();
					double fps = (double)fpsnum / (double)fpsden;
					QString timeStr = vsedit::timeToString(
						m_frameShown / fps, true).replace(":", ".");
					return timeStr;
				}
			},

			{"{nm}", tr("clip name"),
				[&]()
				{
					return m_clipName;
				}
			},

			{"{sc}", tr("scene name"),
				[&]()
				{
					return m_sceneName;
				}
			},
		};

		for(const vsedit::VariableToken & var : variables)
		{
			snapshotTemplate = snapshotTemplate.replace(
				var.token, var.evaluate());
		}
	}

	bool silentSnapshot = m_pSettingsManager->getSilentSnapshot();

	QString snapshotFilePath = snapshotTemplate;
	if(snapshotFilePath.isEmpty()) silentSnapshot = false;
	if(currScriptNotSaved || snapshotFilePath.isEmpty())
	{
		snapshotFilePath =
			QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
		snapshotFilePath += QString("/%1-%2").arg(
			QString::number(m_frameShown), QString::number(m_outputIndex));
		snapshotFilePath += fileExtension;
	}

	QStringList saveFormatsList;
	for(const std::pair<QString, QString> & pair : extensionToFilterMap)
		saveFormatsList << pair.second;

	QString selectedFilter = extensionToFilterMap[fileExtension];

	if(currScriptNotSaved || !silentSnapshot)
	{
		snapshotFilePath = QFileDialog::getSaveFileName(this,
			tr("Save frame as image"), snapshotFilePath,
			saveFormatsList.join(";;"), &selectedFilter);
	}

	QFileInfo fileInfo(snapshotFilePath);
	QString suffix = fileInfo.suffix().toLower();

	QByteArray format("png");
	if((suffix == "webp") && webpSupported)
		format = "webp";
	else if(silentSnapshot && suffix != "png")
		snapshotFilePath += ".png";
	if(!snapshotFilePath.isEmpty())
	{
		bool success = m_framePixmap.save(snapshotFilePath, format,
			format == "webp" ? 100 :
			m_pSettingsManager->getPNGSnapshotCompressionLevel());
		if(success)
			m_pSettingsManager->setLastSnapshotExtension(suffix);
		else
		{
			QMessageBox::critical(this, tr("Image save error"),
				tr("Error while saving image ") + snapshotFilePath);
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

// END OF void PreviewDialog::slotZoomRatioChanged(double a_zoomRatio)
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

	m_ui.zoomRatioSpinBox->setScaleMode(scaleMode);
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

	int remainder = m_cpVideoInfo[m_outputIndex]->width - a_value;
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

	int remainder = m_cpVideoInfo[m_outputIndex]->height - a_value;
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

	m_ui.cropRightSpinBox->setValue(m_cpVideoInfo[m_outputIndex]->width -
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

	m_ui.cropBottomSpinBox->setValue(m_cpVideoInfo[m_outputIndex]->height -
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

	m_ui.cropWidthSpinBox->setValue(m_cpVideoInfo[m_outputIndex]->width -
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

	m_ui.cropHeightSpinBox->setValue(m_cpVideoInfo[m_outputIndex]->height -
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
	QString cropString;

	if(cropMode == CropMode::Absolute)
	{
		cropString = QString("***CLIP*** = core.std.CropAbs"
			"(***CLIP***, x=%1, y=%2, width=%3, height=%4)")
			.arg(m_ui.cropLeftSpinBox->value())
			.arg(m_ui.cropTopSpinBox->value())
			.arg(m_ui.cropWidthSpinBox->value())
			.arg(m_ui.cropHeightSpinBox->value());
	}
	else
	{
		cropString = QString("***CLIP*** = core.std.CropRel"
			"(***CLIP***, left=%1, top=%2, right=%3, bottom=%4)")
			.arg(m_ui.cropLeftSpinBox->value())
			.arg(m_ui.cropTopSpinBox->value())
			.arg(m_ui.cropRightSpinBox->value())
			.arg(m_ui.cropBottomSpinBox->value());
	}

	emit signalPasteIntoScriptAtNewLine(cropString);
}

// END OF void PreviewDialog::slotPasteCropSnippetIntoScript()
//==============================================================================

void PreviewDialog::slotCallAdvancedSettingsDialog()
{
	m_pAdvancedSettingsDialog->slotCall();
}

// END OF void PreviewDialog::slotCallAdvancedSettingsDialog()
//==============================================================================

void PreviewDialog::slotToggleTimeLinePanelVisible(bool a_timeLinePanelVisible)
{
	m_ui.timeLinePanel->setVisible(a_timeLinePanelVisible);
	m_pSettingsManager->setTimeLinePanelVisible(a_timeLinePanelVisible);
}

// END OF void PreviewDialog::slotToggleTimeLinePanelVisible(
//		bool a_timeLinePanelVisible)
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

	m_alwaysKeepCurrentFrame = m_pSettingsManager->getAlwaysKeepCurrentFrame();

	m_ui.frameNumberSlider->setUpdatesEnabled(false);

	QFont sliderLabelsFont =
		m_pSettingsManager->getTextFormat(TEXT_FORMAT_ID_TIMELINE).font();
	m_ui.frameNumberSlider->setLabelsFont(sliderLabelsFont);

	QColor bookmarksColor =
		m_pSettingsManager->getColor(COLOR_ID_TIMELINE_BOOKMARKS);
	m_ui.frameNumberSlider->setColor(TimeLineSlider::Bookmark, bookmarksColor);

	m_ui.frameNumberSlider->setUpdatesEnabled(true);
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
	m_pPreviewContextMenu->popup(QCursor::pos());
}

// END OF void PreviewDialog::slotPreviewAreaMouseRightButtonReleased()
//==============================================================================

void PreviewDialog::slotPreviewAreaMouseOverPoint(double a_pX, double a_pY)
{
	if(!m_cpFrame)
		return;

	if(!m_pStatusBarWidget->colorPickerVisible())
		return;

	double value1 = 0.0;
	double value2 = 0.0;
	double value3 = 0.0;
	int preview_values[3] = {0, 0, 0};

	size_t frameX = 0;
	size_t frameY = 0;

	double zoomRatio;

	int width = m_cpVSAPI->getFrameWidth(m_cpFrame, 0);
	int height = m_cpVSAPI->getFrameHeight(m_cpFrame, 0);
	const VSVideoFormat * cpFormat = m_cpVSAPI->getVideoFrameFormat(m_cpFrame);

	if(m_ui.cropPanel->isVisible())
	{
		zoomRatio = m_ui.cropZoomRatioSpinBox->value();
		int cropLeft = m_ui.cropLeftSpinBox->value();
		int cropTop = m_ui.cropTopSpinBox->value();
		int cropWidth = m_ui.cropWidthSpinBox->value();
		int cropHeight = m_ui.cropHeightSpinBox->value();
		frameX = (size_t)(a_pX * m_devicePixelRatio / zoomRatio + cropLeft);
		frameY = (size_t)(a_pY * m_devicePixelRatio / zoomRatio + cropTop);
		if(frameX >= (size_t)(cropLeft + cropWidth) ||
			frameY >= (size_t)(cropTop + cropHeight))
			return;
	}
	else
	{
		ZoomMode zoomMode = (ZoomMode)m_ui.zoomModeComboBox->currentData().toInt();
		if(zoomMode == ZoomMode::NoZoom)
		{
			zoomRatio = 1.0;
		}
		else if(zoomMode == ZoomMode::FixedRatio)
		{
			zoomRatio = m_ui.zoomRatioSpinBox->value();
		}
		else
		{
			if(m_framePixmap.isNull())
			{
				m_pStatusBarWidget->setColorPickerString(QString());
				return;
			}
			QRect previewRect = m_ui.previewArea->geometry();
			int cropSize = m_ui.previewArea->frameWidth() * 2;
			int frameWidth = previewRect.width() * m_devicePixelRatio - cropSize;
			int frameHeight = previewRect.height() * m_devicePixelRatio - cropSize;
			double scaleW = (double)frameWidth / m_framePixmap.width();
			double scaleH = (double)frameHeight / m_framePixmap.height();
			zoomRatio = scaleW < scaleH ? scaleW : scaleH;
		}
		if(zoomRatio <= 0)
			return;
		frameX = (size_t)(a_pX * m_devicePixelRatio / zoomRatio);
		frameY = (size_t)(a_pY * m_devicePixelRatio / zoomRatio);
		if((frameX >= (size_t)width) || (frameY >= (size_t)height))
			return;
	}

	value1 = valueAtPoint(frameX, frameY, 0);
	if(cpFormat->numPlanes > 1)
		value2 = valueAtPoint(frameX, frameY, 1);
	if(cpFormat->numPlanes > 2)
		value3 = valueAtPoint(frameX, frameY, 2);

	previewValueAtPoint(frameX, frameY, preview_values);

	QString l1("1");
	QString l2("2");
	QString l3("3");

	int colorFamily = m_cpVideoInfo[m_outputIndex]->format.colorFamily;

	if(colorFamily == cfYUV)
	{
		l1 = "Y";
		l2 = "U";
		l3 = "V";
	}
	else if(colorFamily == cfRGB)
	{
		l1 = "R";
		l2 = "G";
		l3 = "B";
	}

	QString colorString;

	if(colorFamily == cfGray)
		colorString = QString("Video: G:%1").arg(value1);
	else
	{
		colorString = QString("Video: %1:%2|%3:%4|%5:%6")
			.arg(l1).arg(value1).arg(l2).arg(value2).arg(l3).arg(value3);
	}

	QString coordString = QString("    Position: X:%1|Y:%2")
		.arg(frameX).arg(frameY);

	QString dispString = QString("    Display: R:%1|G:%2|B:%3")
		.arg(preview_values[0]).arg(preview_values[1]).arg(preview_values[2]);

	m_pStatusBarWidget->setColorPickerString(colorString + coordString + 
		dispString);
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
	m_pVapourSynthScriptProcessor->slotResetSettings();
	if(!m_playing)
		requestShowFrame(m_frameExpected);
}

// END OF void PreviewDialog::slotAdvancedSettingsChanged()
//==============================================================================

void PreviewDialog::slotToggleColorPicker(bool a_colorPickerVisible)
{
	m_pStatusBarWidget->setColorPickerVisible(a_colorPickerVisible);
	m_pSettingsManager->setColorPickerVisible(a_colorPickerVisible);
}

// END OF void PreviewDialog::slotToggleColorPicker(bool a_colorPickerVisible)
//==============================================================================

void PreviewDialog::slotSetPlayFPSLimit()
{
	double limit = m_ui.playFpsLimitSpinBox->value();
	if(limit < 1e-3)
		limit = 1e-3;
	m_nativePlaybackRate = false;
	PlayFPSLimitMode mode =
		(PlayFPSLimitMode)m_ui.playFpsLimitModeComboBox->currentData().toInt();
	if(mode == PlayFPSLimitMode::NoLimit)
		m_secondsBetweenFrames = 0.0;
	else if(mode == PlayFPSLimitMode::Custom)
		m_secondsBetweenFrames = 1.0 / limit;
	else if(mode == PlayFPSLimitMode::FromVideo)
	{
		if(!m_cpVideoInfo[m_outputIndex])
			m_secondsBetweenFrames = 0.0;
		else if(m_cpVideoInfo[m_outputIndex]->fpsNum == 0ll)
		{
			m_nativePlaybackRate = true;
			m_secondsBetweenFrames = 0.0;
		}
		else
		{
			m_secondsBetweenFrames =
				(double)m_cpVideoInfo[m_outputIndex]->fpsDen /
				(double)m_cpVideoInfo[m_outputIndex]->fpsNum;
		}
	}
	else
		Q_ASSERT(false);

	m_pSettingsManager->setPlayFPSLimitMode(mode);
	m_pSettingsManager->setPlayFPSLimit(limit);
}

// END OF void PreviewDialog::void slotSetPlayFPSLimit()
//==============================================================================

void PreviewDialog::slotPlay(bool a_play)
{
	if(m_playing == a_play)
		return;

	m_playing = a_play;
	m_pActionPlay->setChecked(m_playing);

	if(m_playing)
	{
		m_pActionPlay->setIcon(m_iconPause);
		m_lastFrameRequestedForPlay = m_frameShown;
		slotProcessPlayQueue();
	}
	else
	{
		clearFramesCache();
		m_pVapourSynthScriptProcessor->flushFrameTicketsQueue();
		m_pActionPlay->setIcon(m_iconPlay);
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

	int nextFrame = (m_frameShown + 1) % m_cpVideoInfo[m_outputIndex]->numFrames;
	Frame referenceFrame(nextFrame, m_outputIndex, nullptr);

	while(!m_framesCache[m_outputIndex].empty())
	{
		std::list<Frame>::const_iterator it =
			std::find(m_framesCache[m_outputIndex].begin(),
			m_framesCache[m_outputIndex].end(), referenceFrame);

		if(it == m_framesCache[m_outputIndex].end())
			break;

		if(m_nativePlaybackRate)
		{
			Q_ASSERT(m_cpVSAPI);
			const VSMap * frameProps = m_cpVSAPI->getFramePropertiesRO(
				it->cpOutputFrame);
			int err_num, err_den;
			int64_t durNum = m_cpVSAPI->mapGetInt(frameProps, "_DurationNum", 0, &err_num);
			int64_t durDen = m_cpVSAPI->mapGetInt(frameProps, "_DurationDen", 0, &err_den);
			if(err_num || err_den || durNum <= 0 || durDen <= 0)
			{
				// Fallback to custom
				double limit = m_ui.playFpsLimitSpinBox->value();
				if(limit < 1e-3)
					limit = 1e-3;
				m_secondsBetweenFrames = 1.0 / limit;
			}
			else
				m_secondsBetweenFrames = (double)durNum / (double)durDen;
		}
		hr_time_point now = hr_clock::now();
		double passed = duration_to_double(now - m_lastFrameShowTime);
		double secondsToNextFrame = m_secondsBetweenFrames - passed;
		if(secondsToNextFrame > 0)
		{
			int millisecondsToNextFrame = std::ceil(secondsToNextFrame * 1000);
			m_pPlayTimer->start(millisecondsToNextFrame);
			break;
		}

		setCurrentFrame(it->cpOutputFrame, it->cpPreviewFrame);
		m_lastFrameShowTime = hr_clock::now();

		m_frameShown = nextFrame;
		m_frameExpected = m_frameShown;
		m_ui.frameNumberSpinBox->setValue(m_frameExpected);
		m_ui.frameNumberSlider->setFrame(m_frameExpected);
		m_framesCache[m_outputIndex].erase(it);
		nextFrame = (m_frameShown + 1) % m_cpVideoInfo[m_outputIndex]->numFrames;
		referenceFrame.number = nextFrame;
	}

	nextFrame = (m_lastFrameRequestedForPlay + 1) %
		m_cpVideoInfo[m_outputIndex]->numFrames;

	while(((m_framesInQueue[m_outputIndex] + m_framesInProcess[m_outputIndex]) <
		m_maxThreads) &&
		(m_framesCache[m_outputIndex].size() <= m_cachedFramesLimit))
	{
		m_pVapourSynthScriptProcessor->requestFrameAsync(nextFrame,
			m_outputIndex, true);
		m_lastFrameRequestedForPlay = nextFrame;
		nextFrame = (nextFrame + 1) % m_cpVideoInfo[m_outputIndex]->numFrames;
	}

	m_processingPlayQueue = false;
}

// END OF void PreviewDialog::slotProcessPlayQueue()
//==============================================================================

void PreviewDialog::slotLoadChapters()
{
	if(m_playing)
		return;

	const QString lastUsedPath = m_pSettingsManager->getLastUsedPath();
	const QString filePath = QFileDialog::getOpenFileName(this,
		tr("Load chapters"), lastUsedPath,
		tr("Chapters file (*.txt;*.xml);;All files (*)"));
	QFile chaptersFile(filePath);
	if(!chaptersFile.open(QIODevice::ReadOnly | QIODevice::Text))
		return;

	const VSVideoInfo * cpVideoInfo =
		m_pVapourSynthScriptProcessor->videoInfo(m_outputIndex);
	if (cpVideoInfo->fpsDen == 0)
	{
		QString infoString = tr(
			"Warning: Load chapters requires clip having constant frame rate. Skipped");
		emit signalWriteLogMessage(mtWarning, infoString);
		return;
	}
	const double fps = (double)cpVideoInfo->fpsNum /
		(double)cpVideoInfo->fpsDen;

	static const QRegExp regExp(R"((\d{2}):(\d{2}):(\d{2})[\.:](\d{3})?)");
	while(!chaptersFile.atEnd())
	{
		const QByteArray line = chaptersFile.readLine();
		if(regExp.indexIn(line) < 0)
			continue;

		const QStringList timecodes = regExp.capturedTexts();

		const double timecode = timecodes.at(1).toDouble() * 3600.0 +
			timecodes.at(2).toDouble() * 60.0 + timecodes.at(3).toDouble() +
			timecodes.at(4).toDouble() / 1000;
		const int frameIndex = round(timecode * fps);
		m_ui.frameNumberSlider->addBookmark(frameIndex);
	}

	saveTimelineBookmarks();
}

// END OF void PreviewDialog::slotLoadBookmarks()
//==============================================================================

void PreviewDialog::slotClearBookmarks()
{
	if(m_playing)
		return;

	QMessageBox quesBox(this);
	vsedit::disableFontKerning(&quesBox);
	quesBox.setWindowTitle(tr("Clear Bookmarks"));
	quesBox.setText(tr("Do you really want to clear timeline bookmarks?"));
	quesBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	quesBox.setDefaultButton(QMessageBox::No);
	int result = quesBox.exec();
	if(result == QMessageBox::No)
		return;

	m_ui.frameNumberSlider->clearBookmarks();
	saveTimelineBookmarks();
}

// END OF void PreviewDialog::slotClearBookmarks()
//==============================================================================

void PreviewDialog::slotBookmarkCurrentFrame()
{
	if(m_playing)
		return;

	m_ui.frameNumberSlider->slotBookmarkCurrentFrame();
	saveTimelineBookmarks();
}

// END OF void PreviewDialog::slotBookmarkCurrentFrame()
//==============================================================================

void PreviewDialog::slotUnbookmarkCurrentFrame()
{
	if(m_playing)
		return;

	m_ui.frameNumberSlider->slotUnbookmarkCurrentFrame();
	saveTimelineBookmarks();
}

// END OF void PreviewDialog::slotUnbookmarkCurrentFrame()
//==============================================================================

void PreviewDialog::slotGoToPreviousBookmark()
{
	if(m_playing)
		return;

	m_ui.frameNumberSlider->slotGoToPreviousBookmark();
}

// END OF void PreviewDialog::slotGoToPreviousBookmark()
//==============================================================================

void PreviewDialog::slotGoToNextBookmark()
{
	if(m_playing)
		return;

	m_ui.frameNumberSlider->slotGoToNextBookmark();
}

// END OF void PreviewDialog::slotGoToNextBookmark()
//==============================================================================

void PreviewDialog::slotPasteShownFrameNumberIntoScript()
{
	emit signalPasteIntoScriptAtCursor(QString::number(m_frameShown));
}

// END OF void PreviewDialog::slotPasteShownFrameNumberIntoScript()
//==============================================================================

void PreviewDialog::slotSaveGeometry()
{
	m_pGeometrySaveTimer->stop();
	m_pSettingsManager->setPreviewDialogGeometry(m_windowGeometry);
}

// END OF void PreviewDialog::slotSaveGeometry()
//==============================================================================

void PreviewDialog::slotSwitchOutputIndex(int a_outputIndex)
{
	// Assuming there's a change
	if(a_outputIndex == m_outputIndex)
		return;

	// Assuming already initialized
	if(!m_pVapourSynthScriptProcessor->isInitialized())
		return;

	// Don't switch when playing (avoiding big troubles ahead)
	if(m_playing)
		return;

	// Don't switch when busy processing the current frame
	if(busy())
		return;

	// Check if output index is available
	const VSVideoInfo * vi = m_pVapourSynthScriptProcessor->
		videoInfo(a_outputIndex);
	if(!vi)
		return;
	m_outputIndex = a_outputIndex;

	// Update stuff
	m_clipName = "";
	m_sceneName = "";
	m_toChangeTitle = true;

	m_cpVideoInfo[m_outputIndex] = vi;
	int lastFrameNumber = m_cpVideoInfo[m_outputIndex]->numFrames - 1;
	m_ui.frameNumberSpinBox->setMaximum(lastFrameNumber);
	m_ui.frameNumberSlider->setFramesNumber(
		m_cpVideoInfo[m_outputIndex]->numFrames);
	if(m_cpVideoInfo[m_outputIndex]->fpsDen == 0)
		m_ui.frameNumberSlider->setFPS(0.0);
	else
	{
		m_ui.frameNumberSlider->setFPS(
			(double)m_cpVideoInfo[m_outputIndex]->fpsNum /
			(double)m_cpVideoInfo[m_outputIndex]->fpsDen);
	}

	m_pStatusBarWidget->setVideoInfo(m_cpVideoInfo[m_outputIndex], m_cpVSAPI);

	if(m_frameExpected > lastFrameNumber)
		m_frameExpected = lastFrameNumber;

	slotSetPlayFPSLimit();

	m_pVapourSynthScriptProcessor->requestFrameAsync(m_frameExpected,
		m_outputIndex, true);
}

// END OF void PreviewDialog::slotSwitchOutputIndex(int a_outputIndex)
//==============================================================================

void PreviewDialog::createActionsAndMenus()
{
	struct ActionToCreate
	{
		QAction ** ppAction;
		const char * id;
		bool checkable;
		const char * slotToConnect;
	};

	ActionToCreate actionsToCreate[] =
	{
		{&m_pActionFrameToClipboard, ACTION_ID_FRAME_TO_CLIPBOARD,
			false, SLOT(slotFrameToClipboard())},
		{&m_pActionSaveSnapshot, ACTION_ID_SAVE_SNAPSHOT,
			false, SLOT(slotSaveSnapshot())},
		{&m_pActionToggleZoomPanel, ACTION_ID_TOGGLE_ZOOM_PANEL,
			true, SLOT(slotToggleZoomPanelVisible(bool))},
		{&m_pActionSetZoomModeNoZoom, ACTION_ID_SET_ZOOM_MODE_NO_ZOOM,
			true, SLOT(slotZoomModeChanged())},
		{&m_pActionSetZoomModeFixedRatio, ACTION_ID_SET_ZOOM_MODE_FIXED_RATIO,
			true, SLOT(slotZoomModeChanged())},
		{&m_pActionSetZoomModeFitToFrame, ACTION_ID_SET_ZOOM_MODE_FIT_TO_FRAME,
			true, SLOT(slotZoomModeChanged())},
		{&m_pActionSetZoomScaleModeNearest,
			ACTION_ID_SET_ZOOM_SCALE_MODE_NEAREST,
			true, SLOT(slotScaleModeChanged())},
		{&m_pActionSetZoomScaleModeBilinear,
			ACTION_ID_SET_ZOOM_SCALE_MODE_BILINEAR,
			true, SLOT(slotScaleModeChanged())},
		{&m_pActionToggleCropPanel, ACTION_ID_TOGGLE_CROP_PANEL,
			true, SLOT(slotToggleCropPanelVisible(bool))},
		{&m_pActionToggleTimeLinePanel, ACTION_ID_TOGGLE_TIMELINE_PANEL,
			true, SLOT(slotToggleTimeLinePanelVisible(bool))},
		{&m_pActionSetTimeLineModeTime, ACTION_ID_SET_TIMELINE_MODE_TIME,
			true, SLOT(slotTimeLineModeChanged())},
		{&m_pActionSetTimeLineModeFrames, ACTION_ID_SET_TIMELINE_MODE_FRAMES,
			true, SLOT(slotTimeLineModeChanged())},
		{&m_pActionTimeStepForward, ACTION_ID_TIME_STEP_FORWARD,
			false, SLOT(slotTimeStepForward())},
		{&m_pActionTimeStepBack, ACTION_ID_TIME_STEP_BACK,
			false, SLOT(slotTimeStepBack())},
		{&m_pActionPasteCropSnippetIntoScript,
			ACTION_ID_PASTE_CROP_SNIPPET_INTO_SCRIPT,
			false, SLOT(slotPasteCropSnippetIntoScript())},
		{&m_pActionAdvancedSettingsDialog, ACTION_ID_ADVANCED_PREVIEW_SETTINGS,
			false, SLOT(slotCallAdvancedSettingsDialog())},
		{&m_pActionToggleColorPicker, ACTION_ID_TOGGLE_COLOR_PICKER,
			true, SLOT(slotToggleColorPicker(bool))},
		{&m_pActionPlay, ACTION_ID_PLAY,
			true, SLOT(slotPlay(bool))},
		{&m_pActionLoadChapters, ACTION_ID_TIMELINE_LOAD_CHAPTERS,
			false, SLOT(slotLoadChapters())},
		{&m_pActionClearBookmarks, ACTION_ID_TIMELINE_CLEAR_BOOKMARKS,
			false, SLOT(slotClearBookmarks())},
		{&m_pActionBookmarkCurrentFrame,
			ACTION_ID_TIMELINE_BOOKMARK_CURRENT_FRAME,
			false, SLOT(slotBookmarkCurrentFrame())},
		{&m_pActionUnbookmarkCurrentFrame,
			ACTION_ID_TIMELINE_UNBOOKMARK_CURRENT_FRAME,
			false, SLOT(slotUnbookmarkCurrentFrame())},
		{&m_pActionGoToPreviousBookmark,
			ACTION_ID_TIMELINE_GO_TO_PREVIOUS_BOOKMARK,
			false, SLOT(slotGoToPreviousBookmark())},
		{&m_pActionGoToNextBookmark, ACTION_ID_TIMELINE_GO_TO_NEXT_BOOKMARK,
			false, SLOT(slotGoToNextBookmark())},
		{&m_pActionPasteShownFrameNumberIntoScript,
			ACTION_ID_PASTE_SHOWN_FRAME_NUMBER_INTO_SCRIPT,
			false, SLOT(slotPasteShownFrameNumberIntoScript())},
		{&m_pActionSwitchToOutputIndex0, ACTION_ID_SET_OUTPUT_INDEX_0,
			false, SLOT(slotSwitchOutputIndex0())},
		{&m_pActionSwitchToOutputIndex1, ACTION_ID_SET_OUTPUT_INDEX_1,
			false, SLOT(slotSwitchOutputIndex1())},
		{&m_pActionSwitchToOutputIndex2, ACTION_ID_SET_OUTPUT_INDEX_2,
			false, SLOT(slotSwitchOutputIndex2())},
		{&m_pActionSwitchToOutputIndex3, ACTION_ID_SET_OUTPUT_INDEX_3,
			false, SLOT(slotSwitchOutputIndex3())},
		{&m_pActionSwitchToOutputIndex4, ACTION_ID_SET_OUTPUT_INDEX_4,
			false, SLOT(slotSwitchOutputIndex4())},
		{&m_pActionSwitchToOutputIndex5, ACTION_ID_SET_OUTPUT_INDEX_5,
			false, SLOT(slotSwitchOutputIndex5())},
		{&m_pActionSwitchToOutputIndex6, ACTION_ID_SET_OUTPUT_INDEX_6,
			false, SLOT(slotSwitchOutputIndex6())},
		{&m_pActionSwitchToOutputIndex7, ACTION_ID_SET_OUTPUT_INDEX_7,
			false, SLOT(slotSwitchOutputIndex7())},
		{&m_pActionSwitchToOutputIndex8, ACTION_ID_SET_OUTPUT_INDEX_8,
			false, SLOT(slotSwitchOutputIndex8())},
		{&m_pActionSwitchToOutputIndex9, ACTION_ID_SET_OUTPUT_INDEX_9,
			false, SLOT(slotSwitchOutputIndex9())},
	};

	for(ActionToCreate & item : actionsToCreate)
	{
		QAction * pAction =
			m_pSettingsManager->createStandardAction(item.id, this);
		*item.ppAction = pAction;
		pAction->setCheckable(item.checkable);
		m_settableActionsList.push_back(pAction);
	}

//------------------------------------------------------------------------------

	m_pPreviewContextMenu = new QMenu(this);
	vsedit::disableFontKerning(m_pPreviewContextMenu);
	m_pPreviewContextMenu->addAction(m_pActionFrameToClipboard);
	m_pPreviewContextMenu->addAction(m_pActionSaveSnapshot);
	m_pActionToggleZoomPanel->setChecked(
		m_pSettingsManager->getZoomPanelVisible());
	m_pPreviewContextMenu->addAction(m_pActionToggleZoomPanel);

//------------------------------------------------------------------------------

	m_pMenuZoomModes = new QMenu(m_pPreviewContextMenu);
	vsedit::disableFontKerning(m_pMenuZoomModes);
	m_pMenuZoomModes->setTitle(tr("Zoom mode"));
	m_pPreviewContextMenu->addMenu(m_pMenuZoomModes);

	m_pActionGroupZoomModes = new QActionGroup(this);

	ZoomMode zoomMode = m_pSettingsManager->getZoomMode();

	struct ZoomModeAction
	{
		QAction * pAction;
		ZoomMode zoomMode;
	};

	ZoomModeAction zoomModeActions[] =
	{
		{m_pActionSetZoomModeNoZoom, ZoomMode::NoZoom},
		{m_pActionSetZoomModeFixedRatio, ZoomMode::FixedRatio},
		{m_pActionSetZoomModeFitToFrame, ZoomMode::FitToFrame},
	};

	for(ZoomModeAction & action : zoomModeActions)
	{
		QString id = action.pAction->data().toString();
		action.pAction->setActionGroup(m_pActionGroupZoomModes);
		m_pMenuZoomModes->addAction(action.pAction);
		m_actionIDToZoomModeMap[id] = action.zoomMode;
		addAction(action.pAction);
		if(zoomMode == action.zoomMode)
			action.pAction->setChecked(true);
	}

//------------------------------------------------------------------------------

	m_pMenuZoomScaleModes = new QMenu(m_pPreviewContextMenu);
	vsedit::disableFontKerning(m_pMenuZoomScaleModes);
	m_pMenuZoomScaleModes->setTitle(tr("Zoom scale mode"));
	m_pPreviewContextMenu->addMenu(m_pMenuZoomScaleModes);

	m_pActionGroupZoomScaleModes = new QActionGroup(this);

	Qt::TransformationMode scaleMode = m_pSettingsManager->getScaleMode();

	struct ScaleModeAction
	{
		QAction * pAction;
		Qt::TransformationMode scaleMode;
	};

	ScaleModeAction scaleModeActions[] =
	{
		{m_pActionSetZoomScaleModeNearest, Qt::FastTransformation},
		{m_pActionSetZoomScaleModeBilinear, Qt::SmoothTransformation},
	};

	for(ScaleModeAction & action : scaleModeActions)
	{
		QString id = action.pAction->data().toString();
		action.pAction->setActionGroup(m_pActionGroupZoomScaleModes);
		m_pMenuZoomScaleModes->addAction(action.pAction);
		m_actionIDToZoomScaleModeMap[id] = action.scaleMode;
		addAction(action.pAction);
		if(scaleMode == action.scaleMode)
			action.pAction->setChecked(true);
	}

	bool noZoom = (zoomMode == ZoomMode::NoZoom);
	m_pMenuZoomScaleModes->setEnabled(!noZoom);

//------------------------------------------------------------------------------

	m_pPreviewContextMenu->addAction(m_pActionToggleCropPanel);
	m_pPreviewContextMenu->addAction(m_pActionToggleTimeLinePanel);
	m_pActionToggleTimeLinePanel->setChecked(
		m_pSettingsManager->getTimeLinePanelVisible());

//------------------------------------------------------------------------------

	m_pMenuTimeLineModes= new QMenu(m_pPreviewContextMenu);
	vsedit::disableFontKerning(m_pMenuTimeLineModes);
	m_pMenuTimeLineModes->setTitle(tr("Timeline display mode"));
	m_pPreviewContextMenu->addMenu(m_pMenuTimeLineModes);

	m_pActionGroupTimeLineModes = new QActionGroup(this);

	TimeLineSlider::DisplayMode timeLineMode =
		m_pSettingsManager->getTimeLineMode();

	struct TimeLineModeAction
	{
		QAction * pAction;
		TimeLineSlider::DisplayMode timeLineMode;
	};

	TimeLineModeAction timeLineModeAction[] =
	{
		{m_pActionSetTimeLineModeTime, TimeLineSlider::DisplayMode::Time},
		{m_pActionSetTimeLineModeFrames, TimeLineSlider::DisplayMode::Frames},
	};

	for(TimeLineModeAction & action : timeLineModeAction)
	{
		QString id = action.pAction->data().toString();
		action.pAction->setActionGroup(m_pActionGroupTimeLineModes);
		m_pMenuTimeLineModes->addAction(action.pAction);
		m_actionIDToTimeLineModeMap[id] = action.timeLineMode;
		addAction(action.pAction);
		if(timeLineMode == action.timeLineMode)
			action.pAction->setChecked(true);
	}

//------------------------------------------------------------------------------

	addAction(m_pActionTimeStepForward);
	addAction(m_pActionTimeStepBack);

	m_pActionToggleColorPicker->setChecked(
		m_pSettingsManager->getColorPickerVisible());
	m_pPreviewContextMenu->addAction(m_pActionToggleColorPicker);

	m_pActionPlay->setChecked(false);
	addAction(m_pActionPlay);

	addAction(m_pActionLoadChapters);
	addAction(m_pActionClearBookmarks);
	addAction(m_pActionBookmarkCurrentFrame);
	addAction(m_pActionUnbookmarkCurrentFrame);
	addAction(m_pActionGoToPreviousBookmark);
	addAction(m_pActionGoToNextBookmark);

	addAction(m_pActionPasteShownFrameNumberIntoScript);
	m_pPreviewContextMenu->addAction(m_pActionPasteShownFrameNumberIntoScript);

//------------------------------------------------------------------------------

	addAction(m_pActionSwitchToOutputIndex0);
	addAction(m_pActionSwitchToOutputIndex1);
	addAction(m_pActionSwitchToOutputIndex2);
	addAction(m_pActionSwitchToOutputIndex3);
	addAction(m_pActionSwitchToOutputIndex4);
	addAction(m_pActionSwitchToOutputIndex5);
	addAction(m_pActionSwitchToOutputIndex6);
	addAction(m_pActionSwitchToOutputIndex7);
	addAction(m_pActionSwitchToOutputIndex8);
	addAction(m_pActionSwitchToOutputIndex9);

//------------------------------------------------------------------------------

	for(ActionToCreate & item : actionsToCreate)
	{
		const char * signal =
			item.checkable ? SIGNAL(toggled(bool)) : SIGNAL(triggered());
		connect(*item.ppAction, signal, this, item.slotToConnect);
	}
}

// END OF void PreviewDialog::createActionsAndMenus()
//==============================================================================

void PreviewDialog::setUpZoomPanel()
{
	m_ui.zoomPanel->setVisible(m_pSettingsManager->getZoomPanelVisible());

	m_ui.zoomRatioSpinBox->setLocale(QLocale("C"));

	m_ui.zoomCheckButton->setDefaultAction(m_pActionToggleZoomPanel);

	m_ui.zoomModeComboBox->addItem(QIcon(":zoom_no_zoom.png"),
		tr("No zoom"), (int)ZoomMode::NoZoom);
	m_ui.zoomModeComboBox->addItem(QIcon(":zoom_fixed_ratio.png"),
		tr("Fixed ratio"), (int)ZoomMode::FixedRatio);
	m_ui.zoomModeComboBox->addItem(QIcon(":zoom_fit_to_frame.png"),
		tr("Fit to frame"), (int)ZoomMode::FitToFrame);

	ZoomMode zoomMode = m_pSettingsManager->getZoomMode();
	int comboIndex = m_ui.zoomModeComboBox->findData((int)zoomMode);
	if(comboIndex != -1)
		m_ui.zoomModeComboBox->setCurrentIndex(comboIndex);
	bool fixedRatio(zoomMode == ZoomMode::FixedRatio);
	m_ui.zoomRatioSpinBox->setEnabled(fixedRatio);

	double zoomRatio = m_pSettingsManager->getZoomRatio();
	m_ui.zoomRatioSpinBox->setValue(zoomRatio);

	m_ui.scaleModeComboBox->addItem(tr("Nearest"),
		(int)Qt::FastTransformation);
	m_ui.scaleModeComboBox->addItem(tr("Bilinear"),
		(int)Qt::SmoothTransformation);
	bool noZoom = (zoomMode == ZoomMode::NoZoom);
	m_ui.scaleModeComboBox->setEnabled(!noZoom);

	Qt::TransformationMode scaleMode = m_pSettingsManager->getScaleMode();
	m_ui.zoomRatioSpinBox->setScaleMode(scaleMode);
	comboIndex = m_ui.scaleModeComboBox->findData((int)scaleMode);
	if(comboIndex != -1)
		m_ui.scaleModeComboBox->setCurrentIndex(comboIndex);

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
	m_ui.timeLinePanel->setVisible(
		m_pSettingsManager->getTimeLinePanelVisible());

    m_ui.playButton->setDefaultAction(m_pActionPlay);
    m_ui.timeLineCheckButton->setDefaultAction(m_pActionToggleTimeLinePanel);
    m_ui.timeStepForwardButton->setDefaultAction(m_pActionTimeStepForward);
    m_ui.timeStepBackButton->setDefaultAction(m_pActionTimeStepBack);

    m_ui.playFpsLimitModeComboBox->addItem(tr("From video"),
		(int)PlayFPSLimitMode::FromVideo);
    m_ui.playFpsLimitModeComboBox->addItem(tr("No limit"),
		(int)PlayFPSLimitMode::NoLimit);
    m_ui.playFpsLimitModeComboBox->addItem(tr("Custom"),
		(int)PlayFPSLimitMode::Custom);

	PlayFPSLimitMode playFpsLimitMode =
		m_pSettingsManager->getPlayFPSLimitMode();
	int comboIndex = m_ui.playFpsLimitModeComboBox->findData(
		(int)playFpsLimitMode);
	if(comboIndex != -1)
		m_ui.playFpsLimitModeComboBox->setCurrentIndex(comboIndex);

	m_ui.playFpsLimitSpinBox->setLocale(QLocale("C"));
	double customFPS = m_pSettingsManager->getPlayFPSLimit();
	m_ui.playFpsLimitSpinBox->setValue(customFPS);

	slotSetPlayFPSLimit();

	m_ui.loadChaptersButton->setDefaultAction(m_pActionLoadChapters);
	m_ui.clearBookmarksButton->setDefaultAction(m_pActionClearBookmarks);
	m_ui.bookmarkCurrentFrameButton->setDefaultAction(
		m_pActionBookmarkCurrentFrame);
	m_ui.unbookmarkCurrentFrameButton->setDefaultAction(
		m_pActionUnbookmarkCurrentFrame);
	m_ui.goToPreviousBookmarkButton->setDefaultAction(
		m_pActionGoToPreviousBookmark);
	m_ui.goToNextBookmarkButton->setDefaultAction(
		m_pActionGoToNextBookmark);

    double timeStep = m_pSettingsManager->getTimeStep();
    m_ui.timeStepEdit->setTime(vsedit::secondsToQTime(timeStep));

    m_ui.timeLineModeComboBox->addItem(QIcon(":timeline.png"), tr("Time"),
		(int)TimeLineSlider::DisplayMode::Time);
    m_ui.timeLineModeComboBox->addItem(QIcon(":timeline_frames.png"),
		tr("Frames"), (int)TimeLineSlider::DisplayMode::Frames);

	TimeLineSlider::DisplayMode timeLineMode =
		m_pSettingsManager->getTimeLineMode();
	comboIndex = m_ui.timeLineModeComboBox->findData((int)timeLineMode);
	if(comboIndex != -1)
		m_ui.timeLineModeComboBox->setCurrentIndex(comboIndex);

	connect(m_ui.timeStepEdit, SIGNAL(timeChanged(const QTime &)),
		this, SLOT(slotTimeStepChanged(const QTime &)));
	connect(m_ui.timeLineModeComboBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(slotTimeLineModeChanged()));
	connect(m_ui.playFpsLimitModeComboBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(slotSetPlayFPSLimit()));
	connect(m_ui.playFpsLimitSpinBox, SIGNAL(valueChanged(double)),
		this, SLOT(slotSetPlayFPSLimit()));
}

// END OF void PreviewDialog::setUpTimeLinePanel()
//==============================================================================

void PreviewDialog::setUpCropPanel()
{
	m_ui.cropCheckButton->setDefaultAction(m_pActionToggleCropPanel);

	m_ui.cropModeComboBox->addItem(tr("Absolute"), (int)CropMode::Absolute);
	m_ui.cropModeComboBox->addItem(tr("Relative"), (int)CropMode::Relative);
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

bool PreviewDialog::requestShowFrame(int a_frameNumber)
{
	if(!m_pVapourSynthScriptProcessor->isInitialized())
		return false;

	if((m_frameShown != -1) && (m_frameShown != m_frameExpected))
		return false;

	m_pVapourSynthScriptProcessor->requestFrameAsync(a_frameNumber,
		m_outputIndex, true);
	return true;
}

// END OF bool PreviewDialog::requestShowFrame(int a_frameNumber)
//==============================================================================

void PreviewDialog::setPreviewPixmap()
{
#if (QT_VERSION_MAJOR < 6)
	m_devicePixelRatio = 1;
#else
	m_devicePixelRatio = window()->devicePixelRatioF();
	if(!m_framePixmap.isNull())
		m_framePixmap.setDevicePixelRatio(m_devicePixelRatio);
#endif
	if(m_ui.cropPanel->isVisible())
	{
		int cropLeft = m_ui.cropLeftSpinBox->value();
		int cropTop = m_ui.cropTopSpinBox->value();
		int cropWidth = m_ui.cropWidthSpinBox->value();
		int cropHeight = m_ui.cropHeightSpinBox->value();
		QPixmap croppedPixmap = m_framePixmap.copy(cropLeft, cropTop,
			cropWidth, cropHeight);
		int ratio = m_ui.cropZoomRatioSpinBox->value();

		if(abs(ratio * m_devicePixelRatio - 1) < 1e-7)
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
		frameWidth = m_framePixmap.width() * ratio;
		frameHeight = m_framePixmap.height() * ratio;
	}
	else
	{
		if(m_framePixmap.isNull())
			return;
		QRect previewRect = m_ui.previewArea->geometry();
		int cropSize = m_ui.previewArea->frameWidth() * 2;
		frameWidth = previewRect.width() * m_devicePixelRatio - cropSize;
		frameHeight = previewRect.height() * m_devicePixelRatio - cropSize;
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

	m_ui.cropLeftSpinBox->setMaximum(m_cpVideoInfo[m_outputIndex]->width - 1);
	m_ui.cropLeftSpinBox->setValue(0);
	m_ui.cropTopSpinBox->setMaximum(m_cpVideoInfo[m_outputIndex]->height - 1);
	m_ui.cropTopSpinBox->setValue(0);

	m_ui.cropWidthSpinBox->setMaximum(m_cpVideoInfo[m_outputIndex]->width);
	m_ui.cropWidthSpinBox->setValue(m_cpVideoInfo[m_outputIndex]->width);
	m_ui.cropHeightSpinBox->setMaximum(m_cpVideoInfo[m_outputIndex]->height);
	m_ui.cropHeightSpinBox->setValue(m_cpVideoInfo[m_outputIndex]->height);

	m_ui.cropRightSpinBox->setMaximum(m_cpVideoInfo[m_outputIndex]->width - 1);
	m_ui.cropRightSpinBox->setValue(0);
	m_ui.cropBottomSpinBox->setMaximum(m_cpVideoInfo[m_outputIndex]->height - 1);
	m_ui.cropBottomSpinBox->setValue(0);

	recalculateCropMods();

	END_CROP_VALUES_CHANGE
}

// END OF void PreviewDialog::resetCropSpinBoxes()
//==============================================================================

void PreviewDialog::setCurrentFrame(const VSFrame * a_cpOutputFrame,
	const VSFrame * a_cpPreviewFrame)
{
	Q_ASSERT(m_cpVSAPI);
	m_cpVSAPI->freeFrame(m_cpFrame);
	m_cpFrame = a_cpOutputFrame;
	// Copy clip and scene names
	const VSMap *props = m_cpVSAPI->getFramePropertiesRO(m_cpFrame);
	int success;
	bool toChangeTitle = m_toChangeTitle;
	const char * clipName = m_cpVSAPI->mapGetData(
		props, "Name", 0, &success);
	if(m_clipName != clipName) // can be nullptr?
	{
		m_clipName = QString(clipName ? clipName : "");
		toChangeTitle = true;
	}
	const char * sceneName = m_cpVSAPI->mapGetData(
		props, "SceneName", 0, &success);
	if(m_sceneName != sceneName)
	{
		m_sceneName = QString(sceneName ? sceneName : "");
		toChangeTitle = true;
	}
	if(toChangeTitle)
	{
		m_toChangeTitle = false;
		setTitle();
	}
	if (m_cpPreviewFrame)
	{
		m_cpVSAPI->freeFrame(m_cpPreviewFrame);
		m_cpPreviewFrame = nullptr;
	}
	m_framePixmap = pixmapFromRGB(a_cpPreviewFrame);
	m_cpPreviewFrame = a_cpPreviewFrame;
	setPreviewPixmap();
	QPointF pixelPos = m_ui.previewArea->pixelPosition();
	m_ui.previewArea->checkMouseOverPreview(pixelPos);
}

// END OF void PreviewDialog::setCurrentFrame(
//		const VSFrame * a_cpOutputFrame,
//		const VSFrame * a_cpPreviewFrame)
//==============================================================================

double PreviewDialog::valueAtPoint(size_t a_x, size_t a_y, int a_plane)
{
	Q_ASSERT(m_cpVSAPI);

	if(!m_cpFrame)
		return 0.0;

	const VSVideoFormat * cpFormat = m_cpVSAPI->getVideoFrameFormat(m_cpFrame);

	Q_ASSERT((a_plane >= 0) && (a_plane < cpFormat->numPlanes));

    const uint8_t * cpPlane =
		m_cpVSAPI->getReadPtr(m_cpFrame, a_plane);

	size_t x = a_x;
	size_t y = a_y;

	if(a_plane != 0)
	{
		x = (a_x >> cpFormat->subSamplingW);
		y = (a_y >> cpFormat->subSamplingH);
	}
	int stride = m_cpVSAPI->getStride(m_cpFrame, a_plane);
	const uint8_t * cpLine = cpPlane + y * stride;

	double value = 0.0;

	if(cpFormat->sampleType == stInteger)
	{
		if(cpFormat->bytesPerSample == 1)
			value = (double)cpLine[x];
		else if(cpFormat->bytesPerSample == 2)
			value = (double)((uint16_t *)cpLine)[x];
		else if(cpFormat->bytesPerSample == 4)
			value = (double)((uint32_t *)cpLine)[x];
	}
	else if(cpFormat->sampleType == stFloat)
	{
		if(cpFormat->bytesPerSample == 2)
		{
			vsedit::FP16 half;
			half.u = ((uint16_t *)cpLine)[x];
			vsedit::FP32 single = vsedit::halfToSingle(half);
			value = (double)single.f;
		}
		else if(cpFormat->bytesPerSample == 4)
			value = (double)((float *)cpLine)[x];
	}

	return value;
}

// END OF double PreviewDialog::valueAtPoint(size_t a_x, size_t a_y,
//		int a_plane)
//==============================================================================

void PreviewDialog::previewValueAtPoint(size_t a_x, size_t a_y, int a_ret[])
{
	// Read RGB values on screen from packed Gray8

	if(!m_cpPreviewFrame)
		return;

	const VSMap *props = m_cpVSAPI->getFramePropertiesRO(m_cpPreviewFrame);
	enum p2p_packing packing_fmt =
		static_cast<p2p_packing>(m_cpVSAPI->mapGetInt(props, "PackingFormat",
		0, nullptr));
	bool is_10_bits = (packing_fmt == p2p_rgb30);
	if (!is_10_bits)
	{
		Q_ASSERT(packing_fmt == p2p_argb32);
	}

    const uint8_t * cpPlane = m_cpVSAPI->getReadPtr(m_cpPreviewFrame, 0);

	size_t x = a_x;
	size_t y = a_y;

	int stride = m_cpVSAPI->getStride(m_cpPreviewFrame, 0);
	const uint8_t * cpLoc = cpPlane + y * stride + x * 4;

	// libp2p will handle endianness
	p2p_buffer_param p = {};
	p.width = 1;
	p.height = 1;
	p.packing = packing_fmt;
	p.src[0] = cpLoc;
	p.src_stride[0] = 1;
	if (is_10_bits)
	{
		uint16_t unpacked[3];
		for (int plane = 0; plane < 3; ++plane)
		{
			p.dst[plane] = &unpacked[plane];
			p.dst_stride[plane] = 1;
		}
		p2p_unpack_frame(&p, 0);
		for (int plane = 0; plane < 3; ++plane)
		{
			a_ret[plane] = static_cast<int>(unpacked[plane]);
		}
	}
	else
	{
		uint8_t unpacked[3];
		for (int plane = 0; plane < 3; ++plane)
		{
			p.dst[plane] = &unpacked[plane];
			p.dst_stride[plane] = 1;
		}
		p2p_unpack_frame(&p, 0);
		for (int plane = 0; plane < 3; ++plane)
		{
			a_ret[plane] = static_cast<int>(unpacked[plane]);
		}
	}
}

// END OF void PreviewDialog::previewValueAtPoint(size_t a_x, size_t a_y,
//		int a_ret[])
//==============================================================================

QPixmap PreviewDialog::pixmapFromRGB(
	const VSFrame * a_cpFrame)
{
	if((!m_cpVSAPI) || (!a_cpFrame))
		return QPixmap();

	const VSVideoFormat * cpFormat = m_cpVSAPI->getVideoFrameFormat(a_cpFrame);
	Q_ASSERT(cpFormat);
	int wwidth = m_cpVSAPI->getFrameWidth(a_cpFrame, 0);

	if((cpFormat->colorFamily != cfGray)
		|| (cpFormat->bitsPerSample != 8)
		|| (wwidth % 4))
	{
		QString errorString = tr("Error forming pixmap from frame. "
			"Expected format Gray8 with width divisible by 4. ");
		emit signalWriteLogMessage(mtCritical, errorString);
		return QPixmap();
	}

	const VSMap *props = m_cpVSAPI->getFramePropertiesRO(a_cpFrame);
	enum p2p_packing packing_fmt = static_cast<p2p_packing>(
		m_cpVSAPI->mapGetInt(props, "PackingFormat", 0, nullptr));
	bool is_10_bits;
	if (packing_fmt == p2p_rgb30)
	{
		is_10_bits = true;
	}
	else if (packing_fmt == p2p_argb32)
	{
		is_10_bits = false;
	}
	else
	{
		QString errorString = tr("Error forming pixmap from frame. "
			"Expected frame being packed from RGB24 or RGB30.");
		emit signalWriteLogMessage(mtCritical, errorString);
		return QPixmap();
	}

	int width = wwidth / 4;
	int height = m_cpVSAPI->getFrameHeight(a_cpFrame, 0);
	int stride = m_cpVSAPI->getStride(a_cpFrame, 0);

	const uint8_t * pData = m_cpVSAPI->getReadPtr(a_cpFrame, 0);
	QImage frameImage(reinterpret_cast<const uchar *>(pData),
		width, height, stride, is_10_bits ?
		QImage::Format_RGB30 : QImage::Format_ARGB32);
	QPixmap framePixmap = QPixmap::fromImage(frameImage, Qt::NoFormatConversion);
	return framePixmap;
}

// END OF QPixmap PreviewDialog::pixmapFromRGB(
//		const VSFrame * a_cpFrame)
//==============================================================================

void PreviewDialog::setTitle()
{
	QString l_scriptName = scriptName();
	QString scriptNameTitle =
		l_scriptName.isEmpty() ? tr("(Untitled)") : l_scriptName;
	QString title = tr("Preview - Index %1 | ").arg(m_outputIndex);
	if(!m_clipName.isEmpty())
		title = title + tr("Name: %1 | ").arg(m_clipName);
	if(!m_sceneName.isEmpty())
		title = title + tr("Scene: %1 | ").arg(m_sceneName);
	title = title + scriptNameTitle;
	setWindowTitle(title);
}

// END OF void PreviewDialog::setTitle()
//==============================================================================

void PreviewDialog::saveTimelineBookmarks()
{
	QString l_scriptName = scriptName();
	if(l_scriptName.isEmpty())
		return;

	QString bookmarksFilePath = l_scriptName +
		QString(TIMELINE_BOOKMARKS_FILE_SUFFIX);
	QFile bookmarksFile(bookmarksFilePath);
	if(!bookmarksFile.open(QIODevice::WriteOnly))
		return;

	std::set<int> bookmarks = m_ui.frameNumberSlider->bookmarks();
	QStringList bookmarksStringList;
	for(int i : bookmarks)
		bookmarksStringList += QString::number(i);

	QString bookmarksString = bookmarksStringList.join(", ");
	bookmarksFile.write(bookmarksString.toUtf8());
	bookmarksFile.close();
}

// END OF void PreviewDialog::saveTimelineBookmarks()
//==============================================================================

void PreviewDialog::loadTimelineBookmarks()
{
	std::set<int> bookmarks;

	QString l_scriptName = scriptName();
	if(l_scriptName.isEmpty())
	{
		m_ui.frameNumberSlider->setBookmarks(bookmarks);
		return;
	}

	QString bookmarksFilePath = l_scriptName +
		QString(TIMELINE_BOOKMARKS_FILE_SUFFIX);
	QFile bookmarksFile(bookmarksFilePath);
	if(!bookmarksFile.open(QIODevice::ReadOnly))
	{
		m_ui.frameNumberSlider->setBookmarks(bookmarks);
		return;
	}

	QString bookmarksString = tr(bookmarksFile.readAll().data());
	bookmarksFile.close();

	QStringList bookmarksStringList = bookmarksString.split(",");
	for(const QString & string : bookmarksStringList)
	{
		bool converted = false;
		int i = string.simplified().toInt(&converted);
		if(converted)
			bookmarks.insert(i);
	}

	m_ui.frameNumberSlider->setBookmarks(bookmarks);
}

// END OF void PreviewDialog::loadTimelineBookmarks()
//==============================================================================

void PreviewDialog::saveGeometryDelayed()
{
	QApplication::processEvents();
	if(!isMaximized())
	{
		m_windowGeometry = saveGeometry();
		m_pGeometrySaveTimer->start();
	}
}

// END OF void PreviewDialog::saveGeometryDelayed()
//==============================================================================
