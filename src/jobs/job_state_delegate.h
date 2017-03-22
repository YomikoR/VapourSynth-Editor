#ifndef JOB_STATE_DELEGATE_H_INCLUDED
#define JOB_STATE_DELEGATE_H_INCLUDED

#include <QStyledItemDelegate>

class JobStateDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:

	JobStateDelegate(QObject * a_pParent = nullptr);
	virtual ~JobStateDelegate();

	virtual void paint(QPainter * a_pPainter,
		const QStyleOptionViewItem & a_option,
		const QModelIndex & a_index) const;

protected:

	virtual bool paintProgressBar(QPainter * a_pPainter,
		const QStyleOptionViewItem & a_option,
		const QModelIndex & a_index) const;
};

#endif // JOB_STATE_DELEGATE_H_INCLUDED
