#ifndef HIGHLIGHT_ITEM_DELEGATE_H_INCLUDED
#define HIGHLIGHT_ITEM_DELEGATE_H_INCLUDED

#include <QStyledItemDelegate>

class HighlightItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:

	HighlightItemDelegate(QObject * a_pParent = nullptr);
	virtual ~HighlightItemDelegate();

	virtual void paint(QPainter * a_pPainter,
		const QStyleOptionViewItem & a_option, const QModelIndex & a_index)
		const override;

};

#endif // HIGHLIGHT_ITEM_DELEGATE_H_INCLUDED
