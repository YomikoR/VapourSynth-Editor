#include "item_delegate_for_hotkey.h"

#include "clearable_key_sequence_editor.h"

//==============================================================================

ItemDelegateForHotkey::ItemDelegateForHotkey(QObject * a_pParent):
	QStyledItemDelegate(a_pParent)
{

}

//==============================================================================

ItemDelegateForHotkey::~ItemDelegateForHotkey()
{

}

//==============================================================================

QWidget * ItemDelegateForHotkey::createEditor(QWidget * a_pParent,
	const QStyleOptionViewItem & a_option, const QModelIndex & a_index) const
{
	(void)a_option;
	(void)a_index;
	ClearableKeySequenceEditor * pEditor =
		new ClearableKeySequenceEditor(a_pParent);
	connect(pEditor, SIGNAL(editingFinished()),
		this, SLOT(slotCommitAndCloseEditor()));
	return pEditor;
}

//==============================================================================

void ItemDelegateForHotkey::setEditorData(QWidget * a_pEditor,
	const QModelIndex & a_index) const
{
	ClearableKeySequenceEditor * pEditor =
		static_cast<ClearableKeySequenceEditor *>(a_pEditor);
	QVariant newValue = a_index.model()->data(a_index, Qt::EditRole);
	pEditor->slotSetKeySequence(newValue.value<QKeySequence>());
}

//==============================================================================

void ItemDelegateForHotkey::updateEditorGeometry(QWidget * a_pEditor,
	const QStyleOptionViewItem & a_option, const QModelIndex & a_index) const
{
	(void)a_option;
	(void)a_index;
	a_pEditor->setGeometry(a_option.rect);
	return;
}

//==============================================================================

void ItemDelegateForHotkey::setModelData(QWidget * a_pEditor,
	QAbstractItemModel * a_model, const QModelIndex & a_index) const
{
	ClearableKeySequenceEditor * pEditor =
		static_cast<ClearableKeySequenceEditor *>(a_pEditor);
	a_model->setData(a_index, pEditor->keySequence(), Qt::EditRole);
	return;
}

//==============================================================================

void ItemDelegateForHotkey::slotCommitAndCloseEditor()
{
	ClearableKeySequenceEditor * pEditor =
		qobject_cast<ClearableKeySequenceEditor *>(sender());
    emit commitData(pEditor);
    emit closeEditor(pEditor);
}

//==============================================================================
