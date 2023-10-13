#include "preview_advanced_settings_dialog.h"

#include "../../../common-src/settings/settings_manager.h"
#include "../../../common-src/helpers.h"

#include <QMessageBox>

//==============================================================================

PreviewAdvancedSettingsDialog::PreviewAdvancedSettingsDialog(
	SettingsManager * a_pSettingsManager, QWidget * a_pParent) :
	QDialog(a_pParent, 
		  Qt::Dialog
		| Qt::CustomizeWindowHint
		| Qt::WindowTitleHint
		| Qt::WindowCloseButtonHint)
	, m_pSettingsManager(a_pSettingsManager)
{
	vsedit::disableFontKerning(this);
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
	m_ui.chromaResamplingFilterComboBox->addItem(tr("Spline64"),
		(int)ResamplingFilter::Spline64);
	m_ui.chromaResamplingFilterComboBox->addItem(tr("Lanczos"),
		(int)ResamplingFilter::Lanczos);

	m_ui.chromaPlacementComboBox->addItem(tr("Left / MPEG2"),
		(int)ChromaPlacement::LEFT);
	m_ui.chromaPlacementComboBox->addItem(tr("Center / MPEG1 / JPEG"),
		(int)ChromaPlacement::CENTER);
	m_ui.chromaPlacementComboBox->addItem(tr("Top-left"),
		(int)ChromaPlacement::TOP_LEFT);

	m_ui.ditherTypeComboBox->addItem("Error Diffusion",
		(int)DitherType::ERROR_DIFFUSION);
	m_ui.ditherTypeComboBox->addItem("None", (int)DitherType::NONE);
	m_ui.ditherTypeComboBox->addItem("Ordered", (int)DitherType::ORDERED);
	m_ui.ditherTypeComboBox->addItem("Random", (int)DitherType::RANDOM);

	connect(m_ui.okButton, SIGNAL(clicked()), this, SLOT(slotOk()));
	connect(m_ui.applyButton, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(m_ui.resetToDefaultButton, SIGNAL(clicked()),
		this, SLOT(slotResetToDefault()));
	connect(m_ui.cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	connect(m_ui.silentSnapshotCheckBox, SIGNAL(clicked()),
		this, SLOT(slotSilentSnapshotChanged()));
	connect(m_ui.argumentsHelpButton, SIGNAL(clicked()),
		this, SLOT(slotArgumentsHelpButtonPressed()));
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

	DitherType ditherType = m_pSettingsManager->getDitherType();
	comboIndex = m_ui.ditherTypeComboBox->findData((int)ditherType);
	if(comboIndex != -1)
		m_ui.ditherTypeComboBox->setCurrentIndex(comboIndex);

	m_ui.bicubicFilterParameterBSpinBox->setValue(
		m_pSettingsManager->getBicubicFilterParameterB());
	m_ui.bicubicFilterParameterCSpinBox->setValue(
		m_pSettingsManager->getBicubicFilterParameterC());
	m_ui.lanczosFilterTapsSpinBox->setValue(
		m_pSettingsManager->getLanczosFilterTaps());

	bool silentSnapshotEnabled = m_pSettingsManager->getSilentSnapshot();
	m_ui.silentSnapshotCheckBox->setChecked(silentSnapshotEnabled);
	m_ui.saveSnapshotTemplateLineEdit->setText(
		m_pSettingsManager->getSnapshotTemplate());
	m_ui.saveSnapshotTemplateLineEdit->setEnabled(silentSnapshotEnabled);

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
	m_pSettingsManager->setDitherType((DitherType)
		m_ui.ditherTypeComboBox->currentData().toInt());
	m_pSettingsManager->setSilentSnapshot(
		m_ui.silentSnapshotCheckBox->isChecked());
	m_pSettingsManager->setSnapshotTemplate(
		m_ui.saveSnapshotTemplateLineEdit->text());

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
	
	DitherType ditherType = DEFAULT_DITHER_TYPE;
	comboIndex = m_ui.ditherTypeComboBox->findData((int)ditherType);
	if(comboIndex != -1)
		m_ui.ditherTypeComboBox->setCurrentIndex(comboIndex);

	m_ui.silentSnapshotCheckBox->setChecked(DEFAULT_SILENT_SNAPSHOT);
	m_ui.saveSnapshotTemplateLineEdit->setText(DEFAULT_SNAPSHOT_TEMPLATE);
	m_ui.saveSnapshotTemplateLineEdit->setEnabled(false);
}

// END OF void PreviewAdvancedSettingsDialog::slotResetToDefault()
//==============================================================================

void PreviewAdvancedSettingsDialog::slotSilentSnapshotChanged()
{
	bool silentSnapshotEnabled = m_ui.silentSnapshotCheckBox->isChecked();
	m_ui.saveSnapshotTemplateLineEdit->setEnabled(silentSnapshotEnabled);

	emit signalSilentSnapshotChanged();
}

void PreviewAdvancedSettingsDialog::slotArgumentsHelpButtonPressed()
{
	QString argumentsHelpString = tr("Use the following placeholders:");
	argumentsHelpString += QString("\n%1 - %2")
		.arg(tr("{f}")).arg(tr("script file path"));
	argumentsHelpString += QString("\n%1 - %2")
		.arg(tr("{d}")).arg(tr("script file directory"));
	argumentsHelpString += QString("\n%1 - %2")
		.arg(tr("{n}")).arg(tr("script file name"));
	argumentsHelpString += QString("\n%1 - %2")
		.arg(tr("{o}")).arg(tr("output index"));
	argumentsHelpString += QString("\n%1 - %2")
		.arg(tr("{i}")).arg(tr("frame number"));
	argumentsHelpString += QString("\n%1 - %2")
		.arg(tr("{t}")).arg(tr("timestamp"));
	argumentsHelpString += QString("\n%1 - %2")
		.arg(tr("{nm}")).arg(tr("clip name"));
	argumentsHelpString += QString("\n%1 - %2")
		.arg(tr("{sc}")).arg(tr("scene name"));
	QString title = tr("Snapshot template arguments");
	QMessageBox msgBox(this);
	msgBox.setWindowTitle(title);
	msgBox.setText(argumentsHelpString);
	vsedit::disableFontKerning(&msgBox);
	msgBox.exec();
}
