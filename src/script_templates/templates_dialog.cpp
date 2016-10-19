#include "templates_dialog.h"

#include <QMessageBox>
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
	connect(m_ui.snippetNameComboBox, SIGNAL(activated(const QString &)),
		this, SLOT(slotSnippetNameComboBoxActivated(const QString &)));
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

	m_ui.snippetNameComboBox->clear();
	m_codeSnippets = m_pSettingsManager->getAllCodeSnippets();
	for(const CodeSnippet & snippet : m_codeSnippets)
		m_ui.snippetNameComboBox->addItem(snippet.name);
	m_ui.snippetNameComboBox->setCurrentIndex(0);
	slotSnippetNameComboBoxActivated(m_ui.snippetNameComboBox->currentText());

	QString newScriptTemplate = m_pSettingsManager->getNewScriptTemplate();
	m_ui.newScriptTemplateEdit->setPlainText(newScriptTemplate);
}

// END OF void TemplatesDialog::slotLoadSettings()
//==============================================================================

void TemplatesDialog::slotSnippetPasteIntoScriptButtonClicked()
{
	QString text = m_ui.snippetEdit->text();
	if(!text.isEmpty())
		emit signalPasteCodeSnippet(text);
}

// END OF void TemplatesDialog::slotSnippetPasteIntoScriptButtonClicked()
//==============================================================================

void TemplatesDialog::slotSnippetSaveButtonClicked()
{
	CodeSnippet snippet(m_ui.snippetNameComboBox->currentText(),
		m_ui.snippetEdit->text());

	bool success = m_pSettingsManager->saveCodeSnippet(snippet);
	if(!success)
		return;

	std::vector<CodeSnippet>::iterator it = std::find(
		m_codeSnippets.begin(), m_codeSnippets.end(), snippet);
	if(it == m_codeSnippets.end())
	{
		assert(m_ui.snippetNameComboBox->findText(snippet.name) == -1);
		m_codeSnippets.push_back(snippet);
		m_ui.snippetNameComboBox->addItem(snippet.name);
		m_ui.snippetNameComboBox->model()->sort(0);
	}
	else
	{
		assert(m_ui.snippetNameComboBox->findText(snippet.name) != -1);
		*it = snippet;
	}
}

// END OF void TemplatesDialog::slotSnippetSaveButtonClicked()
//==============================================================================

void TemplatesDialog::slotSnippetDeleteButtonClicked()
{
	CodeSnippet snippet(m_ui.snippetNameComboBox->currentText());
	if(snippet.name.isEmpty())
		return;

	QMessageBox::StandardButton result = QMessageBox::question(this,
		trUtf8("Delete snippet"), trUtf8("Do you really want to delete "
		"snippet \'%1\'?").arg(snippet.name),
		QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No),
		QMessageBox::No);
	if(result == QMessageBox::No)
		return;

	std::vector<CodeSnippet>::iterator it = std::find(
		m_codeSnippets.begin(), m_codeSnippets.end(), snippet);
	if(it == m_codeSnippets.end())
	{
		assert(m_ui.snippetNameComboBox->findText(snippet.name) == -1);
		return;
	}

	int index = m_ui.snippetNameComboBox->findText(snippet.name);
	assert(index != -1);
	m_ui.snippetNameComboBox->removeItem(index);
	m_codeSnippets.erase(it);
	m_ui.snippetNameComboBox->setCurrentIndex(0);
	slotSnippetNameComboBoxActivated(m_ui.snippetNameComboBox->currentText());

	m_pSettingsManager->deleteCodeSnippet(snippet.name);
}

// END OF void TemplatesDialog::slotSnippetDeleteButtonClicked()
//==============================================================================

void TemplatesDialog::slotSnippetNameComboBoxActivated(const QString & a_text)
{
	if(a_text.isEmpty())
	{
		m_ui.snippetEdit->clear();
		return;
	}

	CodeSnippet snippet(a_text);

	std::vector<CodeSnippet>::iterator it = std::find(
		m_codeSnippets.begin(), m_codeSnippets.end(), snippet);
	if(it == m_codeSnippets.end())
		return;

	snippet = *it;

	m_ui.snippetEdit->setPlainText(snippet.text);
}

// END OF void TemplatesDialog::slotSnippetNameComboBoxActivated(
//		const QString & a_text)
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
