#ifndef JOBS_DIALOG_H_INCLUDED
#define JOBS_DIALOG_H_INCLUDED

#include <ui_jobs_dialog.h>

class SettingsManager;
class JobsModel;
class JobEditDialog;

class JobsDialog : public QDialog
{
	Q_OBJECT

public:

	JobsDialog(SettingsManager * a_pSettingsManager, JobsModel * a_pJobsModel,
		QWidget * a_pParent = nullptr);

	virtual ~JobsDialog();

public slots:

private slots:

	void slotJobNewButtonClicked();
	void slotJobEditButtonClicked();
	void slotJobMoveUpButtonClicked();
	void slotJobMoveDownButtonClicked();
	void slotJobDeleteButtonClicked();
	void slotJobResetStateButtonClicked();
	void slotStartButtonClicked();
	void slotPauseButtonClicked();
	void slotResumeButtonClicked();
	void slotAbortButtonClicked();

private:

	bool updateJob(int a_index);

	Ui::JobsDialog m_ui;

	SettingsManager * m_pSettingsManager;

	JobsModel * m_pJobsModel;

	JobEditDialog * m_pJobEditDialog;
};

#endif // JOBS_DIALOG_H_INCLUDED
