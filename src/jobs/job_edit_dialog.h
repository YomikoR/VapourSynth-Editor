#ifndef JOB_EDIT_DIALOG_H_INCLUDED
#define JOB_EDIT_DIALOG_H_INCLUDED

#include <ui_job_edit_dialog.h>

#include "job.h"

class SettingsManager;

class JobEditDialog : public QDialog
{
	Q_OBJECT

public:

	JobEditDialog(SettingsManager * a_pSettingsManager,
		QWidget * a_pParent = nullptr);

	virtual ~JobEditDialog();

	JobProperties jobProperties() const;

public slots:

	int call(const QString & a_title, const JobProperties & a_jobProperties);

private slots:

	void slotJobTypeChanged(int a_index);
	void slotEncodingScriptBrowseButtonClicked();
	void slotEncodingPresetComboBoxActivated(const QString & a_text);
	void slotEncodingPresetSaveButtonClicked();
	void slotEncodingPresetDeleteButton();
	void slotEncodingExecutableBrowseButtonClicked();
	void slotEncodingArgumentsHelpButtonClicked();
	void slotProcessExecutableBrowseButtonClicked();

private:

	void setUpEncodingPresets();

	QString chooseExecutable(const QString & a_dialogTitle,
		const QString & a_initialPath = QString());

	Ui::JobEditDialog m_ui;

	SettingsManager * m_pSettingsManager;

	std::vector<EncodingPreset> m_encodingPresets;
};

#endif // JOB_EDIT_DIALOG_H_INCLUDED
