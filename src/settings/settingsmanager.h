#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QVariant>
#include <QByteArray>
#include <QStringList>
#include <QKeySequence>
#include <QTextCharFormat>
#include <QColor>

#include "../preview/timelineslider.h"

//==============================================================================

enum class ZoomMode
{
	NoZoom,
	FixedRatio,
	FitToFrame
};

enum class CropMode
{
	Absolute,
	Relative
};

enum class ResamplingFilter : int
{
	Point,
	Bilinear,
	Bicubic,
	Spline16,
	Spline36,
	Lanczos
};

enum class YuvToRgbConversionMatrix
{
	Bt601,
	Bt709
};

enum class ChromaPlacement
{
	MPEG1,
	MPEG2
};

//==============================================================================

extern const ResamplingFilter DEFAULT_CHROMA_RESAMPLING_FILTER;
extern const YuvToRgbConversionMatrix DEFAULT_YUV_TO_RGB_CONVERSION_MATRIX;
extern const ChromaPlacement DEFAULT_CHROMA_PLACEMENT;
extern const double DEFAULT_BICUBIC_FILTER_PARAMETER_B;
extern const double DEFAULT_BICUBIC_FILTER_PARAMETER_C;
extern const int DEFAULT_LANCZOS_FILTER_TAPS;

//==============================================================================

extern const char ACTION_ID_NEW_SCRIPT[];
extern const char ACTION_ID_OPEN_SCRIPT[];
extern const char ACTION_ID_SAVE_SCRIPT[];
extern const char ACTION_ID_SAVE_SCRIPT_AS[];
extern const char ACTION_ID_SETTINGS[];
extern const char ACTION_ID_PREVIEW[];
extern const char ACTION_ID_CHECK_SCRIPT[];
extern const char ACTION_ID_EXIT[];
extern const char ACTION_ID_ABOUT[];
extern const char ACTION_ID_AUTOCOMPLETE[];
extern const char ACTION_ID_SAVE_SNAPSHOT[];
extern const char ACTION_ID_TOGGLE_ZOOM_PANEL[];
extern const char ACTION_ID_SET_ZOOM_MODE_NO_ZOOM[];
extern const char ACTION_ID_SET_ZOOM_MODE_FIXED_RATIO[];
extern const char ACTION_ID_SET_ZOOM_MODE_FIT_TO_FRAME[];
extern const char ACTION_ID_SET_ZOOM_SCALE_MODE_NEAREST[];
extern const char ACTION_ID_SET_ZOOM_SCALE_MODE_BILINEAR[];
extern const char ACTION_ID_TOGGLE_CROP_PANEL[];
extern const char ACTION_ID_PASTE_CROP_SNIPPET_INTO_SCRIPT[];
extern const char ACTION_ID_FRAME_TO_CLIPBOARD[];
extern const char ACTION_ID_TOGGLE_TIMELINE_PANEL[];
extern const char ACTION_ID_SET_TIMELINE_MODE_TIME[];
extern const char ACTION_ID_SET_TIMELINE_MODE_FRAMES[];
extern const char ACTION_ID_TIME_STEP_FORWARD[];
extern const char ACTION_ID_TIME_STEP_BACK[];
extern const char ACTION_ID_ADVANCED_PREVIEW_SETTINGS[];
extern const char ACTION_ID_TOGGLE_COLOR_PICKER[];

//==============================================================================

extern const char TEXT_FORMAT_ID_COMMON_SCRIPT_TEXT[];
extern const char TEXT_FORMAT_ID_KEYWORD[];
extern const char TEXT_FORMAT_ID_OPERATOR[];
extern const char TEXT_FORMAT_ID_STRING[];
extern const char TEXT_FORMAT_ID_NUMBER[];
extern const char TEXT_FORMAT_ID_COMMENT[];
extern const char TEXT_FORMAT_ID_VS_CORE[];
extern const char TEXT_FORMAT_ID_VS_NAMESPACE[];
extern const char TEXT_FORMAT_ID_VS_FUNCTION[];
extern const char TEXT_FORMAT_ID_VS_ARGUMENT[];
extern const char TEXT_FORMAT_ID_TIMELINE[];

extern const char COLOR_ID_TEXT_BACKGROUND[];
extern const char COLOR_ID_ACTIVE_LINE[];

//==============================================================================

class SettingsManager : public QObject
{
	public:

		SettingsManager(QObject * a_pParent);
		virtual ~SettingsManager();

		//----------------------------------------------------------------------

		bool getPortableMode() const;

		bool setPortableMode(bool a_portableMod);

		//----------------------------------------------------------------------

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

		QString getLastUsedPath() const;

		bool setLastUsedPath(const QString& a_lastUsedPath);

		QByteArray getMainWindowGeometry() const;

		bool setMainWindowGeometry(const QByteArray & a_mainWindowGeometry);

		bool getMainWindowMaximized() const;

		bool setMainWindowMaximized(bool a_mainWindowMaximized);

		QByteArray getPreviewDialogGeometry() const;

		bool setPreviewDialogGeometry(
			const QByteArray & a_previewDialogGeometry);

		bool getPreviewDialogMaximized() const;

		bool setPreviewDialogMaximized(bool a_previewDialogMaximized);

		bool getAutoLoadLastScript() const;

		bool setAutoLoadLastScript(bool a_autoLoadLastScript);

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

		bool getPromptToSaveChanges() const;

		bool setPromptToSaveChanges(bool a_prompt);

		QStringList getRecentFilesList() const;

		bool addToRecentFilesList(const QString & a_filePath);

		unsigned int getMaxRecentFilesNumber() const;

		bool setMaxRecentFilesNumber(unsigned int a_maxRecentFilesNumber);

		QStringList getVapourSynthLibraryPaths() const;

		bool setVapourSynthLibraryPaths(const QStringList & a_pathsList);

		QStringList getVapourSynthPluginsPaths() const;

		bool setVapourSynthPluginsPaths(const QStringList & a_pathsList);

		QStringList getVapourSynthDocumentationPaths() const;

		bool setVapourSynthDocumentationPaths(const QStringList & a_pathsList);

		int getCharactersTypedToStartCompletion() const;

		bool setCharactersTypedToStartCompletion(int a_charactersNumber);

		TimeLineSlider::DisplayMode getTimeLineMode() const;

		bool setTimeLineMode(TimeLineSlider::DisplayMode a_timeLineMode);

		double getTimeStep() const;

		bool setTimeStep(double a_timeStep);

		ResamplingFilter getChromaResamplingFilter() const;

		bool setChromaResamplingFilter(ResamplingFilter a_filter);

		YuvToRgbConversionMatrix getYuvToRgbConversionMatrix() const;

		bool setYuvToRgbConversionMatrix(YuvToRgbConversionMatrix a_matrix);

		ChromaPlacement getChromaPlacement() const;

		bool setChromaPlacement(ChromaPlacement a_placement);

		double getBicubicFilterParameterB() const;

		bool setBicubicFilterParameterB(double a_parameterB);

		double getBicubicFilterParameterC() const;

		bool setBicubicFilterParameterC(double a_parameterC);

		int getLanczosFilterTaps() const;

		bool setLanczosFilterTaps(int a_taps);

		bool getColorPickerVisible() const;

		bool setColorPickerVisible(bool a_colorPickerVisible);

	private:

		QVariant valueInGroup(const QString & a_group, const QString & a_key,
			const QVariant & a_defaultValue = QVariant()) const;

		bool setValueInGroup(const QString & a_group, const QString & a_key,
			const QVariant & a_value);

		QVariant value(const QString & a_key, const QVariant & a_defaultValue =
			QVariant()) const;

		bool setValue(const QString & a_key, const QVariant & a_value);

		QString m_settingsFilePath;
};

//==============================================================================

#endif // SETTINGSMANAGER_H
