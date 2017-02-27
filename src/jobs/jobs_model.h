#ifndef JOBS_MODEL_H_INCLUDED
#define JOBS_MODEL_H_INCLUDED

#include "job.h"

#include <QAbstractItemModel>
#include <vector>

class SettingsManager;

class JobsModel : public QAbstractItemModel
{
	Q_OBJECT

public:

	JobsModel(SettingsManager * a_pSettingsManager,
		QObject * a_pParent = nullptr);
	virtual ~JobsModel();

	virtual QModelIndex index(int a_row, int a_column,
		const QModelIndex & a_parent = QModelIndex()) const override;

	virtual QModelIndex parent(const QModelIndex & a_child) const override;

	virtual Qt::ItemFlags flags(const QModelIndex & a_index) const override;

	virtual QVariant data(const QModelIndex & a_index,
		int a_role = Qt::DisplayRole) const override;

	virtual int rowCount(const QModelIndex & a_parent = QModelIndex()) const
		override;

	virtual int columnCount(const QModelIndex & a_parent = QModelIndex()) const
		override;

	virtual bool setData(const QModelIndex & a_index, const QVariant & a_value,
		int a_role = Qt::EditRole) override;

private:

	ptrdiff_t indexOfJob(const QUuid & a_uuid) const;

	std::vector<vsedit::Job> m_jobs;

	SettingsManager * m_pSettingsManager;
};

#endif // JOBS_MODEL_H_INCLUDED
