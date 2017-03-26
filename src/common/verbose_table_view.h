#ifndef VERBOSE_TABLE_VIEW_H_INCLUDED
#define VERBOSE_TABLE_VIEW_H_INCLUDED

#include <QTableView>

class VerboseTableView : public QTableView
{
	Q_OBJECT

public:

	VerboseTableView(QWidget * a_pParent = nullptr);
	virtual ~VerboseTableView();

signals:

	void currentIndexChanged(const QModelIndex & a_index);

protected:

	virtual void currentChanged(const QModelIndex & a_current,
		const QModelIndex & a_previous) override;

};

#endif // VERBOSE_TABLE_VIEW_H_INCLUDED
