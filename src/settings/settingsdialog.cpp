#include <QFileDialog>
#include <QListWidgetItem>
#include <QFontDialog>
#include <QColorDialog>

#include "settingsmanager.h"
#include "itemdelegateforhotkey.h"
#include "theme_elements_model.h"

#include "settingsdialog.h"

//==============================================================================

SettingsDialog::SettingsDialog(SettingsManager * a_pSettingsManager,
	QWidget * a_pParent) :
	QDialog(a_pParent, (Qt::WindowFlags)0
		| Qt::Dialog
		| Qt::CustomizeWindowHint
		| Qt::WindowTitleHint
		| Qt::WindowCloseButtonHint)
	, m_pSettingsManager(a_pSettingsManager)
	, m_pActionsHotkeyEditModel(nullptr)
	, m_pItemDelegateForHotkey(nullptr)
	, m_pThemeElementsModel(nullptr)
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

	m_pThemeElementsModel = new ThemeElementsModel(m_pSettingsManager, this);
	m_ui.themeElementsList->setModel(m_pThemeElementsModel);
	addThemeElements();

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

	connect(m_ui.themeElementsList, SIGNAL(clicked(const QModelIndex &)),
		this, SLOT(slotThemeElementSelected(const QModelIndex &)));
	connect(m_ui.fontButton, SIGNAL(clicked()),
		this, SLOT(slotFontButtonClicked()));
	connect(m_ui.colourButton, SIGNAL(clicked()),
		this, SLOT(slotColourButtonClicked()));
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

	m_pThemeElementsModel->reloadThemeSettings();

	QModelIndex firstElement = m_pActionsHotkeyEditModel->index(0, 0);
	m_ui.themeElementsList->setCurrentIndex(firstElement);

	show();
}

// END OF void SettingsDialog::slotCall()
//==============================================================================

void SettingsDialog::addThemeElements()
{
	m_pThemeElementsModel->addTextCharFormat(TEXT_FORMAT_ID_COMMON_SCRIPT_TEXT,
		trUtf8("Common script text"));
	m_pThemeElementsModel->addTextCharFormat(TEXT_FORMAT_ID_KEYWORD,
		trUtf8("Keyword"));
	m_pThemeElementsModel->addTextCharFormat(TEXT_FORMAT_ID_OPERATOR,
		trUtf8("Operator"));
	m_pThemeElementsModel->addTextCharFormat(TEXT_FORMAT_ID_STRING,
		trUtf8("String"));
	m_pThemeElementsModel->addTextCharFormat(TEXT_FORMAT_ID_NUMBER,
		trUtf8("Number"));
	m_pThemeElementsModel->addTextCharFormat(TEXT_FORMAT_ID_COMMENT,
		trUtf8("Comment"));
	m_pThemeElementsModel->addTextCharFormat(TEXT_FORMAT_ID_VS_CORE,
		trUtf8("VapourSynth core"));
	m_pThemeElementsModel->addTextCharFormat(TEXT_FORMAT_ID_VS_NAMESPACE,
		trUtf8("VapourSynth namespace"));
	m_pThemeElementsModel->addTextCharFormat(TEXT_FORMAT_ID_VS_FUNCTION,
		trUtf8("VapourSynth function"));
	m_pThemeElementsModel->addTextCharFormat(TEXT_FORMAT_ID_VS_ARGUMENT,
		trUtf8("VapourSynth argument"));
	m_pThemeElementsModel->addTextCharFormat(TEXT_FORMAT_ID_TIMELINE,
		trUtf8("Timeline labels"));
	m_pThemeElementsModel->addColor(COLOR_ID_TEXT_BACKGROUND,
		trUtf8("Text background color"));
	m_pThemeElementsModel->addColor(COLOR_ID_ACTIVE_LINE,
		trUtf8("Active line color"));
}

// END OF void SettingsDialog::addThemeElements()
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

	m_pThemeElementsModel->slotSaveThemeSettings();

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

void SettingsDialog::slotThemeElementSelected(const QModelIndex & a_index)
{
	if(!a_index.isValid())
		return;

    QString themeElementId = m_pThemeElementsModel->data(
		a_index, Qt::UserRole).toString();
    ThemeElementData themeElementData =
		m_pThemeElementsModel->getThemeElementData(themeElementId);

	if(themeElementData.id.isEmpty())
		return;

	if(themeElementData.type == ThemeElementType::TextCharFormat)
	{
		m_ui.fontButton->setEnabled(true);
		m_ui.colourButton->setEnabled(true);

		m_ui.fontLabel->setText(
			themeElementData.textCharFormat.font().family());
		m_ui.fontLabel->setFont(themeElementData.textCharFormat.font());
		QPalette newPalette = m_ui.fontLabel->palette();
		newPalette.setColor(QPalette::WindowText,
			themeElementData.textCharFormat.foreground().color());
		m_ui.fontLabel->setPalette(newPalette);
		m_ui.fontLabel->update();

		newPalette = m_ui.colourFrame->palette();
		newPalette.setColor(QPalette::Window,
			themeElementData.textCharFormat.foreground().color());
		m_ui.colourFrame->setPalette(newPalette);
		m_ui.colourFrame->update();
	}
	else if(themeElementData.type == ThemeElementType::Color)
	{
		m_ui.fontButton->setEnabled(false);
		m_ui.colourButton->setEnabled(true);

		m_ui.fontLabel->setText(QString());
		m_ui.fontLabel->setFont(QFont());
		m_ui.fontLabel->setPalette(QPalette());
		m_ui.fontLabel->update();

		QPalette newPalette = m_ui.colourFrame->palette();
		newPalette.setColor(QPalette::Window, themeElementData.color);
		m_ui.colourFrame->setPalette(newPalette);
		m_ui.colourFrame->update();
	}
}

// END OF void SettingsDialog::slotThemeElementSelected(
//		const QModelIndex & a_index)
//==============================================================================

void SettingsDialog::slotFontButtonClicked()
{
	QModelIndex index = m_ui.themeElementsList->currentIndex();
	QString id = m_pThemeElementsModel->data(index, Qt::UserRole).toString();
	ThemeElementData themeElementData =
		m_pThemeElementsModel->getThemeElementData(id);

	QFontDialog fontDialog;
	fontDialog.setCurrentFont(themeElementData.textCharFormat.font());
	int returnCode = fontDialog.exec();
	if(returnCode == QDialog::Rejected)
		return;

	QFont newFont = fontDialog.selectedFont();
	themeElementData.textCharFormat.setFont(newFont);
	m_pThemeElementsModel->saveThemeElementData(themeElementData);
	m_ui.fontLabel->setText(newFont.family());
	m_ui.fontLabel->setFont(newFont);
}

// END OF void SettingsDialog::slotFontButtonClicked()
//==============================================================================

void SettingsDialog::slotColourButtonClicked()
{
	QModelIndex index = m_ui.themeElementsList->currentIndex();
	QString id = m_pThemeElementsModel->data(index, Qt::UserRole).toString();
	ThemeElementData themeElementData =
		m_pThemeElementsModel->getThemeElementData(id);

	QColorDialog colorDialog;

	if(themeElementData.type == ThemeElementType::TextCharFormat)
	{
		colorDialog.setCurrentColor(
		themeElementData.textCharFormat.foreground().color());
	}
	else if(themeElementData.type == ThemeElementType::Color)
		colorDialog.setCurrentColor(themeElementData.color);

	int returnCode = colorDialog.exec();
	if(returnCode == QDialog::Rejected)
		return;

	QColor newColor = colorDialog.selectedColor();

	if(themeElementData.type == ThemeElementType::TextCharFormat)
	{
		QBrush brush = themeElementData.textCharFormat.foreground();
		brush.setColor(newColor);
		themeElementData.textCharFormat.setForeground(brush);

		QPalette newPalette = m_ui.fontLabel->palette();
		newPalette.setColor(QPalette::WindowText,
			themeElementData.textCharFormat.foreground().color());
		m_ui.fontLabel->setPalette(newPalette);
		m_ui.fontLabel->update();

		newPalette = m_ui.colourFrame->palette();
		newPalette.setColor(QPalette::Window,
			themeElementData.textCharFormat.foreground().color());
		m_ui.colourFrame->setPalette(newPalette);
		m_ui.colourFrame->update();
	}
	else if(themeElementData.type == ThemeElementType::Color)
	{
		themeElementData.color = newColor;

		QPalette newPalette = m_ui.colourFrame->palette();
		newPalette.setColor(QPalette::Window, themeElementData.color);
		m_ui.colourFrame->setPalette(newPalette);
		m_ui.colourFrame->update();
	}

	m_pThemeElementsModel->saveThemeElementData(themeElementData);
}

// END OF void SettingsDialog::slotFontButtonClicked()
//==============================================================================
