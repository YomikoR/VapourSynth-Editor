#ifndef ITEMDELEGATEFORHOTKEY_H
#define ITEMDELEGATEFORHOTKEY_H

#include <QStyledItemDelegate>
#include <QWidget>
#include <QKeySequence>

class QKeySequenceEdit;
class QToolButton;

class ClearableKeySequenceEditor : public QWidget
{
	Q_OBJECT

	public:

		ClearableKeySequenceEditor(QWidget * a_pParent);

		virtual ~ClearableKeySequenceEditor();

		QKeySequence keySequence() const;

	public slots:

		void slotSetKeySequence(const QKeySequence & a_keySequence);

	signals:

		void editingFinished();

	private:

		QKeySequenceEdit * m_pKeySequenceEdit;

		QToolButton * m_pClearKeySequenceButton;
};

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
