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
const int JobsModel::TIME_START_COLUMN = 5;
const int JobsModel::TIME_END_COLUMN = 6;
const int JobsModel::FPS_COLUMN = 7;
const int JobsModel::COLUMNS_NUMBER = 8;

//==============================================================================

JobsModel::JobsModel(SettingsManager * a_pSettingsManager,
	VSScriptLibrary * a_pVSScriptLibrary, QObject * a_pParent):
	  QAbstractItemModel(a_pParent)
	, m_pSettingsManager(a_pSettingsManager)
	, m_pVSScriptLibrary(a_pVSScriptLibrary)
	, m_fpsDisplayPrecision(DEFAULT_FPS_DISPLAY_PRECISION)
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
	case TIME_START_COLUMN:
		return trUtf8("Started");
	case TIME_END_COLUMN:
		return trUtf8("Ended");
	case FPS_COLUMN:
		return trUtf8("FPS");
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

	const QString dateTimeFormat = "yyyy-MM-dd\nhh:mm:ss.z";

	vsedit::Job * pJob = m_jobs[row];
	if(!pJob)
		return QVariant();

	if((a_role == Qt::DisplayRole) || (a_role == Qt::ToolTipRole))
	{
		if(column == NAME_COLUMN)
			return trUtf8("Job %1").arg(row + 1);
		else if(column == TYPE_COLUMN)
			return vsedit::Job::typeName(pJob->type());
		else if(column == SUBJECT_COLUMN)
			return pJob->subject();
		else if(column == STATE_COLUMN)
			return vsedit::Job::stateName(pJob->state());
		else if(column == DEPENDS_ON_COLUMN)
		{
			QStringList dependsList;
			for(const QUuid & a_uuid : pJob->dependsOnJobIds())
			{
				ptrdiff_t index = indexOfJob(a_uuid);
				if(index < 0)
					dependsList << trUtf8("<invalid job>");
				else
					dependsList << trUtf8("Job %1").arg(index + 1);
			}
			return dependsList.join(", ");
		}
		else if(column == TIME_START_COLUMN)
		{
			QDateTime timeStarted = pJob->properties().timeStarted;
			if(timeStarted != QDateTime())
				return timeStarted.toLocalTime().toString(dateTimeFormat);
		}
		else if(column == TIME_END_COLUMN)
		{
			QDateTime timeStarted = pJob->properties().timeEnded;
			if(timeStarted != QDateTime())
				return timeStarted.toLocalTime().toString(dateTimeFormat);
		}
		else if((column == FPS_COLUMN) &&
			(pJob->type() == JobType::EncodeScriptCLI) &&
			(pJob->framesProcessed() > 0))
		{
			QString fps = QString::number(pJob->fps(), 'f',
				m_fpsDisplayPrecision);
			if(vsedit::contains(ACTIVE_JOB_STATES, pJob->state()) &&
				(pJob->framesProcessed() < pJob->framesTotal()))
			{
				fps += "\n";
				fps += vsedit::timeToString(pJob->secondsToFinish());
			}
			return fps;
		}
	}
	else if(a_role == Qt::BackgroundRole)
	{
		QColor color = QGuiApplication::palette().color(QPalette::Base);

		switch(pJob->state())
		{
		case JobState::Aborted:
		case JobState::Failed:
		case JobState::DependencyNotMet:
			color = QColor("#ffcccc");
			break;
		case JobState::Aborting:
			color = QColor("#ffeeee");
			break;
		case JobState::Pausing:
		case JobState::Paused:
			color = QColor("#fffddd");
			break;
		case JobState::Completed:
			color = QColor("#ddffdd");
			break;
		case JobState::Running:
			color = QColor("#ddeeff");
			break;
		default:
			break;
		}

		return color;
	}
	else if(a_role == Qt::TextAlignmentRole)
	{
		const int centeredColumns[] = {STATE_COLUMN, TIME_START_COLUMN,
			TIME_END_COLUMN};
		if(vsedit::contains(centeredColumns, column))
			return Qt::AlignCenter;
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
	vsedit::Job * pJob = new vsedit::Job(JobProperties(),  m_pSettingsManager,
		m_pVSScriptLibrary, this);
	connectJob(pJob);
	int newRow = (int)m_jobs.size();
	beginInsertRows(QModelIndex(), newRow, newRow);
	m_jobs.push_back(pJob);
	endInsertRows();
	return newRow;
}

// END OF int JobsModel::createJob()
//==============================================================================

bool JobsModel::moveJobUp(int a_index)
{
	if(a_index == 0)
		return false;

	if(vsedit::contains(m_jobs[a_index]->dependsOnJobIds(),
		m_jobs[a_index - 1]->id()))
		return false;

	std::swap(m_jobs[a_index], m_jobs[a_index - 1]);
	QModelIndex first = createIndex(a_index - 1, 0);
	QModelIndex last = createIndex(a_index, COLUMNS_NUMBER - 1);
	emit dataChanged(first, last);

	first = createIndex(0, DEPENDS_ON_COLUMN);
	last = createIndex((int)m_jobs.size() - 1, DEPENDS_ON_COLUMN);
	emit dataChanged(first, last);

	return true;
}

// END OF bool JobsModel::moveJobUp(int a_index)
//==============================================================================

bool JobsModel::moveJobDown(int a_index)
{
	if(a_index >= ((int)m_jobs.size() - 1))
		return false;

	if(vsedit::contains(m_jobs[a_index + 1]->dependsOnJobIds(),
		m_jobs[a_index]->id()))
		return false;

	std::swap(m_jobs[a_index], m_jobs[a_index + 1]);
	QModelIndex first = createIndex(a_index, 0);
	QModelIndex last = createIndex(a_index + 1, COLUMNS_NUMBER - 1);
	emit dataChanged(first, last);

	first = createIndex(0, DEPENDS_ON_COLUMN);
	last = createIndex((int)m_jobs.size() - 1, DEPENDS_ON_COLUMN);
	emit dataChanged(first, last);

	return true;
}

// END OF bool JobsModel::moveJobDown(int a_index)
//==============================================================================

bool JobsModel::deleteJob(int a_index)
{
	if((a_index < 0) || ((size_t)a_index >= m_jobs.size()))
		return false;

	vsedit::Job * pJob = m_jobs[a_index];
	if(vsedit::contains(ACTIVE_JOB_STATES, pJob->state()))
	{
		emit signalLogMessage(trUtf8("Can not delete an active job."),
			LOG_STYLE_WARNING);
		return false;
	}

	for(const vsedit::Job * cpOtherJob : m_jobs)
	{
		if(vsedit::contains(cpOtherJob->dependsOnJobIds(), pJob->id()))
		{
			emit signalLogMessage(trUtf8("Can not delete a job while "
				"other jobs depend on it."), LOG_STYLE_WARNING);
			return false;
		}
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

bool JobsModel::loadJobs()
{
	if(hasActiveJobs())
	{
		emit signalLogMessage(trUtf8("Can not load jobs. "
			"Some of current jobs are still active.", LOG_STYLE_WARNING));
		return false;
	}

	if((!m_pSettingsManager) || (!m_pVSScriptLibrary))
	{
		emit signalLogMessage(trUtf8("Can not load jobs. "
			"Model is not initialized correctly.", LOG_STYLE_ERROR));
		return false;
	}

	beginResetModel();

	m_jobs.clear();

	std::vector<JobProperties> jobPropertiesList =
		m_pSettingsManager->getJobs();
	for(const JobProperties & properties : jobPropertiesList)
	{
		vsedit::Job * pJob = new vsedit::Job(properties, m_pSettingsManager,
			m_pVSScriptLibrary);
		connectJob(pJob);
		if(vsedit::contains(ACTIVE_JOB_STATES, pJob->state()))
			pJob->setState(JobState::Aborted);
		m_jobs.push_back(pJob);
	}

	endResetModel();

	return true;
}

// END OF bool JobsModel::loadJobs()
//==============================================================================

bool JobsModel::saveJobs()
{
	if(!m_pSettingsManager)
	{
		emit signalLogMessage(trUtf8("Can not save jobs. "
			"Model is not initialized correctly.", LOG_STYLE_ERROR));
		return false;
	}

	std::vector<JobProperties> jobPropertiesList;
	for(const vsedit::Job * cpJob : m_jobs)
		jobPropertiesList.push_back(cpJob->properties());

	bool result = m_pSettingsManager->setJobs(jobPropertiesList);

	if(!result)
		emit signalLogMessage(trUtf8("Failed to save jobs.", LOG_STYLE_ERROR));

	return result;
}

// END OF bool JobsModel::saveJobs()
//==============================================================================

bool JobsModel::hasActiveJobs()
{
	for(const vsedit::Job * cpJob : m_jobs)
	{
		if(vsedit::contains(ACTIVE_JOB_STATES, cpJob->state()))
			return true;
	}
	return false;
}

// END OF bool JobsModel::hasActiveJobs()
//==============================================================================

void JobsModel::startWaitingJobs()
{
	startFirstReadyJob();
}

// END OF void JobsModel::startWaitingJobs()
//==============================================================================

void JobsModel::abortActiveJobs()
{
	for(vsedit::Job * pJob : m_jobs)
	{
		if(!vsedit::contains(ACTIVE_JOB_STATES, pJob->state()))
			continue;
		pJob->abort();
	}
}

// END OF void JobsModel::abortActiveJobs()
//==============================================================================

void JobsModel::pauseActiveJobs()
{
	for(vsedit::Job * pJob : m_jobs)
	{
		if(pJob->state() != JobState::Running)
			continue;
		pJob->pause();
	}
}

// END OF void JobsModel::pauseActiveJobs()
//==============================================================================

void JobsModel::resumePausedJobs()
{
	for(vsedit::Job * pJob : m_jobs)
	{
		if(pJob->state() != JobState::Paused)
			continue;
		pJob->start();
	}
}

// END OF void JobsModel::resumePausedJobs()
//==============================================================================

void JobsModel::resetSelectedJobs()
{
	std::vector<int> jobIndexes = indexesFromSelection();
	for(int jobIndex : jobIndexes)
		setJobState(jobIndex, JobState::Waiting);
}

// END OF void JobsModel::resetSelectedJobs()
//==============================================================================

void JobsModel::deleteSelectedJobs()
{
	std::vector<QUuid> jobIds = idsFromSelection();
	for(const QUuid & jobId : jobIds)
		deleteJob(jobId);
}

// END OF void JobsModel::deleteSelectedJobs()
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

void JobsModel::setSelection(const QItemSelection & a_selection)
{
	m_selection = a_selection;
}

// END OF void JobsModel::setSelection(const QItemSelection & a_selection)
//==============================================================================

void JobsModel::slotJobStateChanged(JobState a_newState, JobState a_oldState)
{
	if(a_oldState == a_newState)
		return;

	vsedit::Job * pJob = qobject_cast<vsedit::Job *>(sender());
	if(!pJob)
		return;

	int jobIndex = indexOfJob(pJob->id());
	notifyJobUpdated(jobIndex);
	saveJobs();

	if(vsedit::contains(ACTIVE_JOB_STATES, a_newState))
		return;

	// Recursion
	if(a_newState == JobState::DependencyNotMet)
		return;

	// State reset
	if(a_newState == JobState::Waiting)
		return;

	startFirstReadyJob(jobIndex + 1);
}

// END OF void JobsModel::slotJobStateChanged(JobState a_newState,
//		JobState a_oldState)
//==============================================================================

void JobsModel::slotJobProgressChanged()
{
	vsedit::Job * pJob = qobject_cast<vsedit::Job *>(sender());
	if(!pJob)
		return;

	int jobIndex = indexOfJob(pJob->id());
	notifyJobUpdated(jobIndex);
}

// END OF void JobsModel::slotJobProgressChanged()
//==============================================================================

int JobsModel::indexOfJob(const QUuid & a_uuid) const
{
	std::vector<vsedit::Job *>::const_iterator it =
		std::find_if(m_jobs.cbegin(), m_jobs.cend(),
			[&](const vsedit::Job * la_pJob)->bool
			{
				return (la_pJob->id() == a_uuid);
			});

	return (it == m_jobs.cend()) ? -1 : std::distance(m_jobs.cbegin(), it);
}

// END OF int JobsModel::indexOfJob(const QUuid & a_uuid) const
//==============================================================================

void JobsModel::clearJobs()
{
	for(vsedit::Job * pJob : m_jobs)
		delete pJob;
	m_jobs.clear();
}

// END OF void JobsModel::clearJobs()
//==============================================================================

bool JobsModel::checkCanModifyJobAndNotify(int a_index)
{
	bool result = canModifyJob(a_index);
	if(!result)
	{
		emit signalLogMessage(trUtf8("Can not modify an active job."),
			LOG_STYLE_WARNING);
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

JobsModel::DependenciesState JobsModel::dependenciesState(int a_index)
{
	if((a_index < 0) || (a_index >= (int)m_jobs.size()))
		return DependenciesState::Failed;

	JobState failStates[] = {JobState::Aborted, JobState::Aborting,
		JobState::DependencyNotMet, JobState::Failed, JobState::FailedCleanUp};

	bool incomplete = false;

	for(const QUuid & id : m_jobs[a_index]->dependsOnJobIds())
	{
		int dependencyIndex = indexOfJob(id);
		if((dependencyIndex < 0) || (dependencyIndex >= (int)m_jobs.size()))
			return DependenciesState::Failed;
		if(m_jobs[dependencyIndex]->state() != JobState::Completed)
			incomplete = true;
		if(vsedit::contains(failStates, m_jobs[dependencyIndex]->state()))
			return DependenciesState::Failed;
	}

	if(incomplete)
		return DependenciesState::Incomplete;

	return DependenciesState::Complete;
}

// END OF JobsModel::DependenciesState JobsModel::dependenciesState(int a_index)
//==============================================================================

void JobsModel::connectJob(vsedit::Job * a_pJob)
{
	connect(a_pJob, SIGNAL(signalStateChanged(JobState, JobState)),
		this, SLOT(slotJobStateChanged(JobState, JobState)));
	connect(a_pJob, SIGNAL(signalProgressChanged()),
		this, SLOT(slotJobProgressChanged()));
	connect(a_pJob, SIGNAL(signalLogMessage(const QString &, const QString &)),
		this, SLOT(slotLogMessage(const QString &, const QString &)));
}

// END OF void JobsModel::connectJob(vsedit::Job * a_pJob)
//==============================================================================

void JobsModel::startFirstReadyJob(int a_fromIndex)
{
	if((a_fromIndex < 0) || (a_fromIndex >= (int)m_jobs.size()))
		return;

	JobState validStates[] = {JobState::Waiting, JobState::Paused};

	for(int i = a_fromIndex; i < (a_fromIndex + (int)m_jobs.size()); ++i)
	{
		int nextIndex = i % m_jobs.size();
		vsedit::Job * pNextJob = m_jobs[nextIndex];
		if(!vsedit::contains(validStates, pNextJob->state()))
			continue;
		DependenciesState jobDependenciesState = dependenciesState(i);
		if(jobDependenciesState == DependenciesState::Failed)
			pNextJob->setState(JobState::DependencyNotMet);
		if(jobDependenciesState != DependenciesState::Complete)
			continue;
		pNextJob->start();
		return;
	}
}

// END OF void JobsModel::startFirstReadyJob(int a_fromIndex)
//==============================================================================

std::vector<int> JobsModel::indexesFromSelection()
{
	std::set<int> indexesSet;
	QModelIndexList modelIndexList = m_selection.indexes();
	for(const QModelIndex & jobIndex : modelIndexList)
		indexesSet.insert(jobIndex.row());
	std::vector<int> indexesVector;
	std::copy(indexesSet.begin(), indexesSet.end(),
		std::back_inserter(indexesVector));
	return indexesVector;
}

// END OF std::vector<int> JobsModel::indexesFromSelection()
//==============================================================================

std::vector<QUuid> JobsModel::idsFromSelection()
{
	std::vector<int> indexesVector = indexesFromSelection();
	std::vector<QUuid> idsVector;
	for(int jobIndex : indexesVector)
		idsVector.push_back(m_jobs[jobIndex]->id());
	return idsVector;
}

// END OF std::vector<QUuid> JobsModel::idsFromSelection()
//==============================================================================
