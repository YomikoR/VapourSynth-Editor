#include "templates_dialog.h"

#include "drop_file_category_model.h"
#include "../../../common-src/settings/settings_manager.h"

#include <QAction>
#include <QMessageBox>

//==============================================================================

TemplatesDialog::TemplatesDialog(SettingsManager * a_pSettingsManager,
	QWidget * a_pParent, Qt::WindowFlags a_flags) :
	  QDialog(a_pParent, a_flags)
	, m_pSettingsManager(a_pSettingsManager)
	, m_pDropFileCategoryModel(nullptr)
	, m_pSaveAction(nullptr)
{
	Q_ASSERT(m_pSettingsManager);
	m_ui.setupUi(this);

	m_scriptEditors = {m_ui.snippetEdit, m_ui.newScriptTemplateEdit,
		m_ui.dropFileCategoryTemplateEdit};

	for(ScriptEditor * pEdit : m_scriptEditors)
		pEdit->setSettingsManager(m_pSettingsManager);

	m_pDropFileCategoryModel = new DropFileCategoryModel(this);
	m_ui.dropFileCategoryView->setModel(m_pDropFileCategoryModel);

	QString fileDropTemplatesInfo = tr("Write file name mask list "
		"as a list of wildcards, separated by semicolons without spaces.\n"
		"In the template below use tokens: ");
	QStringList tokenInfoList;
	std::vector<vsedit::VariableToken> variables =
		m_ui.dropFileCategoryTemplateEdit->variables();
	for(const vsedit::VariableToken & variable : variables)
		tokenInfoList += QString("%1 - %2").arg(variable.token)
			.arg(variable.description);
	fileDropTemplatesInfo += tokenInfoList.join("; ");
	fileDropTemplatesInfo += tr(".");
	m_ui.fileDropTemplatesInfoLabel->setText(fileDropTemplatesInfo);

	m_pSaveAction = m_pSettingsManager->createStandardAction(
		ACTION_ID_SAVE_SCRIPT, this);
	addAction(m_pSaveAction);

	connect(m_pSaveAction, SIGNAL(triggered()),
		this, SLOT(slotSaveActionTriggered()));
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
	connect(m_ui.saveDropFileCategoriesButton, SIGNAL(clicked()),
		this, SLOT(slotSaveDropFileCategoriesButtonClicked()));
	connect(m_ui.revertDropFileCategoriesButton, SIGNAL(clicked()),
		this, SLOT(slotRevertDropFileCategoriesButtonClicked()));
	connect(m_ui.addDropFileCategoryButton, SIGNAL(clicked()),
		this, SLOT(slotAddDropFileCategoryButtonClicked()));
	connect(m_ui.deleteSelectedDropFileCategoryButton, SIGNAL(clicked()),
		this, SLOT(slotDeleteSelectedDropFileCategoryButtonClicked()));
	connect(m_ui.dropFileCategoryView, SIGNAL(pressed(const QModelIndex &)),
		this, SLOT(slotDropFileCategoryViewPressed(const QModelIndex &)));
	connect(m_ui.dropFileCategoryTemplateEdit, SIGNAL(textChanged()),
		this, SLOT(slotUpdateDropFileCategories()));

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
	for(ScriptEditor * pEdit : m_scriptEditors)
		pEdit->setPluginsList(a_pluginsList);
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
	for(ScriptEditor * pEdit : m_scriptEditors)
		pEdit->slotLoadSettings();

	m_pSaveAction->setShortcut(m_pSettingsManager->getHotkey(
		m_pSaveAction->data().toString()));

	m_ui.snippetNameComboBox->clear();
	m_codeSnippets = m_pSettingsManager->getAllCodeSnippets();
	for(const CodeSnippet & snippet : m_codeSnippets)
		m_ui.snippetNameComboBox->addItem(snippet.name);
	m_ui.snippetNameComboBox->setCurrentIndex(0);
	slotSnippetNameComboBoxActivated(m_ui.snippetNameComboBox->currentText());

	QString newScriptTemplate = m_pSettingsManager->getNewScriptTemplate();
	m_ui.newScriptTemplateEdit->setPlainText(newScriptTemplate);

	slotRevertDropFileCategoriesButtonClicked();
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
		Q_ASSERT(m_ui.snippetNameComboBox->findText(snippet.name) == -1);
		m_codeSnippets.push_back(snippet);
		m_ui.snippetNameComboBox->addItem(snippet.name);
		m_ui.snippetNameComboBox->model()->sort(0);
	}
	else
	{
		Q_ASSERT(m_ui.snippetNameComboBox->findText(snippet.name) != -1);
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
		tr("Delete snippet"), tr("Do you really want to delete "
		"snippet \'%1\'?").arg(snippet.name),
		QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No),
		QMessageBox::No);
	if(result == QMessageBox::No)
		return;

	std::vector<CodeSnippet>::iterator it = std::find(
		m_codeSnippets.begin(), m_codeSnippets.end(), snippet);
	if(it == m_codeSnippets.end())
	{
		Q_ASSERT(m_ui.snippetNameComboBox->findText(snippet.name) == -1);
		return;
	}

	int index = m_ui.snippetNameComboBox->findText(snippet.name);
	Q_ASSERT(index != -1);
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

void TemplatesDialog::slotSaveDropFileCategoriesButtonClicked()
{
	int index = m_ui.dropFileCategoryView->currentIndex().row();
	m_pDropFileCategoryModel->setSourceTemplate(index,
		m_ui.dropFileCategoryTemplateEdit->text());
	std::vector<DropFileCategory> categories =
		m_pDropFileCategoryModel->getCategories();
	m_pSettingsManager->setDropFileTemplates(categories);
}

// END OF void TemplatesDialog::slotSaveDropFileCategoriesButtonClicked()
//==============================================================================

void TemplatesDialog::slotRevertDropFileCategoriesButtonClicked()
{
	std::vector<DropFileCategory> categories =
		m_pSettingsManager->getAllDropFileTemplates();
	m_pDropFileCategoryModel->setCategories(categories);
	m_ui.dropFileCategoryView->resizeColumnToContents(0);
	m_ui.dropFileCategoryView->setCurrentIndex(
		m_pDropFileCategoryModel->index(0, 0));
	slotDisplayCurrentDropFileCategoryTemplate();
}

// END OF void TemplatesDialog::slotRevertDropFileCategoriesButtonClicked()
//==============================================================================

void TemplatesDialog::slotAddDropFileCategoryButtonClicked()
{
	m_pDropFileCategoryModel->addCategory();
	QModelIndex index = m_pDropFileCategoryModel->index(
		m_pDropFileCategoryModel->rowCount() - 1, 0);
	m_ui.dropFileCategoryView->setCurrentIndex(index);
	slotDisplayCurrentDropFileCategoryTemplate();
	m_ui.dropFileCategoryView->edit(index);
}

// END OF void TemplatesDialog::slotAddDropFileCategoryButtonClicked()
//==============================================================================

void TemplatesDialog::slotDeleteSelectedDropFileCategoryButtonClicked()
{
	QModelIndex index = m_ui.dropFileCategoryView->currentIndex();
	if(!index.isValid())
		return;

	int row = index.row();
	DropFileCategory category = m_pDropFileCategoryModel->getCategories()[row];

	QMessageBox::StandardButton result = QMessageBox::question(this,
		tr("Delete category"), tr("Do you really want to delete "
		"category \'%1\'?").arg(category.name),
		QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No),
		QMessageBox::No);
	if(result == QMessageBox::No)
		return;

	m_pDropFileCategoryModel->deleteCategory(row);
	slotDisplayCurrentDropFileCategoryTemplate();
}

// END OF void TemplatesDialog::
//		slotDeleteSelectedDropFileCategoryButtonClicked()
//==============================================================================

void TemplatesDialog::slotDropFileCategoryViewPressed(
	const QModelIndex & a_index)
{
	(void)a_index;
	slotDisplayCurrentDropFileCategoryTemplate();
}

// END OF void TemplatesDialog::slotDropFileCategoryViewPressed(
//		const QModelIndex & a_index)
//==============================================================================

void TemplatesDialog::slotDisplayCurrentDropFileCategoryTemplate()
{
	QModelIndex index = m_ui.dropFileCategoryView->currentIndex();
	if(!index.isValid())
		return;
	QString sourceTemplate =
		m_pDropFileCategoryModel->sourceTemplate(index.row());
	m_ui.dropFileCategoryTemplateEdit->setPlainText(sourceTemplate);
}

// END OF void TemplatesDialog::slotDisplayCurrentDropFileCategoryTemplate()
//==============================================================================

void TemplatesDialog::slotUpdateDropFileCategories()
{
	QModelIndex index = m_ui.dropFileCategoryView->currentIndex();
	if(!index.isValid())
		return;
	QString sourceTemplate = m_ui.dropFileCategoryTemplateEdit->text();
	m_pDropFileCategoryModel->setSourceTemplate(index.row(), sourceTemplate);
}

// END OF void TemplatesDialog::slotUpdateDropFileCategories()
//==============================================================================

void TemplatesDialog::slotSaveActionTriggered()
{
	QWidget * pCurrentWidget = m_ui.templatesTabWidget->currentWidget();
	if(pCurrentWidget == m_ui.codeSnippetsTab)
		slotSnippetSaveButtonClicked();
	else if(pCurrentWidget == m_ui.newScriptTemplateTab)
		slotNewScriptTemplateSaveButtonClicked();
	else if(pCurrentWidget == m_ui.fileDropTemplatesTab)
		slotSaveDropFileCategoriesButtonClicked();
}

// END OF void TemplatesDialog::slotSaveActionTriggered()
//==============================================================================
