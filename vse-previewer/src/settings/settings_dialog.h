#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <ui_settings_dialog.h>

#include "actions_hotkey_edit_model.h"

class SettingsManager;
class ItemDelegateForHotkey;
class ThemeElementsModel;

class SettingsDialog : public QDialog
{
	Q_OBJECT

public:

	SettingsDialog(SettingsManager * a_pSettingsManager,
		QWidget * a_pParent = nullptr);

	virtual ~SettingsDialog();

public slots:

	void slotCall();

protected:

signals:

	void signalSettingsChanged();

private:

	void addThemeElements();

	Ui::SettingsDialog m_ui;

	SettingsManager * m_pSettingsManager;

	ActionsHotkeyEditModel * m_pActionsHotkeyEditModel;

	ItemDelegateForHotkey * m_pItemDelegateForHotkey;

	ThemeElementsModel * m_pThemeElementsModel;

private slots:

	void slotOk();

	void slotApply();

	void slotAddVSLibraryPath();

	void slotRemoveVSLibraryPath();

	void slotSelectVSLibraryPath();

	void slotThemeElementSelected(const QModelIndex & a_index);

	void slotFontButtonClicked();

	void slotColourButtonClicked();
};

#endif // SETTINGSDIALOG_H
