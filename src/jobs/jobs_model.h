#ifndef JOBS_MODEL_H_INCLUDED
#define JOBS_MODEL_H_INCLUDED

#include "job.h"

#include <QAbstractItemModel>
#include <vector>

class SettingsManager;
class VSScriptLibrary;

class JobsModel : public QAbstractItemModel
{
	Q_OBJECT

public:

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

	vsedit::Job * createJob();

	bool deleteJob(int a_index);
	bool deleteJob(const vsedit::Job * a_pJob);
	bool deleteJob(const QUuid & a_uuid);

private:

	ptrdiff_t indexOfJob(const QUuid & a_uuid) const;

	void clearJobs();

	std::vector<vsedit::Job *> m_jobs;

	SettingsManager * m_pSettingsManager;
	VSScriptLibrary * m_pVSScriptLibrary;
};

#endif // JOBS_MODEL_H_INCLUDED
