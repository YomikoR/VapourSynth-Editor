#include "verbose_table_view.h"

VerboseTableView::VerboseTableView(QWidget * a_pParent):
	QTableView(a_pParent)
{
}

VerboseTableView::~VerboseTableView()
{
}

void VerboseTableView::currentChanged(const QModelIndex & a_current,
	const QModelIndex & a_previous)
{
	QTableView::currentChanged(a_current, a_previous);
	emit currentIndexChanged(a_current);
}
