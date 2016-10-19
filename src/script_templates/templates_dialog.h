#ifndef TEMPLATES_DIALOG_H_INCLUDED
#define TEMPLATES_DIALOG_H_INCLUDED

#include <ui_templates_dialog.h>

#include "../settings/settingsmanager.h"

#include <QDialog>

class TemplatesDialog : public QDialog
{
	Q_OBJECT

public:

	TemplatesDialog(SettingsManager * a_pSettingsManager,
		QWidget * a_pParent = nullptr,
		Qt::WindowFlags a_flags =
		(Qt::WindowFlags)0
		| Qt::Window
		| Qt::CustomizeWindowHint
		| Qt::WindowMinimizeButtonHint
		| Qt::WindowMaximizeButtonHint
		| Qt::WindowCloseButtonHint);

	virtual ~TemplatesDialog();

	void setPluginsList(const VSPluginsList & a_pluginsList);

public slots:

	void call();

	void slotLoadSettings();

signals:

	void signalPasteCodeSnippet(const QString & a_text);

private slots:

	void slotSnippetPasteIntoScriptButtonClicked();
	void slotSnippetSaveButtonClicked();
	void slotSnippetDeleteButtonClicked();
	void slotSnippetNameComboBoxActivated(const QString & a_text);

	void slotNewScriptTemplateRevertButtonClicked();
	void slotNewScriptTemplateLoadDefaultButtonClicked();
	void slotNewScriptTemplateSaveButtonClicked();

private:

	Ui::TemplatesDialog m_ui;

	SettingsManager * m_pSettingsManager;

	std::vector<CodeSnippet> m_codeSnippets;
};

#endif // TEMPLATES_DIALOG_H_INCLUDED
