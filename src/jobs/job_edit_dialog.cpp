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

vsedit::Job JobEditDialog::job() const
{
	return m_job;
}

// END OF
//==============================================================================


int JobEditDialog::call(const vsedit::Job * a_cpJob)
{
	if(a_cpJob)
		m_job = *a_cpJob;
	else
		m_job = vsedit::Job();

	return exec();
}

// END OF
//==============================================================================
