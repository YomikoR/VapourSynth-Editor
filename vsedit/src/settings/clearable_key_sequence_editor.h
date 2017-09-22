#ifndef CLEARABLE_KEY_SEQUENCE_EDITOR_H_INCLUDED
#define CLEARABLE_KEY_SEQUENCE_EDITOR_H_INCLUDED

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

#endif // CLEARABLE_KEY_SEQUENCE_EDITOR_H_INCLUDED
