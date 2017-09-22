#ifndef JOB_STATE_DELEGATE_H_INCLUDED
#define JOB_STATE_DELEGATE_H_INCLUDED

#include "../../../common-src/settings/settings_definitions_core.h"

#include <QStyledItemDelegate>

class JobStateDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:

	JobStateDelegate(QObject * a_pParent = nullptr);
	virtual ~JobStateDelegate();

	virtual void paint(QPainter * a_pPainter,
		const QStyleOptionViewItem & a_option,
		const QModelIndex & a_index) const override;

protected:

	virtual QColor jobStateColor(JobState a_state,
		const QStyleOptionViewItem & a_option) const;
};

#endif // JOB_STATE_DELEGATE_H_INCLUDED
