#ifndef TEMPLATES_DIALOG_H_INCLUDED
#define TEMPLATES_DIALOG_H_INCLUDED

#include <ui_templates_dialog.h>

#include <QDialog>

class SettingsManager;

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

private slots:

	void slotSnippetPasteIntoScriptButtonClicked();
	void slotSnippetSaveButtonClicked();
	void slotSnippetDeleteButtonClicked();
	void slotNewScriptTemplateRevertButtonClicked();
	void slotNewScriptTemplateLoadDefaultButtonClicked();
	void slotNewScriptTemplateSaveButtonClicked();

private:

	Ui::TemplatesDialog m_ui;

	SettingsManager * m_pSettingsManager;
};

#endif // TEMPLATES_DIALOG_H_INCLUDED
