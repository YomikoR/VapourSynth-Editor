#include "templates_dialog.h"

#include "../settings/settingsmanager.h"

#include <cassert>

//==============================================================================

TemplatesDialog::TemplatesDialog(SettingsManager * a_pSettingsManager,
	QWidget * a_pParent, Qt::WindowFlags a_flags) :
	  QDialog(a_pParent, a_flags)
	, m_pSettingsManager(a_pSettingsManager)
{
	assert(m_pSettingsManager);
	m_ui.setupUi(this);

	m_ui.snippetEdit->setSettingsManager(m_pSettingsManager);
	m_ui.newScriptTemplateEdit->setSettingsManager(m_pSettingsManager);

	connect(m_ui.snippetPasteIntoScriptButton, SIGNAL(clicked()),
		this, SLOT(slotSnippetPasteIntoScriptButtonClicked()));
	connect(m_ui.snippetSaveButton, SIGNAL(clicked()),
		this, SLOT(slotSnippetSaveButtonClicked()));
	connect(m_ui.snippetDeleteButton, SIGNAL(clicked()),
		this, SLOT(slotSnippetDeleteButtonClicked()));
	connect(m_ui.newScriptTemplateRevertButton, SIGNAL(clicked()),
		this, SLOT(slotNewScriptTemplateRevertButtonClicked()));
	connect(m_ui.newScriptTemplateLoadDefaultButton, SIGNAL(clicked()),
		this, SLOT(slotNewScriptTemplateLoadDefaultButtonClicked()));
	connect(m_ui.newScriptTemplateSaveButton, SIGNAL(clicked()),
		this, SLOT(slotNewScriptTemplateSaveButtonClicked()));

	slotLoadSettings();
}

// END OF TemplatesDialog::TemplatesDialog(SettingsManager * a_pSettingsManager,
//		QWidget * a_pParent, Qt::WindowFlags a_flags)
//==============================================================================

TemplatesDialog::~TemplatesDialog()
{
}

// END OF TemplatesDialog::~TemplatesDialog()
//==============================================================================

void TemplatesDialog::setPluginsList(const VSPluginsList & a_pluginsList)
{
	m_ui.snippetEdit->setPluginsList(a_pluginsList);
	m_ui.newScriptTemplateEdit->setPluginsList(a_pluginsList);
}

// END OF void TemplatesDialog::setPluginsList(
//		const VSPluginsList & a_pluginsList)
//==============================================================================

void TemplatesDialog::call()
{
	slotLoadSettings();
	show();
}

// END OF void TemplatesDialog::call()
//==============================================================================

void TemplatesDialog::slotLoadSettings()
{
	m_ui.snippetEdit->slotLoadSettings();
	m_ui.newScriptTemplateEdit->slotLoadSettings();
	QString newScriptTemplate = m_pSettingsManager->getNewScriptTemplate();
	m_ui.newScriptTemplateEdit->setPlainText(newScriptTemplate);
}

// END OF void TemplatesDialog::slotLoadSettings()
//==============================================================================

void TemplatesDialog::slotSnippetPasteIntoScriptButtonClicked()
{
}

// END OF void TemplatesDialog::slotSnippetPasteIntoScriptButtonClicked()
//==============================================================================

void TemplatesDialog::slotSnippetSaveButtonClicked()
{
}

// END OF void TemplatesDialog::slotSnippetSaveButtonClicked()
//==============================================================================

void TemplatesDialog::slotSnippetDeleteButtonClicked()
{
}

// END OF void TemplatesDialog::slotSnippetDeleteButtonClicked()
//==============================================================================

void TemplatesDialog::slotNewScriptTemplateRevertButtonClicked()
{
	QString newScriptTemplate = m_pSettingsManager->getNewScriptTemplate();
	m_ui.newScriptTemplateEdit->setPlainText(newScriptTemplate);
}

// END OF void TemplatesDialog::slotNewScriptTemplateRevertButtonClicked()
//==============================================================================

void TemplatesDialog::slotNewScriptTemplateLoadDefaultButtonClicked()
{
	QString defaultNewScriptTemplate =
		m_pSettingsManager->getDefaultNewScriptTemplate();
	m_ui.newScriptTemplateEdit->setPlainText(defaultNewScriptTemplate);
}

// END OF void TemplatesDialog::slotNewScriptTemplateLoadDefaultButtonClicked()
//==============================================================================

void TemplatesDialog::slotNewScriptTemplateSaveButtonClicked()
{
	QString newScriptTemplate = m_ui.newScriptTemplateEdit->toPlainText();
	m_pSettingsManager->setNewScriptTemplate(newScriptTemplate);
}

// END OF void TemplatesDialog::slotNewScriptTemplateSaveButtonClicked()
//==============================================================================
