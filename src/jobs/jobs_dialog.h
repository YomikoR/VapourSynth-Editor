#ifndef JOBS_DIALOG_H_INCLUDED
#define JOBS_DIALOG_H_INCLUDED

#include <ui_jobs_dialog.h>

class SettingsManager;
class HighlightItemDelegate;
class JobsModel;
class JobEditDialog;
class JobStateDelegate;
class JobDependenciesDelegate;

class JobsDialog : public QDialog
{
	Q_OBJECT

public:

	JobsDialog(SettingsManager * a_pSettingsManager, JobsModel * a_pJobsModel,
		QWidget * a_pParent = nullptr);

	virtual ~JobsDialog();

public slots:

	void show();

protected:

	virtual void moveEvent(QMoveEvent * a_pEvent) override;
	virtual void resizeEvent(QResizeEvent * a_pEvent) override;
	virtual void changeEvent(QEvent * a_pEvent) override;

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

	void slotJobDoubleClicked(const QModelIndex & a_index);

	void slotSelectionChanged();

	void slotSaveHeaderState();

private:

	void saveGeometrySettings();

	void editJob(const QModelIndex & a_index);

	bool updateJob(int a_index);

	Ui::JobsDialog m_ui;

	SettingsManager * m_pSettingsManager;

	JobsModel * m_pJobsModel;
	HighlightItemDelegate * m_pHighlightItemDelegate;
	JobStateDelegate * m_pJobStateDelegate;
	JobDependenciesDelegate * m_pJobDependenciesDelegate;

	JobEditDialog * m_pJobEditDialog;
};

#endif // JOBS_DIALOG_H_INCLUDED
