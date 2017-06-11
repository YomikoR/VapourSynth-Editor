#ifndef JOBS_MANAGER_H_INCLUDED
#define JOBS_MANAGER_H_INCLUDED

#include "job.h"
#include "job_definitions.h"
#include "../../../common-src/log/vs_editor_log_definitions.h"

#include <QObject>
#include <vector>

class SettingsManagerCore;
class VSScriptLibrary;

class JobsManager : public QObject
{
	Q_OBJECT

public:

	JobsManager(SettingsManagerCore * a_pSettingsManager,
		QObject * a_pParent = nullptr);
	virtual ~JobsManager();

	std::vector<JobProperties> jobsProperties() const;

	int createJob(const JobProperties & a_jobProperties = JobProperties());

	bool swapJobs(const QUuid & a_jobID1, const QUuid & a_jobID2);

	bool setJobState(const QUuid & a_uuid, JobState a_state);
	bool changeJob(const JobProperties & a_jobProperties);

	bool loadJobs();
	bool saveJobs();

	bool hasActiveJobs();

	void startWaitingJobs();
	void abortActiveJobs();
	void pauseActiveJobs();
	void resumePausedJobs();
	void resetJobs(const std::vector<QUuid> & a_ids);
	void deleteJobs(const std::vector<QUuid> & a_ids);

signals:

	void signalLogMessage(const QString & a_message,
		const QString & a_style = LOG_STYLE_DEFAULT);
	void signalJobCreated(const JobProperties & a_properties);
	void signalJobChanged(const JobProperties & a_properties);
	void signalJobStateChanged(const QUuid & a_jobID, JobState a_state);
	void signalJobProgressChanged(const QUuid & a_jobID, int a_progress,
		double a_fps);
	void signalJobsSwapped(const QUuid & a_jobID1, const QUuid & a_jobID2);
	void signalJobsDeleted(const std::vector<QUuid> & a_ids);

private slots:

	void slotLogMessage(const QString & a_message,
		const QString & a_style = LOG_STYLE_DEFAULT);
	void slotLogMessage(int a_type, const QString & a_message);

	void slotJobPropertiesChanged();
	void slotJobStateChanged(JobState a_newState, JobState a_oldState);
	void slotJobProgressChanged();

private:

	enum class DependenciesState
	{
		Incomplete,
		Complete,
		Failed,
	};

	bool canModifyJob(int a_index) const;

	int indexOfJob(const QUuid & a_uuid) const;

	void clearJobs();

	bool checkCanModifyJobAndNotify(int a_index);

	DependenciesState dependenciesState(int a_index);

	void connectJob(vsedit::Job * a_pJob);

	void startFirstReadyJob(int a_fromIndex = 0);

	std::vector<JobTicket> m_tickets;

	SettingsManagerCore * m_pSettingsManager;
	VSScriptLibrary * m_pVSScriptLibrary;
};

#endif // JOBS_MANAGER_H_INCLUDED
