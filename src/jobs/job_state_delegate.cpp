#include "job_state_delegate.h"

#include "jobs_model.h"

#include <QApplication>

JobStateDelegate::JobStateDelegate(QObject * a_pParent) :
	QStyledItemDelegate(a_pParent)
{
}

JobStateDelegate::~JobStateDelegate()
{
}

void JobStateDelegate::paint(QPainter * a_pPainter,
	const QStyleOptionViewItem & a_option, const QModelIndex & a_index) const
{
	bool painted = paintProgressBar(a_pPainter, a_option, a_index);
	if(!painted)
		QStyledItemDelegate::paint(a_pPainter, a_option, a_index);
}

bool JobStateDelegate::paintProgressBar(QPainter * a_pPainter,
	const QStyleOptionViewItem & a_option,
	const QModelIndex & a_index) const
{
	const JobsModel * cpModel =
		qobject_cast<const JobsModel *>(a_index.model());
	if((!cpModel) || (a_index.column() != JobsModel::STATE_COLUMN))
		return false;

	const vsedit::Job * cpJob = cpModel->job(a_index.row());
	if(!cpJob)
		return false;

	if(cpJob->type() == JobType::RunShellCommand)
		return false;

	JobState state = cpJob->state();

	JobState noProgressBarSates[] = {JobState::Waiting,
		JobState::DependencyNotMet};
	if(vsedit::contains(noProgressBarSates, state))
		return false;

	JobState processNoProgressBarSates[] = {JobState::Failed,
		JobState::Completed};
	if((cpJob->type() == JobType::RunProcess) &&
			vsedit::contains(processNoProgressBarSates, state))
		return false;

	QStyleOptionProgressBar progressBarOption;
	progressBarOption.rect = a_option.rect;
	progressBarOption.minimum = 0;
	progressBarOption.maximum = cpJob->framesTotal();
	progressBarOption.progress = cpJob->framesProcessed();
	progressBarOption.text = vsedit::Job::stateName(state);
	progressBarOption.textVisible = true;
	progressBarOption.textAlignment = Qt::AlignCenter;

	if(cpJob->type() == JobType::EncodeScriptCLI)
	{
		progressBarOption.text += QString(" %1 / %2")
			.arg(cpJob->framesProcessed()).arg(cpJob->framesTotal());
	}

	QApplication::style()->drawControl(QStyle::CE_ProgressBar,
		&progressBarOption, a_pPainter);

	return true;
}
