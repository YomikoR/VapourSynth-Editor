#include <QKeySequenceEdit>
#include <QToolButton>
#include <QBoxLayout>

#include "itemdelegateforhotkey.h"

//==============================================================================

ClearableKeySequenceEditor::ClearableKeySequenceEditor(QWidget * a_pParent):
	QWidget(a_pParent)
	, m_pKeySequenceEdit(nullptr)
	, m_pClearKeySequenceButton(nullptr)
{
	QHBoxLayout * pLayout = new QHBoxLayout(this);
	pLayout->setMargin(0);
	pLayout->setContentsMargins(0, 0, 0, 0);
	pLayout->setSpacing(0);
	setLayout(pLayout);

	m_pKeySequenceEdit = new QKeySequenceEdit(this);
	pLayout->addWidget(m_pKeySequenceEdit);

	connect(m_pKeySequenceEdit, SIGNAL(editingFinished()),
		this, SIGNAL(editingFinished()));

	m_pClearKeySequenceButton = new QToolButton(this);
	m_pClearKeySequenceButton->setToolTip(trUtf8("Erase hotkey"));
	m_pClearKeySequenceButton->setIcon(QIcon(":erase.png"));
	pLayout->addWidget(m_pClearKeySequenceButton);

	connect(m_pClearKeySequenceButton, SIGNAL(clicked()),
		m_pKeySequenceEdit, SLOT(clear()));
}

ClearableKeySequenceEditor::~ClearableKeySequenceEditor()
{

}

QKeySequence ClearableKeySequenceEditor::keySequence() const
{
	return m_pKeySequenceEdit->keySequence();
}

void ClearableKeySequenceEditor::slotSetKeySequence(
	const QKeySequence & a_keySequence)
{
	m_pKeySequenceEdit->setKeySequence(a_keySequence);
}

//==============================================================================

ItemDelegateForHotkey::ItemDelegateForHotkey(QObject * a_pParent):
	QStyledItemDelegate(a_pParent)
{

}

ItemDelegateForHotkey::~ItemDelegateForHotkey()
{

}

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

void ItemDelegateForHotkey::setEditorData(QWidget * a_pEditor,
	const QModelIndex & a_index) const
{
	ClearableKeySequenceEditor * pEditor =
		static_cast<ClearableKeySequenceEditor *>(a_pEditor);
	QVariant newValue = a_index.model()->data(a_index, Qt::EditRole);
	pEditor->slotSetKeySequence(newValue.value<QKeySequence>());
}

void ItemDelegateForHotkey::updateEditorGeometry(QWidget * a_pEditor,
	const QStyleOptionViewItem & a_option, const QModelIndex & a_index) const
{
	(void)a_option;
	(void)a_index;
	a_pEditor->setGeometry(a_option.rect);
	return;
}

void ItemDelegateForHotkey::setModelData(QWidget * a_pEditor,
	QAbstractItemModel * a_model, const QModelIndex & a_index) const
{
	ClearableKeySequenceEditor * pEditor =
		static_cast<ClearableKeySequenceEditor *>(a_pEditor);
	a_model->setData(a_index, pEditor->keySequence(), Qt::EditRole);
	return;
}

void ItemDelegateForHotkey::slotCommitAndCloseEditor()
{
	ClearableKeySequenceEditor * pEditor =
		qobject_cast<ClearableKeySequenceEditor *>(sender());
    emit commitData(pEditor);
    emit closeEditor(pEditor);
}

//==============================================================================
