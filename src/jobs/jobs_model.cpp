#include "jobs_model.h"

#include "../settings/settings_manager.h"

#include <QGuiApplication>
#include <cassert>

//==============================================================================

const int JobsModel::NAME_COLUMN = 0;
const int JobsModel::TYPE_COLUMN = 1;
const int JobsModel::SUBJECT_COLUMN = 2;
const int JobsModel::STATE_COLUMN = 3;
const int JobsModel::DEPENDS_ON_COLUMN = 4;
const int JobsModel::COLUMNS_NUMBER = 5;

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
		return Qt::NoItemFlags;

	int row = a_index.row();
	int column = a_index.column();

	if((row >= (int)m_jobs.size()) || (column >= COLUMNS_NUMBER))
		return Qt::NoItemFlags;

	Qt::ItemFlags cellFlags = Qt::NoItemFlags
		| Qt::ItemIsEnabled
		| Qt::ItemIsSelectable
	;

	bool modifiable = canModifyJob(row);
	if((a_index.column() == DEPENDS_ON_COLUMN) && modifiable && (row > 0))
		cellFlags |= Qt::ItemIsEditable;

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
	else if(a_role == Qt::BackgroundRole)
	{
		switch(m_jobs[row]->state())
		{
		case JobState::Aborted:
		case JobState::Failed:
		case JobState::DependencyNotMet:
			return QColor("#f9435e");
			break;
		case JobState::Aborting:
			return QColor("#ff9ba5");
		case JobState::Paused:
			return QColor("#f9ed81");
			break;
		case JobState::Completed:
			return QColor("#8fff87");
			break;
		case JobState::Running:
			return QColor("#87cdff");
			break;
		default:
			return QGuiApplication::palette().color(QPalette::Base);
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
	(void)a_role;

	if(!a_index.isValid())
		return false;

	int row = a_index.row();
	int column = a_index.column();

	if(!checkCanModifyJobAndNotify(row))
		return false;

	if((row >= (int)m_jobs.size()) || (column != DEPENDS_ON_COLUMN))
		return false;

	if(!a_value.canConvert<QVariantList>())
		return false;

	QVariantList variantList = a_value.toList();
	std::vector<QUuid> ids;
	for(const QVariant & variant : variantList)
	{
		if(!variant.canConvert<QUuid>())
			return false;
		QUuid id = variant.toUuid();
		int index = indexOfJob(id);
		if((index < 0) || (index > row))
			return false;
		ids.push_back(id);
	}

	m_jobs[row]->setDependsOnJobIds(ids);
	notifyJobUpdated(row);

	return true;
}

// END OF bool JobsModel::setData(const QModelIndex & a_index,
//		const QVariant & a_value, int a_role)
//==============================================================================

const vsedit::Job * JobsModel::job(int a_index) const
{
	if((a_index < 0) || ((size_t)a_index >= m_jobs.size()))
		return nullptr;
	return m_jobs[a_index];
}

// END OF const vsedit::Job * JobsModel::job(int a_index) const
//==============================================================================

int JobsModel::createJob()
{
	vsedit::Job * pJob = new vsedit::Job(m_pSettingsManager,
		m_pVSScriptLibrary, this);
	connect(pJob, SIGNAL(signalLogMessage(const QString &, const QString &)),
		this, SLOT(slotLogMessage(const QString &, const QString &)));
	int newRow = (int)m_jobs.size();
	beginInsertRows(QModelIndex(), newRow, newRow);
	m_jobs.push_back(pJob);
	endInsertRows();
	return newRow;
}

// END OF int JobsModel::createJob()
//==============================================================================

bool JobsModel::deleteJob(int a_index)
{
	if((a_index < 0) || ((size_t)a_index >= m_jobs.size()))
		return false;

	JobState protectedStates[] = {JobState::Running, JobState::Paused,
		JobState::Aborting};

	vsedit::Job * pJob = m_jobs[a_index];
	if(vsedit::contains(protectedStates, pJob->state()))
	{
		emit signalLogMessage(trUtf8("Can not delete an active job."),
			LOG_STYLE_ERROR);
		return false;
	}

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

bool JobsModel::setJobType(int a_index, JobType a_type)
{
	if(!checkCanModifyJobAndNotify(a_index))
		return false;
	bool result = m_jobs[a_index]->setType(a_type);
	if(result)
		result = m_jobs[a_index]->setState(JobState::Waiting);
	notifyJobUpdated(a_index);
	return result;
}

// END OF bool JobsModel::setJobType(int a_index, JobType a_type)
//==============================================================================

bool JobsModel::setJobDependsOnIds(int a_index,
	const std::vector<QUuid> & a_ids)
{
	if(!checkCanModifyJobAndNotify(a_index))
		return false;
	bool result = m_jobs[a_index]->setDependsOnJobIds(a_ids);
	if(result)
		result = m_jobs[a_index]->setState(JobState::Waiting);
	notifyJobUpdated(a_index);
	return result;
}

// END OF bool JobsModel::setJobDependsOnIds(int a_index,
//		const std::vector<QUuid> & a_ids)
//==============================================================================

bool JobsModel::setJobScriptName(int a_index, const QString & a_scriptName)
{
	if(!checkCanModifyJobAndNotify(a_index))
		return false;
	bool result = m_jobs[a_index]->setScriptName(a_scriptName);
	if(result)
		result = m_jobs[a_index]->setState(JobState::Waiting);
	notifyJobUpdated(a_index);
	return result;
}

// END OF bool JobsModel::setJobScriptName(int a_index,
//		const QString & a_scriptName)
//==============================================================================

bool JobsModel::setJobEncodingHeaderType(int a_index,
	EncodingHeaderType a_headerType)
{
	if(!checkCanModifyJobAndNotify(a_index))
		return false;
	bool result = m_jobs[a_index]->setEncodingHeaderType(a_headerType);
	if(result)
		result = m_jobs[a_index]->setState(JobState::Waiting);
	notifyJobUpdated(a_index);
	return result;
}

// END OF bool JobsModel::setJobEncodingHeaderType(int a_index,
//		EncodingHeaderType a_headerType)
//==============================================================================

bool JobsModel::setJobExecutablePath(int a_index, const QString & a_path)
{
	if(!checkCanModifyJobAndNotify(a_index))
		return false;
	bool result = m_jobs[a_index]->setExecutablePath(a_path);
	if(result)
		result = m_jobs[a_index]->setState(JobState::Waiting);
	notifyJobUpdated(a_index);
	return result;
}

// END OF bool JobsModel::setJobExecutablePath(int a_index,
//		const QString & a_path)
//==============================================================================

bool JobsModel::setJobArguments(int a_index, const QString & a_arguments)
{
	if(!checkCanModifyJobAndNotify(a_index))
		return false;
	bool result = m_jobs[a_index]->setArguments(a_arguments);
	if(result)
		result = m_jobs[a_index]->setState(JobState::Waiting);
	notifyJobUpdated(a_index);
	return result;
}

// END OF bool JobsModel::setJobArguments(int a_index,
//		const QString & a_arguments)
//==============================================================================

bool JobsModel::setJobShellCommand(int a_index, const QString & a_command)
{
	if(!checkCanModifyJobAndNotify(a_index))
		return false;
	bool result = m_jobs[a_index]->setShellCommand(a_command);
	if(result)
		result = m_jobs[a_index]->setState(JobState::Waiting);
	notifyJobUpdated(a_index);
	return result;
}

// END OF bool JobsModel::setJobShellCommand(int a_index,
//		const QString & a_command)
//==============================================================================

bool JobsModel::setJobState(int a_index, JobState a_state)
{
	if(!checkCanModifyJobAndNotify(a_index))
		return false;
	bool result = m_jobs[a_index]->setState(a_state);
	notifyJobUpdated(a_index);
	return result;
}

// END OF bool JobsModel::setJobState(int a_index, JobState a_state)
//==============================================================================

void JobsModel::slotLogMessage(const QString & a_message,
	const QString & a_style)
{
	QString message = a_message;
	const vsedit::Job * cpJob = qobject_cast<const vsedit::Job *>(sender());
	if(cpJob)
	{
		ptrdiff_t index = indexOfJob(cpJob->id());
		if(index >= 0)
			message = QString("Job %1: %2").arg(index + 1).arg(a_message);
	}

	emit signalLogMessage(message, a_style);
}

// END OF void JobsModel::slotLogMessage(const QString & a_message,
//		const QString & a_style)
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

bool JobsModel::canModifyJob(int a_index) const
{
	if((a_index < 0) || ((size_t)a_index >= m_jobs.size()))
		return false;

	vsedit::Job * pJob = m_jobs[a_index];
	assert(pJob);
	if(!pJob)
		return false;

	JobState forbiddenStates[] = {JobState::Running, JobState::Paused,
		JobState::Aborting};
	if(vsedit::contains(forbiddenStates, pJob->state()))
		return false;

	return true;
}

// END OF bool JobsModel::canModifyJob(int a_index) const
//==============================================================================

bool JobsModel::checkCanModifyJobAndNotify(int a_index)
{
	bool result = canModifyJob(a_index);
	if(!result)
	{
		emit signalLogMessage(trUtf8("Can not modify an active job."),
			LOG_STYLE_ERROR);
	}
	return result;
}

// END OF bool JobsModel::checkCanModifyJobAndNotify(int a_index)
//==============================================================================

void JobsModel::notifyJobUpdated(int a_index)
{
	QModelIndex first = createIndex(a_index, 0);
	QModelIndex last = createIndex(a_index, COLUMNS_NUMBER - 1);
	emit dataChanged(first, last);
}

// END OF void JobsModel::noifyJobUpdated(int a_index)
//==============================================================================
