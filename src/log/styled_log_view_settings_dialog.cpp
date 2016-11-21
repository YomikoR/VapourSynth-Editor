#include "styled_log_view_settings_dialog.h"

#include "log_styles_model.h"

#include <cassert>

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
	assert(m_pLogStylesModel);
	return m_pLogStylesModel->styles();
}

// END OF std::vector<TextBlockStyle> StyledLogViewSettingsDialog::styles()
//		const
//==============================================================================

void StyledLogViewSettingsDialog::setStyles(
	const std::vector<TextBlockStyle> & a_styles)
{
	assert(m_pLogStylesModel);
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
