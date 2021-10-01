#include "jobs_manager.h"

#include "../../../common-src/settings/settings_manager_core.h"
#include "../../../common-src/vapoursynth/vs_script_library.h"

//==============================================================================

JobsManager::JobsManager(SettingsManagerCore * a_pSettingsManager,
	QObject * a_pParent) :
	  QObject(a_pParent)
	, m_pSettingsManager(a_pSettingsManager)
	, m_pVSScriptLibrary(nullptr)
{
	Q_ASSERT(m_pSettingsManager);

	m_pVSScriptLibrary = new VSScriptLibrary(m_pSettingsManager, this);

	connect(m_pVSScriptLibrary,
		SIGNAL(signalWriteLogMessage(int, const QString &)),
		this, SLOT(slotLogMessage(int, const QString &)));
}

// END OF
//==============================================================================

JobsManager::~JobsManager()
{
}

// END OF
//==============================================================================

std::vector<JobProperties> JobsManager::jobsProperties() const
{
	std::vector<JobProperties> properties;
	for(const JobTicket & ticket : m_tickets)
		properties.push_back(ticket.pJob->properties());
	return properties;
}

// END OF
//==============================================================================

int JobsManager::createJob(const JobProperties & a_jobProperties)
{
	vsedit::Job * pJob = new vsedit::Job(a_jobProperties,  m_pSettingsManager,
		m_pVSScriptLibrary, this);
	connectJob(pJob);
	JobTicket ticket = {pJob, JobWantTo::Nothing};
	m_tickets.push_back(ticket);
	int newRow = (int)m_tickets.size();
	saveJobs();
	emit signalJobCreated(a_jobProperties);
	return newRow;
}

// END OF
//==============================================================================

bool JobsManager::swapJobs(const QUuid & a_jobID1, const QUuid & a_jobID2)
{
	int lowerIndex = indexOfJob(a_jobID1);
	if(lowerIndex < 0)
		return false;
	int higherIndex = indexOfJob(a_jobID2);
	if(higherIndex < 0)
		return false;
	if(higherIndex < lowerIndex)
		std::swap(lowerIndex, higherIndex);

	for(int i = lowerIndex; i < higherIndex; ++i)
	{
		if(vsedit::contains(m_tickets[higherIndex].pJob->dependsOnJobIds(),
			m_tickets[i].pJob->id()))
			return false;
	}

	for(int i = lowerIndex + 1; i < higherIndex; ++i)
	{
		if(vsedit::contains(m_tickets[i].pJob->dependsOnJobIds(),
			m_tickets[lowerIndex].pJob->id()))
			return false;
	}

	std::swap(m_tickets[lowerIndex], m_tickets[higherIndex]);
	saveJobs();
	emit signalJobsSwapped(a_jobID1, a_jobID2);
	return true;
}

// END OF
//==============================================================================

bool JobsManager::setJobState(const QUuid & a_jobID, JobState a_state)
{
	int index = indexOfJob(a_jobID);
	if(!checkCanModifyJobAndNotify(index))
		return false;
	bool result = m_tickets[index].pJob->setState(a_state);
	if(result)
		saveJobs();
	return result;
}

// END OF
//==============================================================================

bool JobsManager::setJobDependsOnIds(const QUuid & a_jobID,
	const std::vector<QUuid> & a_dependencies)
{
	int index = indexOfJob(a_jobID);
	if(!checkCanModifyJobAndNotify(index))
		return false;

	for(const QUuid & id : a_dependencies)
	{
		int dependencyIndex = indexOfJob(id);
		if((dependencyIndex < 0) || (dependencyIndex >= index))
			return false;
	}

	bool result = m_tickets[index].pJob->setDependsOnJobIds(a_dependencies);
	if(!result)
		return false;

	saveJobs();
	emit signalJobDependenciesChanged(a_jobID, a_dependencies);

	return true;
}

// END OF
//==============================================================================

bool JobsManager::changeJob(const JobProperties & a_jobProperties)
{
	int index = indexOfJob(a_jobProperties.id);
	if(!checkCanModifyJobAndNotify(index))
		return false;
	vsedit::Job * pJob = m_tickets[index].pJob;
	bool result = pJob->setProperties(a_jobProperties);
	if(result)
		result = pJob->setState(JobState::Waiting);
	saveJobs();
	emit signalJobChanged(pJob->properties());
	return result;
}

// END OF
//==============================================================================

bool JobsManager::loadJobs()
{
	if(hasActiveJobs())
	{
		emit signalLogMessage(tr("Can not load jobs. "
			"Some of current jobs are still active.", LOG_STYLE_WARNING));
		return false;
	}

	if((!m_pSettingsManager) || (!m_pVSScriptLibrary))
	{
		emit signalLogMessage(tr("Can not load jobs. "
			"Model is not initialized correctly.", LOG_STYLE_ERROR));
		return false;
	}

	clearJobs();

	std::vector<JobProperties> jobPropertiesList =
		m_pSettingsManager->getJobs();
	for(const JobProperties & properties : jobPropertiesList)
	{
		vsedit::Job * pJob = new vsedit::Job(properties, m_pSettingsManager,
			m_pVSScriptLibrary);
		if(vsedit::contains(ACTIVE_JOB_STATES, pJob->state()))
			pJob->setState(JobState::Aborted);
		connectJob(pJob);
		JobTicket ticket = {pJob, JobWantTo::Nothing};
		m_tickets.push_back(ticket);
	}

	return true;
}

// END OF
//==============================================================================

bool JobsManager::saveJobs()
{
	if(!m_pSettingsManager)
	{
		emit signalLogMessage(tr("Can not save jobs. "
			"Model is not initialized correctly.", LOG_STYLE_ERROR));
		return false;
	}

	std::vector<JobProperties> jobPropertiesList;
	for(const JobTicket & ticket : m_tickets)
		jobPropertiesList.push_back(ticket.pJob->properties());

	bool result = m_pSettingsManager->setJobs(jobPropertiesList);

	if(!result)
		emit signalLogMessage(tr("Failed to save jobs.", LOG_STYLE_ERROR));

	return result;
}

// END OF
//==============================================================================

bool JobsManager::hasActiveJobs()
{
	for(const JobTicket & ticket : m_tickets)
	{
		if(vsedit::contains(ACTIVE_JOB_STATES, ticket.pJob->state()))
			return true;
	}
	return false;
}

// END OF
//==============================================================================

void JobsManager::startWaitingJobs()
{
	startFirstReadyJob();
}

// END OF
//==============================================================================

void JobsManager::abortActiveJobs()
{
	for(JobTicket & ticket : m_tickets)
	{
		if(!vsedit::contains(ACTIVE_JOB_STATES, ticket.pJob->state()))
			continue;
		ticket.whenDone = JobWantTo::Nothing;
		ticket.pJob->abort();
	}
}

// END OF
//==============================================================================

void JobsManager::pauseActiveJobs()
{
	for(JobTicket & ticket : m_tickets)
	{
		if(ticket.pJob->state() != JobState::Running)
			continue;
		ticket.pJob->pause();
	}
}

// END OF
//==============================================================================

void JobsManager::resumePausedJobs()
{
	for(JobTicket & ticket : m_tickets)
	{
		if(ticket.pJob->state() != JobState::Paused)
			continue;
		ticket.pJob->start();
	}
}

// END OF
//==============================================================================

void JobsManager::resetJobs(const std::vector<QUuid> & a_ids)
{
	for(const QUuid & id : a_ids)
		setJobState(id, JobState::Waiting);
}

// END OF
//==============================================================================

void JobsManager::deleteJobs(const std::vector<QUuid> & a_ids)
{
	std::vector<QUuid> deletedJobs;
	for(const QUuid & id : a_ids)
	{
		int index = indexOfJob(id);
		if(index < 0)
			continue;

		vsedit::Job * pJob = m_tickets[index].pJob;
		if(vsedit::contains(ACTIVE_JOB_STATES, pJob->state()))
		{
			emit signalLogMessage(tr("Can not delete an active job."),
				LOG_STYLE_WARNING);
			return;
		}

		for(const JobTicket & ticket : m_tickets)
		{
			if(vsedit::contains(ticket.pJob->dependsOnJobIds(), pJob->id()))
			{
				emit signalLogMessage(tr("Can not delete a job while "
					"other jobs depend on it."), LOG_STYLE_WARNING);
				return;
			}
		}

		delete pJob;
		m_tickets.erase(m_tickets.begin() + index);
		deletedJobs.push_back(id);
	}
	saveJobs();
	emit signalJobsDeleted(deletedJobs);
}

// END OF
//==============================================================================

void JobsManager::slotLogMessage(const QString & a_message,
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

// END OF
//==============================================================================

void JobsManager::slotLogMessage(int a_type, const QString & a_message)
{
	QString style = vsMessageTypeToStyleName(a_type);
	slotLogMessage(a_message, style);
}

// END OF
//==============================================================================

void JobsManager::slotJobPropertiesChanged()
{
	vsedit::Job * pJob = qobject_cast<vsedit::Job *>(sender());
	if(!pJob)
		return;
	emit signalJobChanged(pJob->properties());
}

// END OF
//==============================================================================

void JobsManager::slotJobStateChanged(JobState a_newState, JobState a_oldState)
{
	if(a_oldState == a_newState)
		return;

	vsedit::Job * pJob = qobject_cast<vsedit::Job *>(sender());
	if(!pJob)
		return;

	saveJobs();
	emit signalJobStateChanged(pJob->id(), a_newState);

	if(vsedit::contains(ACTIVE_JOB_STATES, a_newState))
		return;

	// Recursion
	if(a_newState == JobState::DependencyNotMet)
		return;

	// State reset
	if(a_newState == JobState::Waiting)
		return;

	int jobIndex = indexOfJob(pJob->id());

	if(m_tickets[jobIndex].whenDone == JobWantTo::RunNext)
		startFirstReadyJob(jobIndex + 1);

	m_tickets[jobIndex].whenDone = JobWantTo::Nothing;
}

// END OF
//==============================================================================

void JobsManager::slotJobProgressChanged()
{
	vsedit::Job * pJob = qobject_cast<vsedit::Job *>(sender());
	if(!pJob)
		return;
	emit signalJobProgressChanged(pJob->id(), pJob->framesProcessed(),
		pJob->fps());
}

// END OF
//==============================================================================

void JobsManager::slotJobStartTimeChanged()
{
	vsedit::Job * pJob = qobject_cast<vsedit::Job *>(sender());
	if(!pJob)
		return;
	emit signalJobStartTimeChanged(pJob->id(), pJob->properties().timeStarted);
}

// END OF
//==============================================================================

void JobsManager::slotJobEndTimeChanged()
{
	vsedit::Job * pJob = qobject_cast<vsedit::Job *>(sender());
	if(!pJob)
		return;
	emit signalJobEndTimeChanged(pJob->id(), pJob->properties().timeEnded);
}

// END OF
//==============================================================================

bool JobsManager::canModifyJob(int a_index) const
{
	if((a_index < 0) || ((size_t)a_index >= m_tickets.size()))
		return false;

	vsedit::Job * pJob = m_tickets[a_index].pJob;
	Q_ASSERT(pJob);
	if(!pJob)
		return false;

	if(vsedit::contains(ACTIVE_JOB_STATES, pJob->state()))
		return false;

	return true;
}

// END OF
//==============================================================================

int JobsManager::indexOfJob(const QUuid & a_uuid) const
{
	std::vector<JobTicket>::const_iterator it =
		std::find_if(m_tickets.cbegin(), m_tickets.cend(),
			[&](const JobTicket & a_ticket)->bool
			{
				return (a_ticket.pJob->id() == a_uuid);
			});

	return (it == m_tickets.cend()) ?
		-1 : std::distance(m_tickets.cbegin(), it);
}

// END OF
//==============================================================================

void JobsManager::clearJobs()
{
	for(JobTicket & ticket : m_tickets)
		delete ticket.pJob;
	m_tickets.clear();
}

// END OF
//==============================================================================

bool JobsManager::checkCanModifyJobAndNotify(int a_index)
{
	bool result = canModifyJob(a_index);
	if(!result)
	{
		emit signalLogMessage(tr("Can not modify an active job."),
			LOG_STYLE_WARNING);
	}
	return result;
}

// END OF
//==============================================================================

JobsManager::DependenciesState JobsManager::dependenciesState(int a_index)
{
	if((a_index < 0) || (a_index >= (int)m_tickets.size()))
		return DependenciesState::Failed;

	JobState failStates[] = {JobState::Aborted, JobState::Aborting,
		JobState::DependencyNotMet, JobState::Failed, JobState::FailedCleanUp};

	bool incomplete = false;

	for(const QUuid & id : m_tickets[a_index].pJob->dependsOnJobIds())
	{
		int dependencyIndex = indexOfJob(id);
		if((dependencyIndex < 0) || (dependencyIndex >= (int)m_tickets.size()))
			return DependenciesState::Failed;
		const vsedit::Job * pDependentJob = m_tickets[dependencyIndex].pJob;
		if(pDependentJob->state() != JobState::Completed)
			incomplete = true;
		if(vsedit::contains(failStates, pDependentJob->state()))
			return DependenciesState::Failed;
	}

	if(incomplete)
		return DependenciesState::Incomplete;

	return DependenciesState::Complete;
}

// END OF
//==============================================================================

void JobsManager::connectJob(vsedit::Job * a_pJob)
{
	connect(a_pJob, SIGNAL(signalPropertiesChanged()),
		this, SLOT(slotJobPropertiesChanged()));
	connect(a_pJob, SIGNAL(signalStateChanged(JobState, JobState)),
		this, SLOT(slotJobStateChanged(JobState, JobState)));
	connect(a_pJob, SIGNAL(signalProgressChanged()),
		this, SLOT(slotJobProgressChanged()));
	connect(a_pJob, SIGNAL(signalStartTimeChanged()),
		this, SLOT(slotJobStartTimeChanged()));
	connect(a_pJob, SIGNAL(signalEndTimeChanged()),
		this, SLOT(slotJobEndTimeChanged()));
	connect(a_pJob, SIGNAL(signalLogMessage(const QString &, const QString &)),
		this, SLOT(slotLogMessage(const QString &, const QString &)));
}

// END OF
//==============================================================================

void JobsManager::startFirstReadyJob(int a_fromIndex)
{
	if((a_fromIndex < 0) || (a_fromIndex >= (int)m_tickets.size()))
		return;

	JobState validStates[] = {JobState::Waiting, JobState::Paused};

	for(int i = a_fromIndex; i < (a_fromIndex + (int)m_tickets.size()); ++i)
	{
		int nextIndex = i % m_tickets.size();
		vsedit::Job * pNextJob = m_tickets[nextIndex].pJob;
		if(pNextJob->isActive())
			return;
		if(!vsedit::contains(validStates, pNextJob->state()))
			continue;
		DependenciesState jobDependenciesState = dependenciesState(i);
		if(jobDependenciesState == DependenciesState::Failed)
			pNextJob->setState(JobState::DependencyNotMet);
		if(jobDependenciesState != DependenciesState::Complete)
			continue;
		m_tickets[nextIndex].whenDone = JobWantTo::RunNext;
		pNextJob->start();
		return;
	}
}

// END OF
//==============================================================================
