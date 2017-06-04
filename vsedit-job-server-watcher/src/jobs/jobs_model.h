#ifndef JOBS_MODEL_H_INCLUDED
#define JOBS_MODEL_H_INCLUDED

#include "../../../common-src/settings/settings_definitions_core.h"
#include "../../../common-src/log/styled_log_view_core.h"

#include <QAbstractItemModel>
#include <QItemSelection>
#include <vector>

class SettingsManager;

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
		QObject * a_pParent = nullptr);
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

	JobProperties jobProperties(int a_index) const;

	int createJob(const JobProperties & a_jobProperties);

	bool swapJobs(const QUuid & a_id1, const QUuid & a_id2);

	bool deleteJobs(std::vector<QUuid> a_ids);

	bool setJobProperties(const QUuid & a_id,
		const JobProperties & a_jobProperties);
	bool setJobDependsOnIds(const QUuid & a_id,
		const std::vector<QUuid> & a_ids);
	bool setJobProgress(const QUuid & a_id, int a_progress, double a_fps);
	bool setJobState(const QUuid & a_id, JobState a_state);

	bool canModifyJob(int a_index) const;

	bool hasActiveJobs();

	std::vector<int> indexesFromSelection(const QItemSelection & a_selection);
	std::vector<QUuid> idsFromSelection(const QItemSelection & a_selection);

signals:

	void signalLogMessage(const QString & a_message,
		const QString & a_style = LOG_STYLE_DEFAULT);

	void signalStateChanged(int a_job, int a_jobsTotal, JobState a_state,
		int a_progress, int a_progressMax);

	void signalSetDependencies(const QUuid & a_id,
		std::vector<QUuid> a_dependencies);

private:

	int indexOfJob(const QUuid & a_id) const;

	void notifyJobUpdated(int a_index, int a_column = -1);

	void notifyState(int a_index);

	std::vector<JobProperties> m_jobs;

	SettingsManager * m_pSettingsManager;

	int m_fpsDisplayPrecision;
};

#endif // JOBS_MODEL_H_INCLUDED
