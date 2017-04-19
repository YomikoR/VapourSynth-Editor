#ifndef JOB_DEPENDENCIES_DELEGATE_H_INCLUDED
#define JOB_DEPENDENCIES_DELEGATE_H_INCLUDED

#include "../common/highlight_item_delegate.h"

class JobDependenciesDelegate : public HighlightItemDelegate
{
	Q_OBJECT

public:

	JobDependenciesDelegate(QObject * a_pParent = nullptr);
	virtual ~JobDependenciesDelegate();

	virtual QWidget * createEditor(QWidget * a_pParent,
		const QStyleOptionViewItem & a_option,
		const QModelIndex & a_index) const override;

	virtual void setEditorData(QWidget * a_pEditor,
		const QModelIndex & a_index) const override;

	virtual void setModelData(QWidget * a_pEditor,
		QAbstractItemModel * a_pModel, const QModelIndex & a_index)
		const override;

	virtual void updateEditorGeometry(QWidget * a_pEditor,
		const QStyleOptionViewItem & a_option,
		const QModelIndex & a_index) const override;

};

#endif // JOB_DEPENDENCIES_DELEGATE_H_INCLUDED
