#include "jobs_dialog.h"

#include "../settings/settings_manager.h"
#include "jobs_model.h"
#include "job_state_delegate.h"
#include "job_dependencies_delegate.h"
#include "job_edit_dialog.h"

#include <QMessageBox>
#include <QHeaderView>

//==============================================================================

JobsDialog::JobsDialog(SettingsManager * a_pSettingsManager,
	JobsModel * a_pJobsModel, QWidget * a_pParent) :
	  QDialog(a_pParent, (Qt::WindowFlags)0
		| Qt::Dialog
		| Qt::CustomizeWindowHint
		| Qt::WindowTitleHint
		| Qt::WindowMinimizeButtonHint
		| Qt::WindowMaximizeButtonHint
		| Qt::WindowCloseButtonHint)
	, m_pSettingsManager(a_pSettingsManager)
	, m_pJobsModel(a_pJobsModel)
	, m_pJobStateDelegate(nullptr)
	, m_pJobDependenciesDelegate(nullptr)
	, m_pJobEditDialog(nullptr)
{
	m_ui.setupUi(this);

	m_ui.jobsTableView->setModel(m_pJobsModel);
	m_pJobStateDelegate = new JobStateDelegate(this);
	m_ui.jobsTableView->setItemDelegateForColumn(
		JobsModel::STATE_COLUMN, m_pJobStateDelegate);
	m_pJobDependenciesDelegate = new JobDependenciesDelegate(this);
	m_ui.jobsTableView->setItemDelegateForColumn(
		JobsModel::DEPENDS_ON_COLUMN, m_pJobDependenciesDelegate);

	QHeaderView * pHorizontalHeader = m_ui.jobsTableView->horizontalHeader();
    pHorizontalHeader->setSectionResizeMode(JobsModel::NAME_COLUMN,
		QHeaderView::ResizeToContents);
    pHorizontalHeader->setSectionResizeMode(JobsModel::TYPE_COLUMN,
		QHeaderView::ResizeToContents);
    pHorizontalHeader->setSectionResizeMode(JobsModel::SUBJECT_COLUMN,
		QHeaderView::Interactive);
    pHorizontalHeader->setSectionResizeMode(JobsModel::STATE_COLUMN,
		QHeaderView::Stretch);
    pHorizontalHeader->setSectionResizeMode(JobsModel::DEPENDS_ON_COLUMN,
		QHeaderView::ResizeToContents);

	QHeaderView * pVerticalHeader = m_ui.jobsTableView->verticalHeader();
	pVerticalHeader->setSectionResizeMode(QHeaderView::ResizeToContents);

	m_pJobEditDialog = new JobEditDialog(m_pSettingsManager, this);

	connect(m_pJobsModel,
		SIGNAL(signalLogMessage(const QString &, const QString &)),
		m_ui.log, SLOT(addEntry(const QString &, const QString &)));
	connect(m_ui.jobNewButton, SIGNAL(clicked()),
		this, SLOT(slotJobNewButtonClicked()));
	connect(m_ui.jobEditButton, SIGNAL(clicked()),
		this, SLOT(slotJobEditButtonClicked()));
	connect(m_ui.jobMoveUpButton, SIGNAL(clicked()),
		this, SLOT(slotJobMoveUpButtonClicked()));
	connect(m_ui.jobMoveDownButton, SIGNAL(clicked()),
		this, SLOT(slotJobMoveDownButtonClicked()));
	connect(m_ui.jobDeleteButton, SIGNAL(clicked()),
		this, SLOT(slotJobDeleteButtonClicked()));
	connect(m_ui.jobResetStateButton, SIGNAL(clicked()),
		this, SLOT(slotJobResetStateButtonClicked()));
	connect(m_ui.startButton, SIGNAL(clicked()),
		this, SLOT(slotStartButtonClicked()));
	connect(m_ui.pauseButton, SIGNAL(clicked()),
		this, SLOT(slotPauseButtonClicked()));
	connect(m_ui.resumeButton, SIGNAL(clicked()),
		this, SLOT(slotResumeButtonClicked()));
	connect(m_ui.abortButton, SIGNAL(clicked()),
		this, SLOT(slotAbortButtonClicked()));

	int index = m_pJobsModel->createJob();
	m_pJobsModel->setJobState(index, JobState::Aborted);
	index = m_pJobsModel->createJob();
	m_pJobsModel->setJobState(index, JobState::Aborting);
	index = m_pJobsModel->createJob();
	m_pJobsModel->setJobState(index, JobState::Completed);
	index = m_pJobsModel->createJob();
	m_pJobsModel->setJobState(index, JobState::DependencyNotMet);
	index = m_pJobsModel->createJob();
	m_pJobsModel->setJobState(index, JobState::Failed);
	index = m_pJobsModel->createJob();
	m_pJobsModel->setJobState(index, JobState::Paused);
	index = m_pJobsModel->createJob();
	m_pJobsModel->setJobState(index, JobState::Running);
	index = m_pJobsModel->createJob();
	m_pJobsModel->setJobState(index, JobState::Waiting);
}

// END OF
//==============================================================================

JobsDialog::~JobsDialog()
{
}

// END OF
//==============================================================================

void JobsDialog::slotJobNewButtonClicked()
{
	int result = m_pJobEditDialog->call(trUtf8("New job"));
	if(result == QDialog::Rejected)
		return;
	int index = m_pJobsModel->createJob();
	updateJob(index);
}

// END OF
//==============================================================================

void JobsDialog::slotJobEditButtonClicked()
{
	QModelIndex index = m_ui.jobsTableView->currentIndex();
	const vsedit::Job * cpJob = m_pJobsModel->job(index.row());
	if(!cpJob)
		return;
	int result = m_pJobEditDialog->call(trUtf8("Edit Job %1")
		.arg(index.row() + 1), cpJob);
	if(result == QDialog::Rejected)
		return;
	updateJob(index.row());
}

// END OF
//==============================================================================

void JobsDialog::slotJobMoveUpButtonClicked()
{
}

// END OF
//==============================================================================

void JobsDialog::slotJobMoveDownButtonClicked()
{
}

// END OF
//==============================================================================

void JobsDialog::slotJobDeleteButtonClicked()
{
	QModelIndex index = m_ui.jobsTableView->currentIndex();
	if(!index.isValid())
		return;

	QMessageBox::StandardButton result = QMessageBox::question(this,
		trUtf8("Delete job"), trUtf8("Do you really want to delete Job %1?")
		.arg(index.row() + 1),
		QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No),
		QMessageBox::No);
	if(result == QMessageBox::No)
		return;

	m_pJobsModel->deleteJob(index.row());
}

// END OF
//==============================================================================

void JobsDialog::slotJobResetStateButtonClicked()
{
	QModelIndex index = m_ui.jobsTableView->currentIndex();
	if(!index.isValid())
		return;
	m_pJobsModel->setJobState(index.row(), JobState::Waiting);
}

// END OF
//==============================================================================

void JobsDialog::slotStartButtonClicked()
{
}

// END OF
//==============================================================================

void JobsDialog::slotPauseButtonClicked()
{
}

// END OF
//==============================================================================

void JobsDialog::slotResumeButtonClicked()
{
}

// END OF
//==============================================================================

void JobsDialog::slotAbortButtonClicked()
{
}

// END OF
//==============================================================================

bool JobsDialog::updateJob(int a_index)
{
	JobType type = m_pJobEditDialog->jobType();
	bool result = m_pJobsModel->setJobType(a_index, type);
	if(type == JobType::EncodeScriptCLI)
	{
		result = result && m_pJobsModel->setJobScriptName(a_index,
			m_pJobEditDialog->encodingScriptPath());
		result = result && m_pJobsModel->setJobEncodingHeaderType(a_index,
			m_pJobEditDialog->encodingHeaderType());
		result = result && m_pJobsModel->setJobExecutablePath(a_index,
			m_pJobEditDialog->encodingExecutablePath());
		result = result && m_pJobsModel->setJobArguments(a_index,
			m_pJobEditDialog->encodingArguments());
	}
	else if(type == JobType::RunProcess)
	{
		result = result && m_pJobsModel->setJobExecutablePath(a_index,
			m_pJobEditDialog->processExecutablePath());
		result = result && m_pJobsModel->setJobArguments(a_index,
			m_pJobEditDialog->processArguments());
	}
	else if(type == JobType::RunShellCommand)
	{
		result = result && m_pJobsModel->setJobShellCommand(a_index,
			m_pJobEditDialog->shellCommand());
	}
	else
		assert(false);

	return result;
}

// END OF
//==============================================================================
