#include "job.h"

//==============================================================================

vsedit::Job::Job(QObject * a_pParent) :
	  QObject(a_pParent)
{
}

// END OF
//==============================================================================

vsedit::Job::~Job()
{
}

// END OF
//==============================================================================

void vsedit::Job::start()
{
}

// END OF
//==============================================================================

void vsedit::Job::pause()
{
}

// END OF
//==============================================================================

void vsedit::Job::abort()
{
}

// END OF
//==============================================================================

QUuid vsedit::Job::id()
{
	return m_id;
}

// END OF
//==============================================================================

bool vsedit::Job::setId(const QUuid & a_id)
{
	m_id = a_id;
	return true;
}

// END OF
//==============================================================================

JobType vsedit::Job::type()
{
	return m_type;
}

// END OF
//==============================================================================

bool vsedit::Job::setType(JobType a_type)
{
	m_type = a_type;
	return true;
}

// END OF
//==============================================================================

JobState vsedit::Job::state()
{
	return m_jobState;
}

// END OF
//==============================================================================

bool vsedit::Job::setState(JobState a_state)
{
	m_jobState = a_state;
	return true;
}

// END OF
//==============================================================================

std::vector<QUuid> vsedit::Job::dependsOnJobIds()
{
	return m_dependsOnJobIds;
}

// END OF
//==============================================================================

bool vsedit::Job::setDependsOnJobIds(const std::vector<QUuid> & a_ids)
{
	m_dependsOnJobIds = a_ids;
	return true;
}

// END OF
//==============================================================================

