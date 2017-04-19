#include "highlight_item_delegate.h"

#include "helpers.h"

#include <QApplication>

HighlightItemDelegate::HighlightItemDelegate(QObject * a_pParent):
	QStyledItemDelegate(a_pParent)
{
}

HighlightItemDelegate::~HighlightItemDelegate()
{
}

void HighlightItemDelegate::paint(QPainter * a_pPainter,
	const QStyleOptionViewItem & a_option, const QModelIndex & a_index) const
{
	QStyleOptionViewItemV4 newOption = a_option;
	initStyleOption(&newOption, a_index);

	QColor backgroundColor = a_index.model()->data(a_index,
		Qt::BackgroundRole).value<QBrush>().color();
	if(newOption.state & QStyle::State_Selected)
		backgroundColor = vsedit::highlight(backgroundColor);
	newOption.state &= ~QStyle::State_Selected;
	newOption.backgroundBrush = backgroundColor;

	QApplication::style()->drawControl(QStyle::CE_ItemViewItem,
		&newOption, a_pPainter);
}
