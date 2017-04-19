#ifndef JOBS_MODEL_H_INCLUDED
#define JOBS_MODEL_H_INCLUDED

#include "job.h"

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

	bool canModifyJob(int a_index) const;

	bool loadJobs();
	bool saveJobs();

	bool hasActiveJobs();

	void startWaitingJobs();
	void abortActiveJobs();
	void pauseActiveJobs();
	void resumeJobs();

signals:

	void signalLogMessage(const QString & a_message,
		const QString & a_style = LOG_STYLE_DEFAULT);

	void signalNoActiveJobs();

private slots:

	void slotLogMessage(const QString & a_message, const QString & a_style);

	void slotSelectionChanged(const QItemSelection & a_selected,
		const QItemSelection & a_deselected);

	void slotJobStateChanged(JobState a_newState, JobState a_oldState);
	void slotJobProgressChanged();

private:

	enum class WantTo
	{
		Idle,
		RunOne,
		RunAll,
		AbortOne,
		AbortAll,
		PauseOne,
		PauseAll,
	};

	int indexOfJob(const QUuid & a_uuid) const;

	void clearJobs();

	bool checkCanModifyJobAndNotify(int a_index);

	void notifyJobUpdated(int a_index);

	bool dependenciesMet(int a_index);

	void connectJob(vsedit::Job * a_pJob);

	std::vector<vsedit::Job *> m_jobs;

	SettingsManager * m_pSettingsManager;
	VSScriptLibrary * m_pVSScriptLibrary;

	QItemSelection m_selection;

	WantTo m_wantTo;
};

#endif // JOBS_MODEL_H_INCLUDED
