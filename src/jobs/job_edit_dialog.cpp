#include "job_edit_dialog.h"

#include "../settings/settings_manager.h"

#include <map>
#include <cassert>

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
	m_ui.jobTypeComboBox->setCurrentIndex(0);
	slotJobTypeChanged(m_ui.jobTypeComboBox->currentIndex());

	connect(m_ui.jobTypeComboBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(slotJobTypeChanged(int)));
	connect(m_ui.jobSaveButton, SIGNAL(clicked()), this, SLOT(accept()));
	connect(m_ui.cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

// END OF
//==============================================================================

JobEditDialog::~JobEditDialog()
{
}

// END OF
//==============================================================================

int JobEditDialog::call(const vsedit::Job * a_pJob)
{
	if(a_pJob)
	{
		int index = m_ui.jobTypeComboBox->findData((int)a_pJob->type());
		m_ui.jobTypeComboBox->setCurrentIndex(index);
	}


	return exec();
}

// END OF
//==============================================================================

void JobEditDialog::slotJobTypeChanged(int a_index)
{
	std::map<JobType, QWidget *> panels =
	{
		{JobType::EncodeScriptCLI, m_ui.encodingPanel},
		{JobType::RunProcess, m_ui.processPanel},
		{JobType::RunShellCommand, m_ui.shellCommandPanel},
	};

	JobType jobType = (JobType)m_ui.jobTypeComboBox->itemData(a_index).toInt();

	for(const std::pair<JobType, QWidget *> & pair : panels)
	{
		if(pair.first == jobType)
			pair.second->setVisible(true);
		else
			pair.second->setVisible(false);
	}
}

// END OF
//==============================================================================

