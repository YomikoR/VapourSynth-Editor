#include "job_edit_dialog.h"

#include "../settings/settings_manager.h"

//==============================================================================

JobEditDialog::JobEditDialog(SettingsManager * a_pSettingsManager,
	QWidget * a_pParent) :
	  QDialog(a_pParent, (Qt::WindowFlags)0
		| Qt::Dialog
		| Qt::CustomizeWindowHint
		| Qt::WindowTitleHint
		| Qt::WindowMinimizeButtonHint
		| Qt::WindowMaximizeButtonHint
		| Qt::WindowCloseButtonHint)
	, m_pSettingsManager(a_pSettingsManager)
	, m_pJob(nullptr)
{
	m_ui.setupUi(this);

	JobType jobTypes[] = {JobType::EncodeScriptCLI, JobType::RunProcess,
		JobType::RunShellCommand};
	for(const JobType & jobType : jobTypes)
		m_ui.jobTypeComboBox->addItem(vsedit::Job::typeName(jobType),
			(int)jobType);
}

// END OF
//==============================================================================

JobEditDialog::~JobEditDialog()
{
}

// END OF
//==============================================================================

int JobEditDialog::call(vsedit::Job * a_pJob)
{
	if(!a_pJob)
		return QDialog::Rejected;

	m_pJob = a_pJob;
	return exec();
}

// END OF
//==============================================================================
