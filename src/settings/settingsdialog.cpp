#include <QFileDialog>
#include <QListWidgetItem>

#include "settingsmanager.h"
#include "itemdelegateforhotkey.h"

#include "settingsdialog.h"

//==============================================================================

SettingsDialog::SettingsDialog(SettingsManager * a_pSettingsManager,
	QWidget * a_pParent) :
	QDialog(a_pParent, (Qt::WindowFlags)0
		| Qt::Dialog
		| Qt::CustomizeWindowHint
		| Qt::WindowCloseButtonHint)
	, m_pSettingsManager(a_pSettingsManager)
	, m_pActionsHotkeyEditModel(nullptr)
	,m_pItemDelegateForHotkey(nullptr)
{
	m_ui.setupUi(this);
	setWindowIcon(QIcon(":settings.png"));

	m_ui.addVSLibraryPathButton->setIcon(QIcon(":folder_add.png"));
	m_ui.removeVSLibraryPathButton->setIcon(QIcon(":folder_remove.png"));
	m_ui.selectVSLibraryPathButton->setIcon(QIcon(":folder.png"));
	m_ui.addVSPluginsPathButton->setIcon(QIcon(":folder_add.png"));
	m_ui.removeVSPluginsPathButton->setIcon(QIcon(":folder_remove.png"));
	m_ui.selectVSPluginsPathButton->setIcon(QIcon(":folder.png"));
	m_ui.addVSDocumentationPathButton->setIcon(QIcon(":folder_add.png"));
	m_ui.removeVSDocumentationPathButton->setIcon(QIcon(":folder_remove.png"));
	m_ui.selectVSDocumentationPathButton->setIcon(QIcon(":folder.png"));

	m_pActionsHotkeyEditModel = new ActionsHotkeyEditModel(m_pSettingsManager,
		this);
	m_ui.hotkeysTable->setModel(m_pActionsHotkeyEditModel);

	m_pItemDelegateForHotkey = new ItemDelegateForHotkey(this);
	m_ui.hotkeysTable->setItemDelegateForColumn(1, m_pItemDelegateForHotkey);

	connect(m_ui.okButton, SIGNAL(clicked()), this, SLOT(slotOk()));
	connect(m_ui.applyButton, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(m_ui.cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

	connect(m_ui.addVSLibraryPathButton, SIGNAL(clicked()),
		this, SLOT(slotAddVSLibraryPath()));
	connect(m_ui.removeVSLibraryPathButton, SIGNAL(clicked()),
		this, SLOT(slotRemoveVSLibraryPath()));
	connect(m_ui.selectVSLibraryPathButton, SIGNAL(clicked()),
		this, SLOT(slotSelectVSLibraryPath()));

	connect(m_ui.addVSPluginsPathButton, SIGNAL(clicked()),
		this, SLOT(slotAddVSPluginsPath()));
	connect(m_ui.removeVSPluginsPathButton, SIGNAL(clicked()),
		this, SLOT(slotRemoveVSPluginsPath()));
	connect(m_ui.selectVSPluginsPathButton, SIGNAL(clicked()),
		this, SLOT(slotSelectVSPluginsPath()));

	connect(m_ui.addVSDocumentationPathButton, SIGNAL(clicked()),
		this, SLOT(slotAddVSDocumentationPath()));
	connect(m_ui.removeVSDocumentationPathButton, SIGNAL(clicked()),
		this, SLOT(slotRemoveVSDocumentationPath()));
	connect(m_ui.selectVSDocumentationPathButton, SIGNAL(clicked()),
		this, SLOT(slotSelectVSDocumentationPath()));
}

// END OF SettingsDialog::SettingsDialog(SettingsManager * a_pSettingsManager,
//		QWidget * a_pParent, Qt::WindowFlags a_windowFlags)
//==============================================================================

SettingsDialog::~SettingsDialog()
{

}

// END OF SettingsDialog::~SettingsDialog()
//==============================================================================

void SettingsDialog::addSettableActions(const ActionDataList & a_actionsList)
{
	m_pActionsHotkeyEditModel->addActions(a_actionsList);
	m_ui.hotkeysTable->resizeColumnsToContents();
	m_ui.hotkeysTable->resizeRowsToContents();
}
// END OF void SettingsDialog::addSettableActions(
//		const ActionDataList & a_actionsList)
//==============================================================================

void SettingsDialog::slotCall()
{
	m_ui.autoLoadLastScriptCheckBox->setChecked(
		m_pSettingsManager->getAutoLoadLastScript());
	m_ui.promptToSaveChangesCheckBox->setChecked(
		m_pSettingsManager->getPromptToSaveChanges());
	m_ui.portableModeCheckBox->setChecked(
		m_pSettingsManager->getPortableMode());
	m_ui.maxRecentFilesSpinBox->setValue(
		m_pSettingsManager->getMaxRecentFilesNumber());
	m_ui.charactersTypedToStartCompletionSpinBox->setValue(
		m_pSettingsManager->getCharactersTypedToStartCompletion());

	m_ui.vsLibraryPathsListWidget->clear();
	m_ui.vsLibraryPathsListWidget->addItems(
		m_pSettingsManager->getVapourSynthLibraryPaths());
	m_ui.vsLibraryPathEdit->clear();

	m_ui.vsPluginsPathsListWidget->clear();
	m_ui.vsPluginsPathsListWidget->addItems(
		m_pSettingsManager->getVapourSynthPluginsPaths());
	m_ui.vsPluginsPathEdit->clear();

	m_ui.vsDocumentationPathsListWidget->clear();
	m_ui.vsDocumentationPathsListWidget->addItems(
		m_pSettingsManager->getVapourSynthDocumentationPaths());
	m_ui.vsDocumentationPathEdit->clear();

	m_ui.settingsTabWidget->setCurrentIndex(0);

	m_pActionsHotkeyEditModel->reloadHotkeysSettings();
	show();
}

// END OF void SettingsDialog::slotCall()
//==============================================================================

void SettingsDialog::slotOk()
{
	slotApply();
	accept();
}

// END OF void SettingsDialog::slotOk()
//==============================================================================

void SettingsDialog::slotApply()
{
	m_pSettingsManager->setAutoLoadLastScript(
		m_ui.autoLoadLastScriptCheckBox->isChecked());
	m_pSettingsManager->setPromptToSaveChanges(
		m_ui.promptToSaveChangesCheckBox->isChecked());
	m_pSettingsManager->setPortableMode(
		m_ui.portableModeCheckBox->isChecked());
	m_pSettingsManager->setMaxRecentFilesNumber(
		m_ui.maxRecentFilesSpinBox->value());
	m_pSettingsManager->setCharactersTypedToStartCompletion(
		m_ui.charactersTypedToStartCompletionSpinBox->value());

	QStringList vapourSynthLibraryPaths;
	int vsLibraryPathsNumber = m_ui.vsLibraryPathsListWidget->count();
	for(int i = 0; i < vsLibraryPathsNumber; ++i)
	{
		QString path = m_ui.vsLibraryPathsListWidget->item(i)->text();
		vapourSynthLibraryPaths.append(path);
	}
	vapourSynthLibraryPaths.removeDuplicates();
	m_pSettingsManager->setVapourSynthLibraryPaths(vapourSynthLibraryPaths);

	QStringList vapourSynthPluginsPaths;
	int vsPluginsPathsNumber = m_ui.vsPluginsPathsListWidget->count();
	for(int i = 0; i < vsPluginsPathsNumber; ++i)
	{
		QString path = m_ui.vsPluginsPathsListWidget->item(i)->text();
		vapourSynthPluginsPaths.append(path);
	}
	vapourSynthPluginsPaths.removeDuplicates();
	m_pSettingsManager->setVapourSynthPluginsPaths(vapourSynthPluginsPaths);

	QStringList vapourSynthDocumentationPaths;
	int vsDocumentationPathsNumber =
		m_ui.vsDocumentationPathsListWidget->count();
	for(int i = 0; i < vsDocumentationPathsNumber; ++i)
	{
		QString path = m_ui.vsDocumentationPathsListWidget->item(i)->text();
		vapourSynthDocumentationPaths.append(path);
	}
	vapourSynthDocumentationPaths.removeDuplicates();
	m_pSettingsManager->setVapourSynthDocumentationPaths(
		vapourSynthDocumentationPaths);

	m_pActionsHotkeyEditModel->slotSaveActionsHotkeys();

	emit signalSettingsChanged();
}

// END OF void SettingsDialog::slotApply()
//==============================================================================

void SettingsDialog::slotAddVSLibraryPath()
{
	QString newPath = m_ui.vsLibraryPathEdit->text();
	if(newPath.isEmpty())
		return;
	int pathsNumber = m_ui.vsLibraryPathsListWidget->count();
	for(int i = 0; i < pathsNumber; ++i)
	{
		QString path = m_ui.vsLibraryPathsListWidget->item(i)->text();
		if(path == newPath)
			return;
	}
	QListWidgetItem * pListItem = new QListWidgetItem(newPath,
		m_ui.vsLibraryPathsListWidget);
	pListItem->setToolTip(newPath);
}

// END OF void SettingsDialog::slotAddVSLibraryPath()
//==============================================================================

void SettingsDialog::slotRemoveVSLibraryPath()
{
	QListWidgetItem * pCurrentItem =
		m_ui.vsLibraryPathsListWidget->currentItem();
	if(pCurrentItem)
		delete pCurrentItem;
}

// END OF void SettingsDialog::slotRemoveVSPluginsPath()
//==============================================================================

void SettingsDialog::slotSelectVSLibraryPath()
{
	QString path = QFileDialog::getExistingDirectory(this,
		trUtf8("Select VapourSynth library search path"),
		m_ui.vsLibraryPathEdit->text());
	if(!path.isEmpty())
		m_ui.vsLibraryPathEdit->setText(path);
}

// END OF void SettingsDialog::slotSelectVSLibraryPath()
//==============================================================================

void SettingsDialog::slotAddVSPluginsPath()
{
	QString newPath = m_ui.vsPluginsPathEdit->text();
	if(newPath.isEmpty())
		return;
	int pathsNumber = m_ui.vsPluginsPathsListWidget->count();
	for(int i = 0; i < pathsNumber; ++i)
	{
		QString path = m_ui.vsPluginsPathsListWidget->item(i)->text();
		if(path == newPath)
			return;
	}
	QListWidgetItem * pListItem = new QListWidgetItem(newPath,
		m_ui.vsPluginsPathsListWidget);
	pListItem->setToolTip(newPath);
}

// END OF void SettingsDialog::slotAddVSPluginsPath()
//==============================================================================

void SettingsDialog::slotRemoveVSPluginsPath()
{
	QListWidgetItem * pCurrentItem =
		m_ui.vsPluginsPathsListWidget->currentItem();
	if(pCurrentItem)
		delete pCurrentItem;
}

// END OF void SettingsDialog::slotRemoveVSPluginsPath()
//==============================================================================

void SettingsDialog::slotSelectVSPluginsPath()
{
	QString path = QFileDialog::getExistingDirectory(this,
		trUtf8("Select VapourSynth plugins path"),
		m_ui.vsPluginsPathEdit->text());
	if(!path.isEmpty())
		m_ui.vsPluginsPathEdit->setText(path);
}

// END OF void SettingsDialog::slotSelectVSPluginsPath()
//==============================================================================

void SettingsDialog::slotAddVSDocumentationPath()
{
	QString newPath = m_ui.vsDocumentationPathEdit->text();
	if(newPath.isEmpty())
		return;
	int pathsNumber = m_ui.vsDocumentationPathsListWidget->count();
	for(int i = 0; i < pathsNumber; ++i)
	{
		QString path = m_ui.vsDocumentationPathsListWidget->item(i)->text();
		if(path == newPath)
			return;
	}
	QListWidgetItem * pListItem = new QListWidgetItem(newPath,
		m_ui.vsDocumentationPathsListWidget);
	pListItem->setToolTip(newPath);
}

// END OF void SettingsDialog::slotAddVSDocumentationPath()
//==============================================================================

void SettingsDialog::slotRemoveVSDocumentationPath()
{
	QListWidgetItem * pCurrentItem =
		m_ui.vsDocumentationPathsListWidget->currentItem();
	if(pCurrentItem)
		delete pCurrentItem;
}

// END OF void SettingsDialog::slotRemoveVSDocumentationPath()
//==============================================================================

void SettingsDialog::slotSelectVSDocumentationPath()
{
	QString path = QFileDialog::getExistingDirectory(this,
		trUtf8("Select documentation path"),
		m_ui.vsDocumentationPathEdit->text());
	if(!path.isEmpty())
		m_ui.vsDocumentationPathEdit->setText(path);
}

// END OF void SettingsDialog::slotSelectVSDocumentationPath()
//==============================================================================
