#include "jobs_model.h"

#include "../settings/settings_manager.h"

#include <cassert>

//==============================================================================

const int NAME_COLUMN = 0;
const int TYPE_COLUMN = 1;
const int SUBJECT_COLUMN = 2;
const int STATE_COLUMN = 3;
const int DEPENDS_ON_COLUMN = 4;
const int COLUMNS_NUMBER = 5;

//==============================================================================

JobsModel::JobsModel(SettingsManager * a_pSettingsManager, QObject * a_pParent):
	  QAbstractItemModel(a_pParent)
	, m_pSettingsManager(a_pSettingsManager)
{
	assert(m_pSettingsManager);
}

// END OF JobsModel::JobsModel(SettingsManager * a_pSettingsManager,
//		QObject * a_pParent)
//==============================================================================

JobsModel::~JobsModel()
{
}

// END OF JobsModel::~JobsModel()
//==============================================================================

QModelIndex JobsModel::index(int a_row, int a_column,
	const QModelIndex & a_parent) const
{
	(void)a_parent;
	return createIndex(a_row, a_column);
}

// END OF QModelIndex JobsModel::index(int a_row, int a_column,
//		const QModelIndex & a_parent) const
//==============================================================================

QModelIndex JobsModel::parent(const QModelIndex & a_child) const
{
	(void)a_child;
	return QModelIndex();
}

// END OF QModelIndex JobsModel::parent(const QModelIndex & a_child) const
//==============================================================================

Qt::ItemFlags JobsModel::flags(const QModelIndex & a_index) const
{
	if (!a_index.isValid())
	{
		return Qt::NoItemFlags;
	}

	Qt::ItemFlags cellFlags = Qt::NoItemFlags
		| Qt::ItemIsEnabled
		| Qt::ItemIsSelectable
	;

	return cellFlags;
}

// END OF Qt::ItemFlags JobsModel::flags(const QModelIndex & a_index) const
//==============================================================================

QVariant JobsModel::data(const QModelIndex & a_index, int a_role) const
{
	if(!a_index.isValid())
		return QVariant();

	int row = a_index.row();
	int column = a_index.column();

	if((a_index.row() >= (int)m_jobs.size()) ||
		(a_index.column() >= COLUMNS_NUMBER))
		return QVariant();

	if((a_role == Qt::DisplayRole) || (a_role == Qt::ToolTipRole))
	{
		if(column == NAME_COLUMN)
			return trUtf8("Job %1").arg(row + 1);
		else if(column == TYPE_COLUMN)
			return vsedit::Job::typeName(m_jobs[row].type());
		else if(column == SUBJECT_COLUMN)
			return m_jobs[row].subject();
		else if(column == STATE_COLUMN)
			return vsedit::Job::stateName(m_jobs[row].state());
		else if(column == DEPENDS_ON_COLUMN)
		{
			QStringList dependsList;
			for(const QUuid & a_uuid : m_jobs[row].dependsOnJobIds())
			{
				ptrdiff_t index = indexOfJob(a_uuid);
				if(index < 0)
					dependsList << trUtf8("<invalid job>");
				else
					dependsList << trUtf8("Job %1").arg(index + 1);
			}
			return dependsList.join(", ");
		}
	}

	return QVariant();
}

// END OF QVariant JobsModel::data(const QModelIndex & a_index, int a_role)
//		const
//==============================================================================

int JobsModel::rowCount(const QModelIndex & a_parent) const
{
	(void)a_parent;
	return (int)m_jobs.size();
}

// END OF int JobsModel::rowCount(const QModelIndex & a_parent) const
//==============================================================================

int JobsModel::columnCount(const QModelIndex & a_parent) const
{
	(void)a_parent;
	return COLUMNS_NUMBER;
}

// END OF int JobsModel::columnCount(const QModelIndex & a_parent) const
//==============================================================================

bool JobsModel::setData(const QModelIndex & a_index, const QVariant & a_value,
	int a_role)
{
	(void)a_index;
	(void)a_value;
	(void)a_role;
	return false;
}

// END OF bool JobsModel::setData(const QModelIndex & a_index,
//		const QVariant & a_value, int a_role)
//==============================================================================

ptrdiff_t JobsModel::indexOfJob(const QUuid & a_uuid) const
{
	std::vector<vsedit::Job>::const_iterator it = std::find_if(m_jobs.cbegin(),
		m_jobs.cend(), [&](const vsedit::Job & la_job)->bool
		{
			return (la_job.id() == a_uuid);
		});

	return (it == m_jobs.cend()) ? -1 : std::distance(m_jobs.cbegin(), it);
}

// END OF ptrdiff_t JobsModel::indexOfJob(const QUuid & a_uuid) const
//==============================================================================
