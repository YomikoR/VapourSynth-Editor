#include "clearable_key_sequence_editor.h"

#include <QKeySequenceEdit>
#include <QToolButton>
#include <QBoxLayout>

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
	m_pClearKeySequenceButton->setToolTip(tr("Erase hotkey"));
	m_pClearKeySequenceButton->setIcon(QIcon(":erase.png"));
	pLayout->addWidget(m_pClearKeySequenceButton);

	connect(m_pClearKeySequenceButton, SIGNAL(clicked()),
		m_pKeySequenceEdit, SLOT(clear()));
}

//==============================================================================

ClearableKeySequenceEditor::~ClearableKeySequenceEditor()
{

}

//==============================================================================

QKeySequence ClearableKeySequenceEditor::keySequence() const
{
	return m_pKeySequenceEdit->keySequence();
}

//==============================================================================

void ClearableKeySequenceEditor::slotSetKeySequence(
	const QKeySequence & a_keySequence)
{
	m_pKeySequenceEdit->setKeySequence(a_keySequence);
}

//==============================================================================
