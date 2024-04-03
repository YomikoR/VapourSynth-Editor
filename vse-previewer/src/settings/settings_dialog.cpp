#include "settings_dialog.h"

#include "settings_manager.h"
#include "../helpers.h"

#include "item_delegate_for_hotkey.h"
#include "theme_elements_model.h"

#include <QFileDialog>
#include <QListWidgetItem>
#include <QFontDialog>
#include <QColorDialog>

//==============================================================================

SettingsDialog::SettingsDialog(SettingsManager * a_pSettingsManager,
	QWidget * a_pParent) :
	QDialog(a_pParent,
		  Qt::Dialog
		| Qt::CustomizeWindowHint
		| Qt::WindowTitleHint
		| Qt::WindowCloseButtonHint)
	, m_pSettingsManager(a_pSettingsManager)
	, m_pActionsHotkeyEditModel(nullptr)
	, m_pItemDelegateForHotkey(nullptr)
	, m_pThemeElementsModel(nullptr)
{
	vsedit::disableFontKerning(this);
	m_ui.setupUi(this);
	vsedit::disableFontKerning(m_ui.hotkeysTable);
	vsedit::disableFontKerning(m_ui.themeElementsList);
	setWindowIcon(QIcon(":settings.png"));

	m_ui.addVSLibraryPathButton->setIcon(QIcon(":folder_add.png"));
	m_ui.removeVSLibraryPathButton->setIcon(QIcon(":folder_remove.png"));
	m_ui.selectVSLibraryPathButton->setIcon(QIcon(":folder.png"));

	m_pActionsHotkeyEditModel = new ActionsHotkeyEditModel(m_pSettingsManager,
		this);
	m_ui.hotkeysTable->setModel(m_pActionsHotkeyEditModel);

	m_pItemDelegateForHotkey = new ItemDelegateForHotkey(this);
	m_ui.hotkeysTable->setItemDelegateForColumn(2, m_pItemDelegateForHotkey);

	m_ui.hotkeysTable->resizeColumnsToContents();

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

	connect(m_ui.themeElementsList, SIGNAL(clicked(const QModelIndex &)),
		this, SLOT(slotThemeElementSelected(const QModelIndex &)));
	connect(m_ui.fontButton, SIGNAL(clicked()),
		this, SLOT(slotFontButtonClicked()));
	connect(m_ui.colourButton, SIGNAL(clicked()),
		this, SLOT(slotColourButtonClicked()));
}

// END OF SettingsDialog::SettingsDialog(SettingsManager * a_pSettingsManager,
//		QWidget * a_pParent)
//==============================================================================

SettingsDialog::~SettingsDialog()
{

}

// END OF SettingsDialog::~SettingsDialog()
//==============================================================================

void SettingsDialog::closeEvent(QCloseEvent *a_pEvent)
{
	if(testAttribute(Qt::WA_DeleteOnClose)) // Started by exec()
		reject();
	QDialog::closeEvent(a_pEvent);
}

void SettingsDialog::slotCall(bool a_show)
{
	m_ui.portableModeCheckBox->setChecked(
		m_pSettingsManager->getPortableMode());
	m_ui.debugMsgCheckBox->setChecked(
		m_pSettingsManager->getShowDebugMessages());
	m_ui.snapshotCompressionLevelSpinBox->setValue(
		m_pSettingsManager->getPNGSnapshotCompressionLevel());
	m_ui.preferLibraryFromListCheckBox->setChecked(
		m_pSettingsManager->getPreferVSLibrariesFromList());

	m_ui.vsLibraryPathsListWidget->clear();
	m_ui.vsLibraryPathsListWidget->addItems(
		m_pSettingsManager->getVapourSynthLibraryPaths());
	m_ui.vsLibraryPathEdit->clear();

	m_ui.settingsTabWidget->setCurrentIndex(0);

	m_pActionsHotkeyEditModel->reloadHotkeysSettings();

	m_pThemeElementsModel->reloadThemeSettings();

	QModelIndex firstElement = m_pActionsHotkeyEditModel->index(0, 0);
	m_ui.themeElementsList->setCurrentIndex(firstElement);

	if(a_show)
		show();
}

// END OF void SettingsDialog::slotCall()
//==============================================================================

void SettingsDialog::addThemeElements()
{
	m_pThemeElementsModel->addTextCharFormat(TEXT_FORMAT_ID_COMMON_SCRIPT_TEXT,
		tr("Common script text"));
	m_pThemeElementsModel->addTextCharFormat(TEXT_FORMAT_ID_TIMELINE,
		tr("Timeline labels"));
	m_pThemeElementsModel->addColor(COLOR_ID_TIMELINE_BOOKMARKS,
		tr("Timeline bookmarks color"));
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
	m_pSettingsManager->setPortableMode(
		m_ui.portableModeCheckBox->isChecked());
	m_pSettingsManager->setShowDebugMessages(
		m_ui.debugMsgCheckBox->isChecked());
	m_pSettingsManager->setPNGSnapshotCompressionLevel(
		m_ui.snapshotCompressionLevelSpinBox->value());
	m_pSettingsManager->setPreferVSLibrariesFromList(
		m_ui.preferLibraryFromListCheckBox->isChecked());

	QStringList vapourSynthLibraryPaths;
	int vsLibraryPathsNumber = m_ui.vsLibraryPathsListWidget->count();
	for(int i = 0; i < vsLibraryPathsNumber; ++i)
	{
		QString path = m_ui.vsLibraryPathsListWidget->item(i)->text();
		vapourSynthLibraryPaths.append(path);
	}
	vapourSynthLibraryPaths.removeDuplicates();
	m_pSettingsManager->setVapourSynthLibraryPaths(vapourSynthLibraryPaths);

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
		tr("Select VapourSynth library search path"),
		m_ui.vsLibraryPathEdit->text());
	if(!path.isEmpty())
		m_ui.vsLibraryPathEdit->setText(path);
}

// END OF void SettingsDialog::slotSelectVSLibraryPath()
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

// END OF void SettingsDialog::slotColourButtonClicked()
//==============================================================================
