#include <QTextBlock>
#include <QCursor>
#include <QCompleter>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QPainter>
#include <QAction>
#include <algorithm>
#include <cassert>

#include "scriptcompletermodel.h"
#include "scriptcompleter.h"
#include "syntaxhighlighter.h"
#include "../settings/settingsmanager.h"
#include "../settings/settingsdialog.h"

#include "scripteditor.h"

//==============================================================================

ScriptEditor::ScriptEditor(QWidget * a_pParent) :
	QPlainTextEdit(a_pParent)
	, m_pSettingsManager(nullptr)
	, m_pSideBox(nullptr)
	, m_sideBoxLineWidth(1)
	, m_sideBoxTextMargin(3)
	, m_pCompleterModel(nullptr)
	, m_pCompleter(nullptr)
	, m_pSyntaxHighlighter(nullptr)
	, m_typedCharacters(0)
	, m_charactersTypedToStartCompletion(
		DEFAULT_CHARACTERS_TYPED_TO_START_COMPLETION)
	, m_plainText()
	, m_backgroundColor(Qt::white)
	, m_activeLineColor(Qt::lightGray)
	, m_commonScriptTextFormat()
	, m_tabText("\t")
	, m_spacesInTab(DEFAULT_SPACES_IN_TAB)
{
	m_pSideBox = new QWidget(this);
	m_pSideBox->installEventFilter(this);

	m_pCompleterModel = new ScriptCompleterModel(this);

	m_pCompleter = new ScriptCompleter(m_pCompleterModel, this);
	m_pCompleter->setWidget(this);
	m_pCompleter->setCompletionMode(QCompleter::PopupCompletion);
	m_pCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	m_pCompleter->setWrapAround(false);

	m_pSyntaxHighlighter = new SyntaxHighlighter(document());

	connect(m_pCompleter, SIGNAL(activated(const QString &)),
		this, SLOT(slotInsertCompletion(const QString &)));
	connect(this, SIGNAL(textChanged()), this, SLOT(slotTextChanged()));
	connect(this, SIGNAL(blockCountChanged(int)),
		this, SLOT(slotUpdateSideBoxWidth()));
	connect(this, SIGNAL(updateRequest(const QRect &, int)),
		this, SLOT(slotUpdateSideBox(const QRect &, int)));
	connect(this, SIGNAL(cursorPositionChanged()),
		this, SLOT(slotHighlightCurrentBlock()));

	slotLoadSettings();
}

// END OF ScriptEditor::ScriptEditor(QWidget * a_pParent)
//==============================================================================

ScriptEditor::~ScriptEditor()
{

}

// END OF ScriptEditor::~ScriptEditor()
//==============================================================================

QString ScriptEditor::text()
{
	return document()->toPlainText();
}

// END OF QString ScriptEditor::text()
//==============================================================================

QPoint ScriptEditor::cursorPosition()
{
	QTextCursor currentCursor = textCursor();
	int line = currentCursor.blockNumber();
	int index = currentCursor.anchor() - currentCursor.position();

	return QPoint(line, index);
}

// END OF QPoint ScriptEditor::cursorPosition()
//==============================================================================

void ScriptEditor::setCursorPosition(const QPoint & a_point)
{
	setCursorPosition(a_point.x(), a_point.y());
}

// END OF void ScriptEditor::setCursorPosition(const QPoint & a_point)
//==============================================================================

void ScriptEditor::setCursorPosition(int a_line, int a_index)
{
	int line = std::max(0, std::min(a_line, blockCount() - 1));
	QTextBlock block = document()->findBlockByNumber(line);
	int index = std::max(0, std::min(a_index, block.length() - 1));
	int newCursorPosition = block.position() + index;
	QTextCursor newCursor = textCursor();
	newCursor.setPosition(newCursorPosition);
	setTextCursor(newCursor);
}

// END OF void ScriptEditor::setCursorPosition(int a_line, int a_index)
//==============================================================================

bool ScriptEditor::isModified()
{
	return document()->isModified();
}

// END OF bool ScriptEditor::isModified()
//==============================================================================

void ScriptEditor::setModified(bool a_modified)
{
	document()->setModified(a_modified);
}

// END OF void ScriptEditor::setModified(bool a_modified)
//==============================================================================

void ScriptEditor::setPluginsList(const VSPluginsList & a_pluginsList)
{
	m_pCompleterModel->setPluginsList(a_pluginsList);
	m_pSyntaxHighlighter->setPluginsList(a_pluginsList);
}

// END OF void ScriptEditor::setModified(bool a_modified)
//==============================================================================

void ScriptEditor::setCharactersTypedToStartCompletion(int a_charactersNumber)
{
	m_charactersTypedToStartCompletion = a_charactersNumber;
}

// END OF void ScriptEditor::setModified(bool a_modified)
//==============================================================================

void ScriptEditor::setSettingsManager(SettingsManager * a_pSettingsManager)
{
	m_pSettingsManager = a_pSettingsManager;
	m_pSyntaxHighlighter->setSettingsManager(a_pSettingsManager);
}

// END OF void ScriptEditor::setSettingsManager(
//		SettingsManager * a_pSettingsManager)
//==============================================================================

void ScriptEditor::setSettingsDialog(SettingsDialog * a_pSettingsDialog)
{
	connect(a_pSettingsDialog, SIGNAL(signalSettingsChanged()),
		this, SLOT(slotLoadSettings()));
	connect(a_pSettingsDialog, SIGNAL(signalSettingsChanged()),
		m_pSyntaxHighlighter, SLOT(slotLoadSettings()));
}

// END OF void ScriptEditor::setSettingsDialog(
//		SettingsManager * a_pSettingsDialog)
//==============================================================================

void ScriptEditor::slotLoadSettings()
{
	if(!m_pSettingsManager)
		return;

	m_commonScriptTextFormat = m_pSettingsManager->getTextFormat(
		TEXT_FORMAT_ID_COMMON_SCRIPT_TEXT);
	QFont commonScriptTextFont = m_commonScriptTextFormat.font();
	document()->setDefaultFont(commonScriptTextFont);

	m_tabText = m_pSettingsManager->getTabText();
	m_spacesInTab = m_pSettingsManager->getSpacesInTab();
	QFontMetrics metrics(commonScriptTextFont);
	setTabStopWidth(metrics.width(' ') * m_spacesInTab);

	m_backgroundColor = m_pSettingsManager->getColor(COLOR_ID_TEXT_BACKGROUND);
	QColor textColor = m_commonScriptTextFormat.foreground().color();
	QPalette newPalette = palette();
	newPalette.setColor(QPalette::Base, m_backgroundColor);
	newPalette.setColor(QPalette::Text, textColor);
	setPalette(newPalette);

	m_activeLineColor = m_pSettingsManager->getColor(COLOR_ID_ACTIVE_LINE);

	slotUpdateSideBoxWidth();

	update();
}

// END OF void ScriptEditor::slotLoadSettings()
//==============================================================================

void ScriptEditor::slotComplete()
{
	QTextCursor currentCursor = textCursor();
	QString lineString = currentCursor.block().text();
	int cursorIndex = currentCursor.positionInBlock();
	int wordStart = cursorIndex;
	while((wordStart > 0) && (lineString[wordStart - 1].isLetterOrNumber() ||
		(QString("._").contains(lineString[wordStart - 1]))))
		wordStart--;
	m_typedCharacters = cursorIndex - wordStart;
	QString typedWord = lineString.mid(wordStart, m_typedCharacters);
	int charactersAfterDot = m_typedCharacters - typedWord.lastIndexOf('.') - 1;

	QAction * pAction = qobject_cast<QAction *>(sender());

	if((charactersAfterDot < m_charactersTypedToStartCompletion) &&
		(pAction == nullptr))
	{
		m_pCompleter->popup()->hide();
		return;
	}

	m_pCompleter->setCompletionPrefix(typedWord);
	m_pCompleter->popup()->setCurrentIndex(
		m_pCompleter->completionModel()->index(0, 0));
	QRect cursorRectangle = cursorRect();
	cursorRectangle.setWidth(m_pCompleter->popup()->sizeHintForColumn(0)
		+ m_pCompleter->popup()->verticalScrollBar()->sizeHint().width());

	m_pCompleter->complete(cursorRectangle);
}

// END OF void ScriptEditor::slotComplete()
//==============================================================================

void ScriptEditor::slotInsertCompletion(const QString & a_completionString)
{
	QTextCursor currentCursor = textCursor();
	currentCursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor,
		m_typedCharacters);
	currentCursor.deleteChar();
	setTextCursor(currentCursor);
	insertPlainText(a_completionString);
}

// END OF void ScriptEditor::slotInsertCompletion(
//		const QString & a_completionString)
//==============================================================================

void ScriptEditor::slotDuplicateSelection()
{
	QTextCursor cursor = textCursor();
	int l_selectionStart = cursor.selectionStart();
	int l_selectionEnd = cursor.selectionEnd();

	QString newText = cursor.selectedText();
	if(cursor.hasSelection())
	{
		cursor.clearSelection();
	}
	else
	{
		newText = cursor.block().text() + QString("\n");
		cursor.movePosition(QTextCursor::StartOfBlock);
	}
	cursor.insertText(newText);

	cursor.setPosition(l_selectionStart, QTextCursor::MoveAnchor);
	cursor.setPosition(l_selectionEnd, QTextCursor::KeepAnchor);
	setTextCursor(cursor);
}

// END OF void ScriptEditor::slotDuplicateSelection()
//==============================================================================

void ScriptEditor::slotCommentSelection()
{
	insertSelectedLinesBegin("#");
}

// END OF void ScriptEditor::slotCommentSelection()
//==============================================================================

void ScriptEditor::slotUncommentSelection()
{
	removeSelectedLinesBegin("#");
}

// END OF void ScriptEditor::slotUncommentSelection()
//==============================================================================

void ScriptEditor::slotTab()
{
	QTextCursor cursor = textCursor();
	if(cursor.hasSelection())
		insertSelectedLinesBegin(m_tabText);
	else
		cursor.insertText(m_tabText);
}

// END OF void ScriptEditor::slotTab()
//==============================================================================

void ScriptEditor::slotBackTab()
{
	QTextCursor cursor = textCursor();
	QTextDocument * pDocument = document();
	QTextBlock firstBlock = pDocument->findBlock(cursor.selectionStart());
	QTextBlock lastBlock = pDocument->findBlock(cursor.selectionEnd());
	int firstBlockNumber = firstBlock.blockNumber();
	int lastBlockNumber = lastBlock.blockNumber();
	cursor.beginEditBlock();
	for(int i = firstBlockNumber; i <= lastBlockNumber; ++i)
	{
		QTextBlock block = pDocument->findBlockByNumber(i);
		int position = block.position();
		cursor.setPosition(position);

		// If line begins with set tabulation text - remove it.
		cursor.setPosition(position + m_tabText.length(),
			QTextCursor::KeepAnchor);
		if(cursor.selectedText() == m_tabText)
		{
			cursor.removeSelectedText();
			continue;
		}

		// Else remove standard tabulation character.
		cursor.setPosition(position + 1, QTextCursor::KeepAnchor);
		if(cursor.selectedText() == "\t")
		{
			cursor.removeSelectedText();
			continue;
		}

		// Else remove set number of space characters used for tabulation.
		int tokenLength = 0;
		int spacesInTab = std::max(1, m_spacesInTab);
		while((tokenLength < spacesInTab) &&
			(pDocument->characterAt(position + tokenLength) == ' '))
			tokenLength++;
		cursor.setPosition(position + tokenLength,
			QTextCursor::KeepAnchor);
		cursor.removeSelectedText();
	}
	cursor.endEditBlock();
}

// END OF void ScriptEditor::slotBackTab()
//==============================================================================


bool ScriptEditor::eventFilter(QObject * a_pObject, QEvent * a_pEvent)
{
	if((a_pObject == m_pSideBox) && (a_pEvent->type() == QEvent::Paint))
	{
		QPaintEvent * pPaintEvent = static_cast<QPaintEvent *>(a_pEvent);
		paintSideBox(pPaintEvent);
		return true;
	}

	return QPlainTextEdit::eventFilter(a_pObject, a_pEvent);
}

// END OF bool ScriptEditor::eventFilter(QObject * a_pObject, QEvent * a_pEvent)
//==============================================================================

void ScriptEditor::resizeEvent(QResizeEvent * a_pEvent)
{
	QPlainTextEdit::resizeEvent(a_pEvent);

	QRect cr = contentsRect();
	m_pSideBox->setGeometry(QRect(cr.left(), cr.top(), sideBoxWidth(),
		cr.height()));
	slotHighlightCurrentBlock();
}

// END OF void ScriptEditor::resizeEvent(QResizeEvent * a_pEvent)
//==============================================================================

void ScriptEditor::keyPressEvent(QKeyEvent * a_pEvent)
{
	int key = a_pEvent->key();
	Qt::KeyboardModifiers modifiers = a_pEvent->modifiers();

	if(m_pCompleter->popup()->isVisible())
	{
		// The following keys are forwarded by the completer to the widget
		switch(key)
		{
			case Qt::Key_Enter:
			case Qt::Key_Return:
			case Qt::Key_Escape:
			case Qt::Key_Tab:
			case Qt::Key_Backtab:
				a_pEvent->ignore();
				return; // let the completer do default behavior
			case Qt::Key_Left:
			case Qt::Key_Right:
				m_pCompleter->popup()->hide();
				break;
			default:
				break;
		}
	}

	if((key == Qt::Key_Tab) && (modifiers == Qt::NoModifier))
	{
		slotTab();
		return;
	}

	if((key == Qt::Key_Backtab) ||
		((key == Qt::Key_Tab) && (modifiers == Qt::ShiftModifier)))
	{
		slotBackTab();
		return;
	}

	QString textBefore = toPlainText();

	QPlainTextEdit::keyPressEvent(a_pEvent);

	if(((key == Qt::Key_Return) && (modifiers == Qt::NoModifier)) ||
		((key == Qt::Key_Enter) && (modifiers == Qt::KeypadModifier)))
		indentNewLine();

	if(textBefore != toPlainText())
		slotComplete();
}

// END OF void ScriptEditor::keyPressEvent(QKeyEvent * a_pEvent)
//==============================================================================

void ScriptEditor::slotTextChanged()
{
	QString newPlainText = toPlainText();
	if(m_plainText == newPlainText)
		return;

	m_plainText = newPlainText;
	QString vsCoreName = getVapourSynthCoreName();
	setChildrenCoreName(vsCoreName);
}

// END OF void ScriptEditor::slotTextChanged()
//==============================================================================

void ScriptEditor::slotUpdateSideBoxWidth()
{
	setViewportMargins(sideBoxWidth(), 0, 0, 0);
}

// END OF void ScriptEditor::slotUpdateSideBoxWidth()
//==============================================================================

void ScriptEditor::slotUpdateSideBox(const QRect & a_rect, int a_dy)
{
	if(a_dy)
		m_pSideBox->scroll(0, a_dy);
	else
		m_pSideBox->update(0, a_rect.y(), m_pSideBox->width(), a_rect.height());
}

// END OF void ScriptEditor::slotUpdateSideBox(const QRect & a_rect, int a_dy)
//==============================================================================

void ScriptEditor::slotHighlightCurrentBlock()
{
	QList<QTextEdit::ExtraSelection> extraTextSelections;
	QTextCursor newCursor = textCursor();
	newCursor.movePosition(QTextCursor::StartOfBlock);
	int linesInBlock = newCursor.block().lineCount();
	for(int i = 0; i < linesInBlock; ++i)
	{
		QTextEdit::ExtraSelection selection;
		selection.format.setBackground(m_activeLineColor);
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);
		selection.cursor = newCursor;
		selection.cursor.clearSelection();
		extraTextSelections.append(selection);
		newCursor.movePosition(QTextCursor::EndOfLine);
		newCursor.movePosition(QTextCursor::NextCharacter);
	}

	setExtraSelections(extraTextSelections);
}

// END OF void ScriptEditor::slotHighlightCurrentBlock()
//==============================================================================

QString ScriptEditor::getVapourSynthCoreName()
{
	QString vapourSynthName;
	QString vsCoreName;
	int blocksCount = document()->blockCount();

	// Search for VapourSynth object.
	// Usually looks like: import vapoursynth as vs
	int vsImportBlock = -1;
	QString searchString("import vapoursynth");
	for(int k = 0; k < blocksCount; ++k)
	{
		QString simplifiedText =
			document()->findBlockByNumber(k).text().simplified();
		int i = simplifiedText.indexOf(searchString);
		if(i < 0)
			continue;

		vapourSynthName = "vapoursynth";
		i += searchString.length();
		if(simplifiedText.mid(i, 4) == " as ")
		{
			i += 4;
			if(!(simplifiedText[i].isLetter() || (simplifiedText[i] == '_')))
				continue;

			int j = i;
			for(; j < simplifiedText.length(); ++j)
			{
				if(!(simplifiedText[j].isLetterOrNumber() ||
					(simplifiedText[j] == '_')))
					break;
			}
			vapourSynthName = simplifiedText.mid(i, j - i);
		}

		vsImportBlock = k;
		break;
	}

	if((vsImportBlock < 0) || (vsImportBlock == (blocksCount - 1)))
		return QString();

	// Search for VapourSynth core creation
	// Usually looks like: core = vs.get_core()
	searchString = QString("%1.get_core").arg(vapourSynthName);
	for(int k = vsImportBlock + 1; k < blocksCount; ++k)
	{
		QString simplifiedText =
			document()->findBlockByNumber(k).text().simplified();
		int i = simplifiedText.indexOf(searchString);
		if(i < 0)
			continue;

		i--;
		if(simplifiedText[i].isSpace())
			i--;
		if(simplifiedText[i] != '=')
			break;
		i--;
		if(simplifiedText[i].isSpace())
			i--;
		int j = i + 1;
		for(; i >= 0; --i)
		{
			if(!(simplifiedText[i].isLetterOrNumber() ||
				(simplifiedText[i] == '_')))
				break;
		}
		i++;
		if(simplifiedText[i].isDigit())
			break;

		vsCoreName = simplifiedText.mid(i, j - i);
		return vsCoreName;
	}

	return QString();
}

// END OF QString ScriptEditor::getVapourSynthCoreName()
//==============================================================================

void ScriptEditor::setChildrenCoreName(const QString & a_coreName)
{
	m_pCompleterModel->setCoreName(a_coreName);
	m_pSyntaxHighlighter->setCoreName(a_coreName);
}

// END OF void ScriptEditor::setChildrenCoreName(const QString & a_coreName)
//==============================================================================

int ScriptEditor::sideBoxWidth() const
{
	QString controlString("9");
	int max = std::max(1, blockCount());
	while(max >= 10)
	{
		max /= 10;
		controlString += "9";
	}

	QFont commonTextFont = m_commonScriptTextFormat.font();
	QFontMetrics metrics(commonTextFont);
	int space = metrics.width(controlString);
	space += m_sideBoxTextMargin * 2;
	space += m_sideBoxLineWidth;

	return space;
}

// END OF int ScriptEditor::sideBoxWidth()
//==============================================================================

void ScriptEditor::paintSideBox(QPaintEvent * a_pEvent)
{
	QPainter painter(m_pSideBox);
	painter.fillRect(a_pEvent->rect(), m_backgroundColor);

	// Draw border line between sidebox and text.
	QRect borderLineRect = a_pEvent->rect();
	borderLineRect.setLeft(borderLineRect.right() - m_sideBoxLineWidth + 1);
	painter.fillRect(borderLineRect, m_activeLineColor);

	// Draw visible lines numbers.

	painter.setPen(m_commonScriptTextFormat.foreground().color());
	QFont commonTextFont = m_commonScriptTextFormat.font();
	QFontMetrics metrics(commonTextFont);
	int labelHeight = metrics.height();
	painter.setFont(commonTextFont);

	int lineNumberWidth = m_pSideBox->width() - m_sideBoxLineWidth -
		m_sideBoxTextMargin * 2;
	QPointF offset = contentOffset();
	int viewportHeight = viewport()->rect().height();
	qreal blockTop;
	for(QTextBlock textBlock = firstVisibleBlock();
		textBlock.isValid() &&
		((blockTop = blockBoundingGeometry(textBlock).translated(offset).top())
		< viewportHeight);
		textBlock = textBlock.next())
	{
		if(!textBlock.isVisible())
			continue;
		QString number = QString::number(textBlock.blockNumber() + 1);
		painter.drawText(m_sideBoxTextMargin, blockTop, lineNumberWidth,
			labelHeight, Qt::AlignRight, number);
	}
}

// END OF void ScriptEditor::paintSideBox(QPaintEvent * a_pEvent)
//==============================================================================

void ScriptEditor::indentNewLine()
{
	QTextCursor currentCursor = textCursor();
	QTextBlock currentBlock = currentCursor.block();
	int blockNumber = currentBlock.blockNumber();
	assert(blockNumber != 0);
	QTextBlock previousBlock =
		document()->findBlockByNumber(blockNumber - 1);
	QString blockText = previousBlock.text();
	int blockLength = blockText.length();
	QString indentation;
	for(int i = 0; i < blockLength; ++i)
	{
		if(!blockText[i].isSpace())
			break;
		indentation += blockText[i];
	}
	currentCursor.insertText(indentation);
}

// END OF void ScriptEditor::indentNewLine()
//==============================================================================

void ScriptEditor::insertSelectedLinesBegin(const QString & a_text)
{
	QTextCursor cursor = textCursor();
	QTextDocument * pDocument = document();
	QTextBlock firstBlock = pDocument->findBlock(cursor.selectionStart());
	QTextBlock lastBlock = pDocument->findBlock(cursor.selectionEnd());
	int firstBlockNumber = firstBlock.blockNumber();
	int lastBlockNumber = lastBlock.blockNumber();
	cursor.beginEditBlock();
	for(int i = firstBlockNumber; i <= lastBlockNumber; ++i)
	{
		QTextBlock block = pDocument->findBlockByNumber(i);
		int position = block.position();
		cursor.setPosition(position);
		cursor.insertText(a_text);
	}
	cursor.endEditBlock();
}

// END OF void ScriptEditor::insertSelectedLinesBegin(const QString & a_text)
//==============================================================================

void ScriptEditor::removeSelectedLinesBegin(const QString & a_text)
{
	QTextCursor cursor = textCursor();
	QTextDocument * pDocument = document();
	QTextBlock firstBlock = pDocument->findBlock(cursor.selectionStart());
	QTextBlock lastBlock = pDocument->findBlock(cursor.selectionEnd());
	int firstBlockNumber = firstBlock.blockNumber();
	int lastBlockNumber = lastBlock.blockNumber();
	int tokenLength = a_text.length();
	cursor.beginEditBlock();
	for(int i = firstBlockNumber; i <= lastBlockNumber; ++i)
	{
		QTextBlock block = pDocument->findBlockByNumber(i);
		int position = block.position();
		cursor.setPosition(position);
		cursor.setPosition(position + tokenLength, QTextCursor::KeepAnchor);
		if(cursor.selectedText() == a_text)
			cursor.removeSelectedText();
	}
	cursor.endEditBlock();
}

// END OF void ScriptEditor::removeSelectedLinesBegin(const QString & a_text)
//==============================================================================
