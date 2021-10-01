#include "preview_advanced_settings_dialog.h"

#include "../../../common-src/settings/settings_manager.h"

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

	m_ui.yuvMatrixCoefficientsComboBox->addItem(
		tr("709"), (int)YuvMatrixCoefficients::m709);
	m_ui.yuvMatrixCoefficientsComboBox->addItem(
		tr("470BG"), (int)YuvMatrixCoefficients::m470BG);
	m_ui.yuvMatrixCoefficientsComboBox->addItem(
		tr("170M"), (int)YuvMatrixCoefficients::m170M);
	m_ui.yuvMatrixCoefficientsComboBox->addItem(
		tr("2020 NCL"), (int)YuvMatrixCoefficients::m2020_NCL);
	m_ui.yuvMatrixCoefficientsComboBox->addItem(
		tr("2020 CL"), (int)YuvMatrixCoefficients::m2020_CL);

	m_ui.chromaResamplingFilterComboBox->addItem(tr("Point"),
		(int)ResamplingFilter::Point);
	m_ui.chromaResamplingFilterComboBox->addItem(tr("Bilinear"),
		(int)ResamplingFilter::Bilinear);
	m_ui.chromaResamplingFilterComboBox->addItem(tr("Bicubic"),
		(int)ResamplingFilter::Bicubic);
	m_ui.chromaResamplingFilterComboBox->addItem(tr("Spline16"),
		(int)ResamplingFilter::Spline16);
	m_ui.chromaResamplingFilterComboBox->addItem(tr("Spline36"),
		(int)ResamplingFilter::Spline36);
	m_ui.chromaResamplingFilterComboBox->addItem(tr("Lanczos"),
		(int)ResamplingFilter::Lanczos);

	m_ui.chromaPlacementComboBox->addItem(tr("MPEG1"),
		(int)ChromaPlacement::MPEG1);
	m_ui.chromaPlacementComboBox->addItem(tr("MPEG2"),
		(int)ChromaPlacement::MPEG2);
	m_ui.chromaPlacementComboBox->addItem(tr("DV"),
		(int)ChromaPlacement::DV);

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
	YuvMatrixCoefficients matrix =
		m_pSettingsManager->getYuvMatrixCoefficients();
	int comboIndex =
		m_ui.yuvMatrixCoefficientsComboBox->findData((int)matrix);
	if(comboIndex != -1)
		m_ui.yuvMatrixCoefficientsComboBox->setCurrentIndex(comboIndex);

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
	m_pSettingsManager->setYuvMatrixCoefficients((YuvMatrixCoefficients)
		m_ui.yuvMatrixCoefficientsComboBox->currentData().toInt());
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
	YuvMatrixCoefficients matrix = DEFAULT_YUV_MATRIX_COEFFICIENTS;
	int comboIndex = m_ui.yuvMatrixCoefficientsComboBox->findData((int)matrix);
	if(comboIndex != -1)
		m_ui.yuvMatrixCoefficientsComboBox->setCurrentIndex(comboIndex);

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
