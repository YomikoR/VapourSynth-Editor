#include "jobs_dialog.h"

#include "../settings/settings_manager.h"
#include "../common/highlight_item_delegate.h"
#include "jobs_model.h"
#include "job_state_delegate.h"
#include "job_dependencies_delegate.h"
#include "job_edit_dialog.h"

#include <QMessageBox>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QMenu>

#ifdef Q_OS_WIN
	#include <QWinTaskbarButton>
	#include <QWinTaskbarProgress>
#endif

//==============================================================================

const char JobsDialog::WINDOW_TITLE[] = "VapourSynth jobs";

//==============================================================================

JobsDialog::JobsDialog(SettingsManager * a_pSettingsManager,
	JobsModel * a_pJobsModel, VSScriptLibrary * a_pVSScriptLibrary,
	QWidget * a_pParent) :
	  QDialog(a_pParent, Qt::Window)
	, m_pSettingsManager(a_pSettingsManager)
	, m_pJobsModel(a_pJobsModel)
	, m_pVSScriptLibrary(a_pVSScriptLibrary)
	, m_pHighlightItemDelegate(nullptr)
	, m_pJobStateDelegate(nullptr)
	, m_pJobDependenciesDelegate(nullptr)
	, m_pJobEditDialog(nullptr)
	, m_pJobsHeaderMenu(nullptr)
#ifdef Q_OS_WIN
	, m_pWinTaskbarButton(nullptr)
	, m_pWinTaskbarProgress(nullptr)
#endif
{
	m_ui.setupUi(this);
	setWindowTitle(trUtf8(WINDOW_TITLE));

	m_ui.jobsTableView->setModel(m_pJobsModel);
	m_pHighlightItemDelegate = new HighlightItemDelegate(this);
	m_ui.jobsTableView->setItemDelegate(m_pHighlightItemDelegate);
	m_pJobStateDelegate = new JobStateDelegate(this);
	m_ui.jobsTableView->setItemDelegateForColumn(
		JobsModel::STATE_COLUMN, m_pJobStateDelegate);
	m_pJobDependenciesDelegate = new JobDependenciesDelegate(this);
	m_ui.jobsTableView->setItemDelegateForColumn(
		JobsModel::DEPENDS_ON_COLUMN, m_pJobDependenciesDelegate);

	QHeaderView * pHorizontalHeader = m_ui.jobsTableView->horizontalHeader();
	pHorizontalHeader->setSectionsMovable(true);
	pHorizontalHeader->setSectionResizeMode(JobsModel::NAME_COLUMN,
		QHeaderView::ResizeToContents);
	pHorizontalHeader->setSectionResizeMode(JobsModel::TYPE_COLUMN,
		QHeaderView::ResizeToContents);
	pHorizontalHeader->setSectionResizeMode(JobsModel::SUBJECT_COLUMN,
		QHeaderView::Interactive);
	pHorizontalHeader->resizeSection(JobsModel::SUBJECT_COLUMN, 500);
	pHorizontalHeader->setSectionResizeMode(JobsModel::STATE_COLUMN,
		QHeaderView::Stretch);
	pHorizontalHeader->setSectionResizeMode(JobsModel::DEPENDS_ON_COLUMN,
		QHeaderView::ResizeToContents);
	pHorizontalHeader->setSectionResizeMode(JobsModel::TIME_START_COLUMN,
		QHeaderView::ResizeToContents);
	pHorizontalHeader->setSectionResizeMode(JobsModel::TIME_END_COLUMN,
		QHeaderView::ResizeToContents);
	pHorizontalHeader->setSectionResizeMode(JobsModel::FPS_COLUMN,
		QHeaderView::ResizeToContents);
	pHorizontalHeader->setSectionResizeMode(JobsModel::CORE_COLUMN,
		QHeaderView::ResizeToContents);

	QHeaderView * pVerticalHeader = m_ui.jobsTableView->verticalHeader();
	pVerticalHeader->setSectionResizeMode(QHeaderView::ResizeToContents);

	m_pJobEditDialog = new JobEditDialog(m_pSettingsManager,
		m_pVSScriptLibrary, this);

	QItemSelectionModel * pSelectionModel =
		m_ui.jobsTableView->selectionModel();

	QByteArray newGeometry = m_pSettingsManager->getJobsDialogGeometry();
	if(!newGeometry.isEmpty())
		restoreGeometry(newGeometry);

	QByteArray headerState = m_pSettingsManager->getJobsHeaderState();
	if(!headerState.isEmpty())
		pHorizontalHeader->restoreState(headerState);

	pHorizontalHeader->setContextMenuPolicy(Qt::CustomContextMenu);
	m_pJobsHeaderMenu = new QMenu(pHorizontalHeader);
	for(int i = 0; i < m_pJobsModel->columnCount(); ++i)
	{
		QAction * pAction = new QAction(m_pJobsHeaderMenu);
		pAction->setText(
			m_pJobsModel->headerData(i, Qt::Horizontal).toString());
		pAction->setData(i);
		pAction->setCheckable(true);
		pAction->setChecked(!pHorizontalHeader->isSectionHidden(i));
		m_pJobsHeaderMenu->addAction(pAction);
		connect(pAction, SIGNAL(toggled(bool)),
			this, SLOT(slotShowJobsHeaderSection(bool)));
	}

	connect(m_pJobsModel,
		SIGNAL(signalLogMessage(const QString &, const QString &)),
		m_ui.log, SLOT(addEntry(const QString &, const QString &)));
	connect(m_pJobsModel,
		SIGNAL(signalStateChanged(int, int, JobState, int, int)),
		this, SLOT(slotJobsStateChanged(int, int, JobState, int, int)));
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
	connect(m_ui.jobsTableView, SIGNAL(doubleClicked(const QModelIndex &)),
		this, SLOT(slotJobDoubleClicked(const QModelIndex &)));
	connect(pSelectionModel, SIGNAL(selectionChanged(const QItemSelection &,
		const QItemSelection &)),
		this, SLOT(slotSelectionChanged()));
	connect(pHorizontalHeader, SIGNAL(sectionResized(int, int, int)),
		this, SLOT(slotSaveHeaderState()));
	connect(pHorizontalHeader, SIGNAL(sectionMoved(int, int, int)),
		this, SLOT(slotSaveHeaderState()));
	connect(pHorizontalHeader, SIGNAL(sectionCountChanged(int, int)),
		this, SLOT(slotSaveHeaderState()));
	connect(pHorizontalHeader, SIGNAL(geometriesChanged()),
		this, SLOT(slotSaveHeaderState()));
	connect(pHorizontalHeader,
		SIGNAL(customContextMenuRequested(const QPoint &)),
		this, SLOT(slotJobsHeaderContextMenu(const QPoint &)));
}

// END OF JobsDialog::JobsDialog(SettingsManager * a_pSettingsManager,
//		JobsModel * a_pJobsModel, VSScriptLibrary * a_pVSScriptLibrary,
//		QWidget * a_pParent)
//==============================================================================

JobsDialog::~JobsDialog()
{
}

// END OF JobsDialog::~JobsDialog()
//==============================================================================

void JobsDialog::show()
{
	if(m_pSettingsManager->getJobsDialogMaximized())
		showMaximized();
	else
		showNormal();
}

// END OF void JobsDialog::show()
//==============================================================================

void JobsDialog::moveEvent(QMoveEvent * a_pEvent)
{
	QDialog::moveEvent(a_pEvent);
	saveGeometrySettings();
}

// END OF void JobsDialog::moveEvent(QMoveEvent * a_pEvent)
//==============================================================================

void JobsDialog::resizeEvent(QResizeEvent * a_pEvent)
{
	QDialog::resizeEvent(a_pEvent);
	saveGeometrySettings();
}

// END OF void JobsDialog::resizeEvent(QResizeEvent * a_pEvent)
//==============================================================================

void JobsDialog::changeEvent(QEvent * a_pEvent)
{
	QDialog::changeEvent(a_pEvent);
	if(a_pEvent->type() == QEvent::WindowStateChange)
	{
		if(isMaximized())
			m_pSettingsManager->setJobsDialogMaximized(true);
		else
			m_pSettingsManager->setJobsDialogMaximized(false);
	}
}

// END OF void JobsDialog::changeEvent(QEvent * a_pEvent)
//==============================================================================

void JobsDialog::showEvent(QShowEvent * a_pEvent)
{
	QDialog::showEvent(a_pEvent);

#ifdef Q_OS_WIN
	if(!m_pWinTaskbarButton)
	{
		m_pWinTaskbarButton = new QWinTaskbarButton(this);
		m_pWinTaskbarButton->setWindow(windowHandle());
		m_pWinTaskbarProgress = m_pWinTaskbarButton->progress();
	}
#endif
}

// END OF void JobsDialog::showEvent(QShowEvent * a_pEvent)
//==============================================================================

void JobsDialog::slotJobNewButtonClicked()
{
	int result = m_pJobEditDialog->call(trUtf8("New job"), JobProperties());
	if(result == QDialog::Rejected)
		return;
	int index = m_pJobsModel->createJob();
	updateJob(index);
	m_ui.jobsTableView->selectRow(index);
	m_ui.jobsTableView->scrollTo(m_pJobsModel->index(index, 0));
}

// END OF void JobsDialog::slotJobNewButtonClicked()
//==============================================================================

void JobsDialog::slotJobEditButtonClicked()
{
	QModelIndex index = m_ui.jobsTableView->currentIndex();
	editJob(index);
}

// END OF void JobsDialog::slotJobEditButtonClicked()
//==============================================================================

void JobsDialog::slotJobMoveUpButtonClicked()
{
	QModelIndex index = m_ui.jobsTableView->currentIndex();
	bool result = m_pJobsModel->moveJobUp(index.row());
	if(result)
		m_ui.jobsTableView->selectRow(index.row() - 1);
}

// END OF void JobsDialog::slotJobMoveUpButtonClicked()
//==============================================================================

void JobsDialog::slotJobMoveDownButtonClicked()
{
	QModelIndex index = m_ui.jobsTableView->currentIndex();
	bool result = m_pJobsModel->moveJobDown(index.row());
	if(result)
		m_ui.jobsTableView->selectRow(index.row() + 1);
}

// END OF void JobsDialog::slotJobMoveDownButtonClicked()
//==============================================================================

void JobsDialog::slotJobDeleteButtonClicked()
{
	if(!m_ui.jobsTableView->selectionModel()->hasSelection())
		return;

	QMessageBox::StandardButton result = QMessageBox::question(this,
		trUtf8("Delete selected jobs"),
		trUtf8("Do you really want to delete selected jobs?"),
		QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No),
		QMessageBox::No);
	if(result == QMessageBox::No)
		return;

	m_pJobsModel->deleteSelectedJobs();
}

// END OF void JobsDialog::slotJobDeleteButtonClicked()
//==============================================================================

void JobsDialog::slotJobResetStateButtonClicked()
{
	m_pJobsModel->resetSelectedJobs();
}

// END OF void JobsDialog::slotJobResetStateButtonClicked()
//==============================================================================

void JobsDialog::slotStartButtonClicked()
{
	m_pJobsModel->startWaitingJobs();
}

// END OF void JobsDialog::slotStartButtonClicked()
//==============================================================================

void JobsDialog::slotPauseButtonClicked()
{
	m_pJobsModel->pauseActiveJobs();
}

// END OF void JobsDialog::slotPauseButtonClicked()
//==============================================================================

void JobsDialog::slotResumeButtonClicked()
{
	m_pJobsModel->resumePausedJobs();
}

// END OF void JobsDialog::slotResumeButtonClicked()
//==============================================================================

void JobsDialog::slotAbortButtonClicked()
{
	m_pJobsModel->abortActiveJobs();
}

// END OF void JobsDialog::slotAbortButtonClicked()
//==============================================================================

void JobsDialog::slotJobDoubleClicked(const QModelIndex & a_index)
{
	Qt::ItemFlags flags = m_pJobsModel->flags(a_index);
	if(flags & Qt::ItemIsEditable)
		return;
	editJob(a_index);
}

// END OF void JobsDialog::slotJobDoubleClicked(const QModelIndex & a_index)
//==============================================================================

void JobsDialog::slotSelectionChanged()
{
	QItemSelectionModel * pSelectionModel =
		m_ui.jobsTableView->selectionModel();
	m_pJobsModel->setSelection(pSelectionModel->selection());
}

// END OF void JobsDialog::slotSelectionChanged()
//==============================================================================

void JobsDialog::slotSaveHeaderState()
{
	QHeaderView * pHeader = m_ui.jobsTableView->horizontalHeader();
	m_pSettingsManager->setJobsHeaderState(pHeader->saveState());
}

// END OF void JobsDialog::slotSaveHeaderState()
//==============================================================================

void JobsDialog::slotJobsHeaderContextMenu(const QPoint & a_point)
{
	(void)a_point;
	m_pJobsHeaderMenu->exec(QCursor::pos());
}

// END OF void JobsDialog::slotJobsHeaderContextMenu(const QPoint & a_point)
//==============================================================================

void JobsDialog::slotShowJobsHeaderSection(bool a_show)
{
	QAction * pAction = qobject_cast<QAction *>(sender());
	if(!pAction)
		return;
	int section = pAction->data().toInt();
	QHeaderView * pHeader = m_ui.jobsTableView->horizontalHeader();
	pHeader->setSectionHidden(section, !a_show);
}

// END OF void JobsDialog::slotShowJobsHeaderSection(bool a_show)
//==============================================================================

void JobsDialog::slotJobsStateChanged(int a_job, int a_jobsTotal,
	JobState a_state, int a_progress, int a_progressMax)
{
	QString title;

	bool allJobsComplete = ((a_state == JobState::Completed) &&
		(a_job == a_jobsTotal));
	bool showJobsProgress = (!allJobsComplete) && (a_job != -1);
	if(showJobsProgress)
	{
		if(a_progressMax > 0)
		{
			int percent = a_progress * 100 / a_progressMax;
			title += QString("%1% ").arg(percent);
		}
		title += QString("%1/%2 ").arg(a_job).arg(a_jobsTotal);
	}

	title += trUtf8(WINDOW_TITLE);
	setWindowTitle(title);

#ifdef Q_OS_WIN
	if(!m_pWinTaskbarProgress)
		return;

	if(allJobsComplete || (a_state == JobState::Waiting))
	{
		m_pWinTaskbarProgress->hide();
		return;
	}

	m_pWinTaskbarProgress->setMaximum(a_progressMax);
	m_pWinTaskbarProgress->setValue(a_progress);
	m_pWinTaskbarProgress->show();

	JobState greenStates[] = {JobState::Running, JobState::Pausing,
		JobState::Aborting, JobState::Completed, JobState::CompletedCleanUp};
	JobState redStates[] = {JobState::Aborted, JobState::FailedCleanUp,
		JobState::Failed, JobState::DependencyNotMet};

	if(vsedit::contains(greenStates, a_state))
		m_pWinTaskbarProgress->resume();
	else if(a_state == JobState::Paused)
		m_pWinTaskbarProgress->pause();
	else if(vsedit::contains(redStates, a_state))
		m_pWinTaskbarProgress->stop();
#endif
}

// END OF void JobsDialog::slotJobsStateChanged(int a_job, int a_jobsTotal,
//		JobState a_state, int a_progress, int a_progressMax)
//==============================================================================

void JobsDialog::saveGeometrySettings()
{
	QApplication::processEvents();
	if(!isMaximized())
		m_pSettingsManager->setJobsDialogGeometry(saveGeometry());
}

// END OF void JobsDialog::saveGeometrySettings()
//==============================================================================

void JobsDialog::editJob(const QModelIndex & a_index)
{
	const vsedit::Job * cpJob = m_pJobsModel->job(a_index.row());
	if(!cpJob)
		return;
	int result = m_pJobEditDialog->call(trUtf8("Edit Job %1")
		.arg(a_index.row() + 1), cpJob->properties());
	if(result == QDialog::Rejected)
		return;
	updateJob(a_index.row());
}

// END OF void JobsDialog::editJob(const QModelIndex & a_index)
//==============================================================================

bool JobsDialog::updateJob(int a_index)
{
	JobProperties newProperties = m_pJobEditDialog->jobProperties();
	bool result = m_pJobsModel->setJobProperties(a_index, newProperties);
	return result;
}

// END OF bool JobsDialog::updateJob(int a_index)
//==============================================================================
