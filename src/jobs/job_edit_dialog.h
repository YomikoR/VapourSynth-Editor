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

public slots:

	int call(vsedit::Job * a_pJob = nullptr);

private slots:

private:

	Ui::JobEditDialog m_ui;

	SettingsManager * m_pSettingsManager;

	vsedit::Job * m_pJob;
};

#endif // JOB_EDIT_DIALOG_H_INCLUDED
