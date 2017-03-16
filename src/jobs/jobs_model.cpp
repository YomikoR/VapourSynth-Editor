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

JobsModel::JobsModel(SettingsManager * a_pSettingsManager,
	VSScriptLibrary * a_pVSScriptLibrary, QObject * a_pParent):
	  QAbstractItemModel(a_pParent)
	, m_pSettingsManager(a_pSettingsManager)
	, m_pVSScriptLibrary(a_pVSScriptLibrary)
{
	assert(m_pSettingsManager);
}

// END OF JobsModel::JobsModel(SettingsManager * a_pSettingsManager,
//		QObject * a_pParent)
//==============================================================================

JobsModel::~JobsModel()
{
	clearJobs();
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

QVariant JobsModel::headerData(int a_section, Qt::Orientation a_orientation,
	int a_role) const
{
	if(a_orientation != Qt::Horizontal)
		return QVariant();

	if((a_role != Qt::DisplayRole) && (a_role != Qt::ToolTipRole))
		return QVariant();

	switch(a_section)
	{
	case NAME_COLUMN:
		return trUtf8("Name");
	case TYPE_COLUMN:
		return trUtf8("Type");
	case SUBJECT_COLUMN:
		return trUtf8("Subject");
	case STATE_COLUMN:
		return trUtf8("State");
	case DEPENDS_ON_COLUMN:
		return trUtf8("Depends on jobs");
	default:
		return QVariant();
	}

	return QVariant();
}

// END OF QVariant JobsModel::headerData(int a_section,
//		Qt::Orientation a_orientation, int a_role) const
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
			return vsedit::Job::typeName(m_jobs[row]->type());
		else if(column == SUBJECT_COLUMN)
			return m_jobs[row]->subject();
		else if(column == STATE_COLUMN)
			return vsedit::Job::stateName(m_jobs[row]->state());
		else if(column == DEPENDS_ON_COLUMN)
		{
			QStringList dependsList;
			for(const QUuid & a_uuid : m_jobs[row]->dependsOnJobIds())
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

vsedit::Job * JobsModel::createJob()
{
	vsedit::Job * pJob = new vsedit::Job(m_pSettingsManager,
		m_pVSScriptLibrary, this);
	int newRow = (int)m_jobs.size();
	beginInsertRows(QModelIndex(), newRow, newRow);
	m_jobs.push_back(pJob);
	endInsertRows();
	return pJob;
}

// END OF vsedit::Job * JobsModel::createJob()
//==============================================================================

bool JobsModel::deleteJob(int a_index)
{
	if((a_index < 0) || (a_index >= m_jobs.size()))
		return false;

	JobState protectedStates[] = {JobState::Running, JobState::Paused,
		JobState::Aborting};

	vsedit::Job * pJob = m_jobs[a_index];
	if(vsedit::contains(protectedStates, pJob->state()))
		return false;

	beginRemoveRows(QModelIndex(), a_index, a_index);
	delete pJob;
	m_jobs.erase(m_jobs.begin() + a_index);
	endRemoveRows();

	return true;
}

// END OF bool JobsModel::deleteJob(int a_index)
//==============================================================================

bool JobsModel::deleteJob(const vsedit::Job * a_pJob)
{
	std::vector<vsedit::Job *>::const_iterator it =
		std::find(m_jobs.begin(), m_jobs.end(), a_pJob);
	if(it == m_jobs.cend())
		return false;
	else
		return deleteJob(std::distance(m_jobs.cbegin(), it));
}

// END OF bool JobsModel::deleteJob(const vsedit::Job * a_pJob)
//==============================================================================

bool JobsModel::deleteJob(const QUuid & a_uuid)
{
	ptrdiff_t index = indexOfJob(a_uuid);
	if(index < 0)
		return false;
	else
		return deleteJob((int)index);
}

// END OF bool JobsModel::deleteJob(const QUuid & a_uuid)
//==============================================================================

ptrdiff_t JobsModel::indexOfJob(const QUuid & a_uuid) const
{
	std::vector<vsedit::Job *>::const_iterator it =
		std::find_if(m_jobs.cbegin(), m_jobs.cend(),
			[&](const vsedit::Job * la_pJob)->bool
			{
				return (la_pJob->id() == a_uuid);
			});

	return (it == m_jobs.cend()) ? -1 : std::distance(m_jobs.cbegin(), it);
}

// END OF ptrdiff_t JobsModel::indexOfJob(const QUuid & a_uuid) const
//==============================================================================

void JobsModel::clearJobs()
{
	for(vsedit::Job * pJob : m_jobs)
		delete pJob;
	m_jobs.clear();
}

// END OF void JobsModel::clearJobs()
//==============================================================================

