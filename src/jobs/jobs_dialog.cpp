#include "jobs_dialog.h"

#include "../settings/settings_manager.h"
#include "jobs_model.h"
#include "job_edit_dialog.h"

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
	, m_pJobEditDialog(nullptr)
{
	m_ui.setupUi(this);

	m_ui.jobsTableView->setModel(m_pJobsModel);

	m_pJobEditDialog = new JobEditDialog(m_pSettingsManager, this);

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
	int result = m_pJobEditDialog->call();
    if(result == QDialog::Rejected)
		return;
}

// END OF
//==============================================================================

void JobsDialog::slotJobEditButtonClicked()
{
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
}

// END OF
//==============================================================================

void JobsDialog::slotJobResetStateButtonClicked()
{
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

