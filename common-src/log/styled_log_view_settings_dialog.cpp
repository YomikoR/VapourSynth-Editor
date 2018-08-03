#include "styled_log_view_settings_dialog.h"

#include "log_styles_model.h"

#include <QGuiApplication>
#include <QFontDialog>
#include <QColorDialog>

//==============================================================================

StyledLogViewSettingsDialog::StyledLogViewSettingsDialog(QWidget * a_pParent):
	  QDialog(a_pParent, (Qt::WindowFlags)0
		| Qt::Dialog
		| Qt::CustomizeWindowHint
		| Qt::WindowTitleHint
		| Qt::WindowCloseButtonHint)
	, m_pLogStylesModel(nullptr)
{
	m_ui.setupUi(this);

	m_pLogStylesModel = new LogStylesModel(this);
	m_ui.stylesView->setModel(m_pLogStylesModel);

	connect(m_ui.okButton, SIGNAL(clicked()), this, SLOT(slotOk()));
	connect(m_ui.applyButton, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(m_ui.cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

	connect(m_ui.stylesView, SIGNAL(clicked(const QModelIndex &)),
		this, SLOT(slotStyleSelected(const QModelIndex &)));

	connect(m_ui.fontButton, SIGNAL(clicked()),
		this, SLOT(slotFontButtonClicked()));
	connect(m_ui.textColorButton, SIGNAL(clicked()),
		this, SLOT(slotTextColorButtonClicked()));
	connect(m_ui.backgroundColorButton, SIGNAL(clicked()),
		this, SLOT(slotBackgroundColorButtonClicked()));
}

// END OF StyledLogViewSettingsDialog::StyledLogViewSettingsDialog(
//		QWidget * a_pParent)
//==============================================================================

StyledLogViewSettingsDialog::~StyledLogViewSettingsDialog()
{
}

// END OF StyledLogViewSettingsDialog::~StyledLogViewSettingsDialog()
//==============================================================================

std::vector<TextBlockStyle> StyledLogViewSettingsDialog::styles() const
{
	Q_ASSERT(m_pLogStylesModel);
	return m_pLogStylesModel->styles();
}

// END OF std::vector<TextBlockStyle> StyledLogViewSettingsDialog::styles()
//		const
//==============================================================================

void StyledLogViewSettingsDialog::setStyles(
	const std::vector<TextBlockStyle> & a_styles)
{
	Q_ASSERT(m_pLogStylesModel);
	m_pLogStylesModel->setStyles(a_styles);
	m_ui.stylesView->resizeRowsToContents();
}

// END OF void StyledLogViewSettingsDialog::setStyles(
//		const std::vector<TextBlockStyle> & a_styles)
//==============================================================================

void StyledLogViewSettingsDialog::slotOk()
{
	slotApply();
	close();
}

// END OF void StyledLogViewSettingsDialog::slotOk()
//==============================================================================

void StyledLogViewSettingsDialog::slotApply()
{
	emit signalSettingsChanged();
}

// END OF void StyledLogViewSettingsDialog::slotApply()
//==============================================================================

void StyledLogViewSettingsDialog::slotStyleSelected(const QModelIndex & a_index)
{
	TextBlockStyle style = m_pLogStylesModel->style(a_index);
	QTextCharFormat format = style.textFormat;

	enableStyleSettingsControls(!style.isAlias);

	QPalette palette = QGuiApplication::palette();
	QString textColorString = style.isAlias ?
		palette.color(QPalette::Disabled, QPalette::WindowText).name() :
		format.foreground().color().name();
	QString backgroundColorString = style.isAlias ?
		palette.color(QPalette::Disabled, QPalette::Window).name() :
		format.background().color().name();
	QString styleSheetString =
		QString("QLabel {background-color : \"%1\"; color : \"%2\";}")
		.arg(backgroundColorString).arg(textColorString);

	m_ui.fontLabel->setStyleSheet(styleSheetString);

	if(style.isAlias)
	{
		m_ui.fontLabel->clear();
	}
	else
	{
		m_ui.fontLabel->setFont(format.font());
		m_ui.fontLabel->setText(format.font().family());
	}
}

// END OF void StyledLogViewSettingsDialog::slotStyleSelected(
//		const QModelIndex & a_index)
//==============================================================================

void StyledLogViewSettingsDialog::slotFontButtonClicked()
{
	QModelIndex index = m_ui.stylesView->currentIndex();
	if(!index.isValid())
		return;

	TextBlockStyle style = m_pLogStylesModel->style(index);

	QFontDialog fontDialog;
	fontDialog.setCurrentFont(style.textFormat.font());
	int returnCode = fontDialog.exec();
	if(returnCode == QDialog::Rejected)
		return;

	QFont newFont = fontDialog.selectedFont();
	m_pLogStylesModel->setStyleFont(index.row(), newFont);
	slotStyleSelected(index);
}

// END OF void StyledLogViewSettingsDialog::slotFontButtonClicked()
//==============================================================================

void StyledLogViewSettingsDialog::slotTextColorButtonClicked()
{
	QModelIndex index = m_ui.stylesView->currentIndex();
	if(!index.isValid())
		return;

	TextBlockStyle style = m_pLogStylesModel->style(index);

	QColorDialog colorDialog;
	colorDialog.setCurrentColor(style.textFormat.foreground().color());

	int returnCode = colorDialog.exec();
	if(returnCode == QDialog::Rejected)
		return;

	QColor newColor = colorDialog.selectedColor();
	m_pLogStylesModel->setStyleTextColor(index.row(), newColor);
	slotStyleSelected(index);
}

// END OF void StyledLogViewSettingsDialog::slotTextColorButtonClicked()
//==============================================================================

void StyledLogViewSettingsDialog::slotBackgroundColorButtonClicked()
{
	QModelIndex index = m_ui.stylesView->currentIndex();
	if(!index.isValid())
		return;

	TextBlockStyle style = m_pLogStylesModel->style(index);

	QColorDialog colorDialog;
	colorDialog.setCurrentColor(style.textFormat.background().color());

	int returnCode = colorDialog.exec();
	if(returnCode == QDialog::Rejected)
		return;

	QColor newColor = colorDialog.selectedColor();
	m_pLogStylesModel->setStyleBackgroundColor(index.row(), newColor);
	slotStyleSelected(index);
}

// END OF void StyledLogViewSettingsDialog::slotBackgroundColorButtonClicked()
//==============================================================================

void StyledLogViewSettingsDialog::enableStyleSettingsControls(bool a_enable)
{
	QWidget * widgetsToEnable[] = {m_ui.fontButton, m_ui.textColorButton,
		m_ui.backgroundColorButton, m_ui.fontLabel};

	for(QWidget * pWidget : widgetsToEnable)
		pWidget->setEnabled(a_enable);
}

// END OF void StyledLogViewSettingsDialog::enableStyleSettingsControls(
//		bool a_enable)
//==============================================================================
