#ifndef JOBS_DIALOG_H_INCLUDED
#define JOBS_DIALOG_H_INCLUDED

#include <ui_jobs_dialog.h>

class SettingsManager;

class JobsDialog : public QDialog
{
	Q_OBJECT

public:

	JobsDialog(SettingsManager * a_pSettingsManager,
		QWidget * a_pParent = nullptr);

	virtual ~JobsDialog();

public slots:

private:

	Ui::JobsDialog m_ui;

	SettingsManager * m_pSettingsManager;

private slots:


};

#endif // JOBS_DIALOG_H_INCLUDED
