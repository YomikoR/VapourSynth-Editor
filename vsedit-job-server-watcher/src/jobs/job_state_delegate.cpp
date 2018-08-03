#include "job_state_delegate.h"

#include "jobs_model.h"

#include "../../../common-src/helpers.h"

#include <QPainter>

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
	a_pPainter->save();
	const JobsModel * cpModel =
		qobject_cast<const JobsModel *>(a_index.model());
	Q_ASSERT(cpModel);
	Q_ASSERT(a_index.column() == JobsModel::STATE_COLUMN);
	JobProperties properties = cpModel->jobProperties(a_index.row());
	JobState state = properties.jobState;
	QColor cellColor = jobStateColor(state, a_option);

	JobState noProgressBarStates[] = {JobState::Waiting,
		JobState::DependencyNotMet};
	bool drawProgress = (properties.type == JobType::EncodeScriptCLI) &&
		(!vsedit::contains(noProgressBarStates, state));

	a_pPainter->setPen(cellColor);
	a_pPainter->setBrush(cellColor);

	QRect innerRect = a_option.rect;
	innerRect.setWidth(a_option.rect.width() - 1);
	innerRect.setHeight(a_option.rect.height() - 1);

	if(drawProgress)
	{
		int progressWidth = properties.framesProcessed * innerRect.width() /
			properties.framesTotal();
		QRect progressRect = innerRect;
		progressRect.setWidth(progressWidth);
		a_pPainter->drawRect(progressRect);
		QRect blankRect = innerRect;
		blankRect.setWidth(innerRect.width() - progressWidth);
		blankRect.translate(progressWidth, 0);
		a_pPainter->setBrush(a_option.palette.color(QPalette::Base));
		a_pPainter->drawRect(blankRect);
	}
	else
		a_pPainter->drawRect(innerRect);

	QString stateText = cpModel->data(a_index).toString();
	if(drawProgress)
	{
		stateText += QString(" %1 / %2").arg(properties.framesProcessed)
			.arg(properties.framesTotal());
	}

	a_pPainter->setPen(a_option.palette.color(QPalette::Text));
	a_pPainter->setFont(a_option.font);
	a_pPainter->drawText(innerRect, Qt::AlignCenter | Qt::TextWordWrap,
		stateText);

	a_pPainter->restore();
}

QColor JobStateDelegate::jobStateColor(JobState a_state,
	const QStyleOptionViewItem & a_option) const
{
	switch(a_state)
	{
	case JobState::Aborted:
	case JobState::Failed:
	case JobState::DependencyNotMet:
		return QColor("#ffcccc");
	case JobState::Aborting:
	case JobState::FailedCleanUp:
		return QColor("#ffeeee");
	case JobState::Pausing:
	case JobState::Paused:
		return QColor("#fffddd");
	case JobState::CompletedCleanUp:
	case JobState::Completed:
		return QColor("#ddffdd");
	case JobState::Running:
		return QColor("#ddeeff");
	default:
		return a_option.palette.color(QPalette::Base);
	}
}
