#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <ui_settingsdialog.h>

#include "actionshotkeyeditmodel.h"

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

		void addSettableActions(const ActionDataList & a_actionsList);

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

		void slotAddVSPluginsPath();

		void slotRemoveVSPluginsPath();

		void slotSelectVSPluginsPath();

		void slotAddVSDocumentationPath();

		void slotRemoveVSDocumentationPath();

		void slotSelectVSDocumentationPath();

		void slotThemeElementSelected(const QModelIndex & a_index);

		void slotFontButtonClicked();

		void slotColourButtonClicked();
};

#endif // SETTINGSDIALOG_H
