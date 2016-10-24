#ifndef PREVIEW_ADVANCED_SETTINGS_DIALOG_H_INCLUDED
#define PREVIEW_ADVANCED_SETTINGS_DIALOG_H_INCLUDED

#include <ui_preview_advanced_settings_dialog.h>

class SettingsManager;

class PreviewAdvancedSettingsDialog : public QDialog
{
	Q_OBJECT

public:

	PreviewAdvancedSettingsDialog(SettingsManager * a_pSettingsManager,
		QWidget * a_pParent = nullptr);

	virtual ~PreviewAdvancedSettingsDialog();

public slots:

	void slotCall();

signals:

	void signalSettingsChanged();

private slots:

	void slotOk();

	void slotApply();

	void slotResetToDefault();

private:

	Ui::PreviewAdvancedSettingsDialog m_ui;

	SettingsManager * m_pSettingsManager;
};

#endif // PREVIEW_ADVANCED_SETTINGS_DIALOG_H_INCLUDED
