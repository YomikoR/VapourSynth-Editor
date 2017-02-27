#include "job.h"

//==============================================================================

vsedit::Job::Job(QObject * a_pParent) :
	  QObject(a_pParent)
{
}

// END OF
//==============================================================================

vsedit::Job::Job(const Job & a_other) :
	  QObject(a_other.parent())
{
}

// END OF
//==============================================================================

vsedit::Job & vsedit::Job::operator=(const vsedit::Job & a_other)
{
	if(&a_other == this)
		return *this;

	setParent(a_other.parent());

	return *this;
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

QUuid vsedit::Job::id() const
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

JobType vsedit::Job::type() const
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

JobState vsedit::Job::state() const
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

std::vector<QUuid> vsedit::Job::dependsOnJobIds() const
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

QString vsedit::Job::subject() const
{
	if(m_type == JobType::EncodeScriptCLI)
		return m_scriptName;
	else if(m_type == JobType::RunProcess)
		return m_executablePath;
	else if(m_type == JobType::RunShellCommand)
		return m_shellCommand;
	else
		return "-";
}

// END OF
//==============================================================================

QString vsedit::Job::typeName(JobType a_type)
{
	static std::map<JobType, QString> typeNameMap =
	{
		{JobType::EncodeScriptCLI, trUtf8("CLI encoding")},
		{JobType::RunProcess, trUtf8("Process run")},
		{JobType::RunShellCommand, trUtf8("Shell command")},
	};

	return typeNameMap[a_type];
}

// END OF
//==============================================================================

QString vsedit::Job::stateName(JobState a_state)
{
	static std::map<JobState, QString> stateNameMap =
	{
		{JobState::Waiting, trUtf8("Waiting")},
		{JobState::Running, trUtf8("Running")},
		{JobState::Paused, trUtf8("Paused")},
		{JobState::Aborted, trUtf8("Aborted")},
		{JobState::Failed, trUtf8("Failed")},
		{JobState::DependencyNotMet, trUtf8("Dependency not met")},
		{JobState::Completed, trUtf8("Completed")},
	};

	return stateNameMap[a_state];
}

// END OF
//==============================================================================
