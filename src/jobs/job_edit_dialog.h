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

	vsedit::Job job() const;

public slots:

	int call(const vsedit::Job * a_cpJob = nullptr);

private slots:

private:

	Ui::JobEditDialog m_ui;

	SettingsManager * m_pSettingsManager;

	vsedit::Job m_job;
};

#endif // JOB_EDIT_DIALOG_H_INCLUDED
