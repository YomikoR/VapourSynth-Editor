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

	JobType jobType() const;
	QString encodingScriptPath() const;
	EncodingHeaderType encodingHeaderType() const;
	QString encodingExecutablePath() const;
	QString encodingArguments() const;
	QString processExecutablePath() const;
	QString processArguments() const;
	QString shellCommand() const;

public slots:

	int call(const QString & a_title, const vsedit::Job * a_pJob = nullptr);

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
