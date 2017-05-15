#ifndef JOBS_DIALOG_H_INCLUDED
#define JOBS_DIALOG_H_INCLUDED

#include <ui_jobs_dialog.h>

#include "../settings/settings_definitions.h"

class SettingsManager;
class JobsModel;
class JobEditDialog;
class JobStateDelegate;
class JobDependenciesDelegate;
class QMenu;
class VSScriptLibrary;

#ifdef Q_OS_WIN
	class QWinTaskbarButton;
	class QWinTaskbarProgress;
#endif

class JobsDialog : public QDialog
{
	Q_OBJECT

public:

	JobsDialog(SettingsManager * a_pSettingsManager, JobsModel * a_pJobsModel,
		VSScriptLibrary * a_pVSScriptLibrary, QWidget * a_pParent = nullptr);

	virtual ~JobsDialog();

public slots:

	void show();

protected:

	virtual void moveEvent(QMoveEvent * a_pEvent) override;
	virtual void resizeEvent(QResizeEvent * a_pEvent) override;
	virtual void changeEvent(QEvent * a_pEvent) override;
	virtual void showEvent(QShowEvent * a_pEvent) override;

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

	void slotJobsHeaderContextMenu(const QPoint & a_point);
	void slotShowJobsHeaderSection(bool a_show);

	void slotJobsStateChanged(int a_job, int a_jobsTotal, JobState a_state,
		int a_progress, int a_progressMax);

private:

	static const char WINDOW_TITLE[];

	void saveGeometrySettings();

	void editJob(const QModelIndex & a_index);

	bool updateJob(int a_index);

	Ui::JobsDialog m_ui;

	SettingsManager * m_pSettingsManager;

	JobsModel * m_pJobsModel;
	VSScriptLibrary * m_pVSScriptLibrary;
	JobStateDelegate * m_pJobStateDelegate;
	JobDependenciesDelegate * m_pJobDependenciesDelegate;

	JobEditDialog * m_pJobEditDialog;

	QMenu * m_pJobsHeaderMenu;

#ifdef Q_OS_WIN
	QWinTaskbarButton * m_pWinTaskbarButton;
	QWinTaskbarProgress * m_pWinTaskbarProgress;
#endif
};

#endif // JOBS_DIALOG_H_INCLUDED
