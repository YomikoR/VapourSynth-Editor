#ifndef JOBS_MODEL_H_INCLUDED
#define JOBS_MODEL_H_INCLUDED

#include "../../../vsedit-job-server/src/jobs/job.h"
#include "../../../vsedit-job-server/src/jobs/job_definitions.h"

#include <QAbstractItemModel>
#include <QItemSelection>
#include <vector>

class SettingsManager;
class VSScriptLibrary;

class JobsModel : public QAbstractItemModel
{
	Q_OBJECT

public:

	static const int NAME_COLUMN;
	static const int TYPE_COLUMN;
	static const int SUBJECT_COLUMN;
	static const int STATE_COLUMN;
	static const int DEPENDS_ON_COLUMN;
	static const int TIME_START_COLUMN;
	static const int TIME_END_COLUMN;
	static const int FPS_COLUMN;
	static const int COLUMNS_NUMBER;

	JobsModel(SettingsManager * a_pSettingsManager,
		VSScriptLibrary * a_pVSScriptLibrary, QObject * a_pParent = nullptr);
	virtual ~JobsModel();

	virtual QModelIndex index(int a_row, int a_column,
		const QModelIndex & a_parent = QModelIndex()) const override;

	virtual QModelIndex parent(const QModelIndex & a_child) const override;

	virtual Qt::ItemFlags flags(const QModelIndex & a_index) const override;

	virtual QVariant headerData(int a_section, Qt::Orientation a_orientation,
		int a_role = Qt::DisplayRole) const override;

	virtual QVariant data(const QModelIndex & a_index,
		int a_role = Qt::DisplayRole) const override;

	virtual int rowCount(const QModelIndex & a_parent = QModelIndex()) const
		override;

	virtual int columnCount(const QModelIndex & a_parent = QModelIndex()) const
		override;

	virtual bool setData(const QModelIndex & a_index, const QVariant & a_value,
		int a_role = Qt::EditRole) override;

	const vsedit::Job * job(int a_index) const;

	int createJob();

	bool moveJobUp(int a_index);
	bool moveJobDown(int a_index);

	bool deleteJob(int a_index);
	bool deleteJob(const vsedit::Job * a_pJob);
	bool deleteJob(const QUuid & a_uuid);

	bool setJobType(int a_index, JobType a_type);
	bool setJobDependsOnIds(int a_index, const std::vector<QUuid> & a_ids);
	bool setJobScriptName(int a_index, const QString & a_scriptName);
	bool setJobEncodingHeaderType(int a_index, EncodingHeaderType a_headerType);
	bool setJobExecutablePath(int a_index, const QString & a_path);
	bool setJobArguments(int a_index, const QString & a_arguments);
	bool setJobShellCommand(int a_index, const QString & a_command);
	bool setJobState(int a_index, JobState a_state);
	bool setJobProperties(int a_index, const JobProperties & a_jobProperties);

	bool canModifyJob(int a_index) const;

	bool loadJobs();
	bool saveJobs();

	bool hasActiveJobs();

	void startWaitingJobs();
	void abortActiveJobs();
	void pauseActiveJobs();
	void resumePausedJobs();
	void resetSelectedJobs();
	void deleteSelectedJobs();

	void setSelection(const QItemSelection & a_selection);

signals:

	void signalLogMessage(const QString & a_message,
		const QString & a_style = LOG_STYLE_DEFAULT);

	void signalStateChanged(int a_job, int a_jobsTotal, JobState a_state,
		int a_progress, int a_progressMax);

private slots:

	void slotLogMessage(const QString & a_message, const QString & a_style);

	void slotJobStateChanged(JobState a_newState, JobState a_oldState);
	void slotJobProgressChanged();

private:

	enum class DependenciesState
	{
		Incomplete,
		Complete,
		Failed,
	};

	int indexOfJob(const QUuid & a_uuid) const;

	void clearJobs();

	bool checkCanModifyJobAndNotify(int a_index);

	void notifyJobUpdated(int a_index, int a_column = -1);

	void notifyState(int a_index);

	DependenciesState dependenciesState(int a_index);

	void connectJob(vsedit::Job * a_pJob);

	void startFirstReadyJob(int a_fromIndex = 0);

	std::vector<int> indexesFromSelection();
	std::vector<QUuid> idsFromSelection();

	std::vector<JobTicket> m_tickets;

	SettingsManager * m_pSettingsManager;
	VSScriptLibrary * m_pVSScriptLibrary;

	QItemSelection m_selection;

	int m_fpsDisplayPrecision;
};

#endif // JOBS_MODEL_H_INCLUDED
