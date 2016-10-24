#ifndef ITEMDELEGATEFORHOTKEY_H
#define ITEMDELEGATEFORHOTKEY_H

#include <QStyledItemDelegate>

class ItemDelegateForHotkey : public QStyledItemDelegate
{
	Q_OBJECT

public:

	ItemDelegateForHotkey(QObject * a_pParent = nullptr);

	virtual ~ItemDelegateForHotkey();

	QWidget * createEditor(QWidget * a_pParent,
		const QStyleOptionViewItem & a_option, const QModelIndex & a_index)
		const override;

	void setEditorData(QWidget * a_pEditor, const QModelIndex & a_index)
		const override;

	void updateEditorGeometry(QWidget * a_pEditor,
		const QStyleOptionViewItem & a_option, const QModelIndex & a_index)
		const override;

	void setModelData(QWidget * a_pEditor, QAbstractItemModel * a_model,
		const QModelIndex & a_index) const override;

private slots:

	void slotCommitAndCloseEditor();
};

#endif // ITEMDELEGATEFORHOTKEY_H
