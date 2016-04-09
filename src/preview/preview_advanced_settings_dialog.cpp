#include "preview_advanced_settings_dialog.h"

#include "../settings/settingsmanager.cpp"

//==============================================================================

PreviewAdvancedSettingsDialog::PreviewAdvancedSettingsDialog(
	SettingsManager * a_pSettingsManager, QWidget * a_pParent) :
	QDialog(a_pParent, (Qt::WindowFlags)0
		| Qt::Dialog
		| Qt::CustomizeWindowHint
		| Qt::WindowTitleHint
		| Qt::WindowCloseButtonHint)
	, m_pSettingsManager(a_pSettingsManager)
{
	m_ui.setupUi(this);
	setWindowIcon(QIcon(":settings.png"));

	m_ui.yuvToRgbConversionMatrixComboBox->addItem(
		trUtf8("BT.601 (SD content)"), (int)YuvToRgbConversionMatrix::Bt601);
	m_ui.yuvToRgbConversionMatrixComboBox->addItem(
		trUtf8("BT.709 (HD content)"), (int)YuvToRgbConversionMatrix::Bt709);

	m_ui.chromaResamplingFilterComboBox->addItem(trUtf8("Point"),
		(int)ResamplingFilter::Point);
	m_ui.chromaResamplingFilterComboBox->addItem(trUtf8("Bilinear"),
		(int)ResamplingFilter::Bilinear);
	m_ui.chromaResamplingFilterComboBox->addItem(trUtf8("Bicubic"),
		(int)ResamplingFilter::Bicubic);
	m_ui.chromaResamplingFilterComboBox->addItem(trUtf8("Spline16"),
		(int)ResamplingFilter::Spline16);
	m_ui.chromaResamplingFilterComboBox->addItem(trUtf8("Spline36"),
		(int)ResamplingFilter::Spline36);
	m_ui.chromaResamplingFilterComboBox->addItem(trUtf8("Lanczos"),
		(int)ResamplingFilter::Lanczos);

	m_ui.chromaPlacementComboBox->addItem(trUtf8("MPEG1"),
		(int)ChromaPlacement::MPEG1);
	m_ui.chromaPlacementComboBox->addItem(trUtf8("MPEG2"),
		(int)ChromaPlacement::MPEG2);

	connect(m_ui.okButton, SIGNAL(clicked()), this, SLOT(slotOk()));
	connect(m_ui.applyButton, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(m_ui.resetToDefaultButton, SIGNAL(clicked()),
		this, SLOT(slotResetToDefault()));
	connect(m_ui.cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

// END OF PreviewAdvancedSettingsDialog::PreviewAdvancedSettingsDialog(
//		SettingsManager * a_pSettingsManager, QWidget * a_pParent)
//==============================================================================

PreviewAdvancedSettingsDialog::~PreviewAdvancedSettingsDialog()
{
}

// END OF PreviewAdvancedSettingsDialog::~PreviewAdvancedSettingsDialog()
//==============================================================================

void PreviewAdvancedSettingsDialog::slotCall()
{
	YuvToRgbConversionMatrix matrix =
		m_pSettingsManager->getYuvToRgbConversionMatrix();
	int comboIndex =
		m_ui.yuvToRgbConversionMatrixComboBox->findData((int)matrix);
	if(comboIndex != -1)
		m_ui.yuvToRgbConversionMatrixComboBox->setCurrentIndex(comboIndex);

	ResamplingFilter filter = m_pSettingsManager->getChromaResamplingFilter();
	comboIndex = m_ui.chromaResamplingFilterComboBox->findData((int)filter);
	if(comboIndex != -1)
		m_ui.chromaResamplingFilterComboBox->setCurrentIndex(comboIndex);

	ChromaPlacement chromaPlacement = m_pSettingsManager->getChromaPlacement();
	comboIndex = m_ui.chromaPlacementComboBox->findData((int)chromaPlacement);
	if(comboIndex != -1)
		m_ui.chromaPlacementComboBox->setCurrentIndex(comboIndex);

	m_ui.bicubicFilterParameterBSpinBox->setValue(
		m_pSettingsManager->getBicubicFilterParameterB());
	m_ui.bicubicFilterParameterCSpinBox->setValue(
		m_pSettingsManager->getBicubicFilterParameterC());
	m_ui.lanczosFilterTapsSpinBox->setValue(
		m_pSettingsManager->getLanczosFilterTaps());

	show();
}

// END OF void PreviewAdvancedSettingsDialog::slotCall()
//==============================================================================

void PreviewAdvancedSettingsDialog::slotOk()
{
	slotApply();
	accept();
}

// END OF void PreviewAdvancedSettingsDialog::slotOk()
//==============================================================================

void PreviewAdvancedSettingsDialog::slotApply()
{
	m_pSettingsManager->setChromaResamplingFilter((ResamplingFilter)
		m_ui.chromaResamplingFilterComboBox->currentData().toInt());
	m_pSettingsManager->setYuvToRgbConversionMatrix((YuvToRgbConversionMatrix)
		m_ui.yuvToRgbConversionMatrixComboBox->currentData().toInt());
	m_pSettingsManager->setChromaPlacement((ChromaPlacement)
		m_ui.chromaPlacementComboBox->currentData().toInt());
	m_pSettingsManager->setBicubicFilterParameterB(
		m_ui.bicubicFilterParameterBSpinBox->value());
	m_pSettingsManager->setBicubicFilterParameterC(
		m_ui.bicubicFilterParameterCSpinBox->value());
	m_pSettingsManager->setLanczosFilterTaps(
		m_ui.lanczosFilterTapsSpinBox->value());

	emit signalSettingsChanged();
}

// END OF void PreviewAdvancedSettingsDialog::slotApply()
//==============================================================================

void PreviewAdvancedSettingsDialog::slotResetToDefault()
{
	YuvToRgbConversionMatrix matrix = DEFAULT_YUV_TO_RGB_CONVERSION_MATRIX;
	int comboIndex =
		m_ui.yuvToRgbConversionMatrixComboBox->findData((int)matrix);
	if(comboIndex != -1)
		m_ui.yuvToRgbConversionMatrixComboBox->setCurrentIndex(comboIndex);

	ResamplingFilter filter = DEFAULT_CHROMA_RESAMPLING_FILTER;
	comboIndex = m_ui.chromaResamplingFilterComboBox->findData((int)filter);
	if(comboIndex != -1)
		m_ui.chromaResamplingFilterComboBox->setCurrentIndex(comboIndex);

	ChromaPlacement chromaPlacement = DEFAULT_CHROMA_PLACEMENT;
	comboIndex = m_ui.chromaPlacementComboBox->findData((int)chromaPlacement);
	if(comboIndex != -1)
		m_ui.chromaPlacementComboBox->setCurrentIndex(comboIndex);

	m_ui.bicubicFilterParameterBSpinBox->setValue(
		DEFAULT_BICUBIC_FILTER_PARAMETER_B);
	m_ui.bicubicFilterParameterCSpinBox->setValue(
		DEFAULT_BICUBIC_FILTER_PARAMETER_C);
	m_ui.lanczosFilterTapsSpinBox->setValue(
		DEFAULT_LANCZOS_FILTER_TAPS);
}

// END OF void PreviewAdvancedSettingsDialog::slotResetToDefault()
//==============================================================================
