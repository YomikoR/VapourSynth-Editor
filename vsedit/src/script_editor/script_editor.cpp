#include "script_editor.h"

#include "script_completer_model.h"
#include "script_completer.h"
#include "syntax_highlighter.h"
#include "../../../common-src/settings/settings_manager.h"
#include "../settings/settings_dialog.h"

#include <QTextBlock>
#include <QCursor>
#include <QCompleter>
#include <QKeyEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QPaintEvent>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QPainter>
#include <QAction>
#include <QFileInfo>
#include <QDir>
#include <QMenu>
#include <algorithm>

//==============================================================================

const char COMMENT_TOKEN[] = "#";

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
	, m_highlightSelectionMatches(DEFAULT_HIGHLIGHT_SELECTION_MATCHES)
	, m_highlightSelectionMatchesMinLength(
		DEFAULT_HIGHLIGHT_SELECTION_MATCHES_MIN_LENGTH)
	, m_commonScriptTextFormat()
	, m_tabText("\t")
	, m_spacesInTab(DEFAULT_SPACES_IN_TAB)
	, m_pContextMenu(nullptr)
	, m_pActionDuplicateSelection(nullptr)
	, m_pActionCommentSelection(nullptr)
	, m_pActionUncommentSelection(nullptr)
	, m_pActionReplaceTabWithSpaces(nullptr)
	, m_pActionAutocomplete(nullptr)
	, m_pActionMoveTextBlockUp(nullptr)
	, m_pActionMoveTextBlockDown(nullptr)
	, m_pActionToggleComment(nullptr)
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

	fillVariables();

	connect(m_pCompleter, SIGNAL(activated(const QString &)),
		this, SLOT(slotInsertCompletion(const QString &)));
	connect(this, SIGNAL(textChanged()), this, SLOT(slotTextChanged()));
	connect(this, SIGNAL(blockCountChanged(int)),
		this, SLOT(slotUpdateSideBoxWidth()));
	connect(this, SIGNAL(updateRequest(const QRect &, int)),
		this, SLOT(slotUpdateSideBox(const QRect &, int)));
	connect(this, SIGNAL(cursorPositionChanged()),
		this, SLOT(slotHighlightCurrentBlockAndMatches()));
	connect(this, SIGNAL(selectionChanged()),
		this, SLOT(slotHighlightCurrentBlockAndMatches()));
	connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
		this, SLOT(slotShowCustomMenu(const QPoint &)));

	setContextMenuPolicy(Qt::CustomContextMenu);
}

// END OF ScriptEditor::ScriptEditor(QWidget * a_pParent)
//==============================================================================

ScriptEditor::~ScriptEditor()
{

}

// END OF ScriptEditor::~ScriptEditor()
//==============================================================================

QString ScriptEditor::text() const
{
	return document()->toPlainText();
}

// END OF QString ScriptEditor::text() const
//==============================================================================

QPoint ScriptEditor::cursorPosition() const
{
	QTextCursor currentCursor = textCursor();
	int line = currentCursor.blockNumber();
	int index = currentCursor.anchor() - currentCursor.position();

	return QPoint(line, index);
}

// END OF QPoint ScriptEditor::cursorPosition() const
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

bool ScriptEditor::isModified() const
{
	return document()->isModified();
}

// END OF bool ScriptEditor::isModified() const
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
	update();
}

// END OF void ScriptEditor::setPluginsList(const VSPluginsList & a_pluginsList)
//==============================================================================

void ScriptEditor::setSettingsManager(SettingsManager * a_pSettingsManager)
{
	m_pSettingsManager = a_pSettingsManager;
	m_pSyntaxHighlighter->setSettingsManager(a_pSettingsManager);
	createActionsAndMenus();
	slotLoadSettings();
}

// END OF void ScriptEditor::setSettingsManager(
//		SettingsManager * a_pSettingsManager)
//==============================================================================

std::vector<QAction *> ScriptEditor::actionsForMenu() const
{
	return {m_pActionDuplicateSelection, m_pActionCommentSelection,
		m_pActionUncommentSelection, m_pActionReplaceTabWithSpaces,
		m_pActionMoveTextBlockUp, m_pActionMoveTextBlockDown,
		m_pActionToggleComment};
}

// END OF std::vector<QAction *> ScriptEditor::actionsForMenu() const
//==============================================================================

std::vector<vsedit::VariableToken> ScriptEditor::variables() const
{
	std::vector<vsedit::VariableToken> cleanVariables = m_variables;
	for(vsedit::VariableToken & variable : cleanVariables)
		variable.evaluate = nullptr;
	return cleanVariables;
}

// END OF std::vector<QAction *> ScriptEditor::actionsForMenu() const
//==============================================================================

void ScriptEditor::slotLoadSettings()
{
	if(!m_pSettingsManager)
		return;

	setUpdatesEnabled(false);

	m_pSyntaxHighlighter->slotLoadSettings();

	m_charactersTypedToStartCompletion =
		m_pSettingsManager->getCharactersTypedToStartCompletion();

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

	QString sheet = QString("QFrame {color: %1; background-color: %2;}")
		.arg(textColor.name()).arg(m_backgroundColor.name());
	setStyleSheet(sheet);

	m_activeLineColor = m_pSettingsManager->getColor(COLOR_ID_ACTIVE_LINE);
	m_selectionMatchesColor =
		m_pSettingsManager->getColor(COLOR_ID_SELECTION_MATCHES);
	m_highlightSelectionMatches =
		m_pSettingsManager->getHighlightSelectionMatches();
	m_highlightSelectionMatchesMinLength =
		m_pSettingsManager->getHighlightSelectionMatchesMinLength();

	QKeySequence hotkey;
	for(QAction * pAction : m_settableActionsList)
	{
		hotkey = m_pSettingsManager->getHotkey(pAction->data().toString());
		pAction->setShortcut(hotkey);
	}

	slotUpdateSideBoxWidth();
	slotHighlightCurrentBlockAndMatches();

	setUpdatesEnabled(true);
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
	insertSelectedLinesBegin(COMMENT_TOKEN);
}

// END OF void ScriptEditor::slotCommentSelection()
//==============================================================================

void ScriptEditor::slotUncommentSelection()
{
	removeSelectedLinesBegin(COMMENT_TOKEN);
}

// END OF void ScriptEditor::slotUncommentSelection()
//==============================================================================

void ScriptEditor::slotReplaceTabWithSpaces()
{
	const QString spaces(m_spacesInTab, ' ');
	QTextDocument * pDocument = document();
	QTextCursor cursor(pDocument);
	cursor.beginEditBlock();
	QTextCursor newCursor = cursor;
	while(!newCursor.isNull())
	{
		newCursor = pDocument->find("\t", newCursor);
		if(newCursor.hasSelection())
			newCursor.insertText(spaces);
	}
	cursor.endEditBlock();
}

// END OF void ScriptEditor::slotReplaceTabWithSpaces()
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
		cursor.setPosition(std::min(position + m_tabText.length(),
			pDocument->characterCount() - 1), QTextCursor::KeepAnchor);
		if(cursor.selectedText() == m_tabText)
		{
			cursor.removeSelectedText();
			continue;
		}

		// Else remove standard tabulation character.
		cursor.setPosition(std::min(position + 1,
			pDocument->characterCount() - 1), QTextCursor::KeepAnchor);
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

void ScriptEditor::slotHome(bool a_select)
{
	QTextCursor cursor = textCursor();
	QTextDocument * pDocument = document();
	QTextBlock firstBlock = pDocument->findBlock(cursor.selectionStart());
	int blockLength = firstBlock.text().length();
	if(blockLength == 0)
		return;
	int position = firstBlock.position();
	int endPosition = cursor.selectionEnd();
	for(int i = 0; i < blockLength; ++i)
	{
		QChar character = pDocument->characterAt(position);
		if(!character.isSpace())
			break;
		position++;
	}
	cursor.setPosition(position);
	if(a_select)
		cursor.setPosition(endPosition, QTextCursor::KeepAnchor);
	setTextCursor(cursor);
}

// END OF void ScriptEditor::slotHome(bool a_select)
//==============================================================================

void ScriptEditor::slotInsertTextAtNewLine(const QString & a_text)
{
	QTextCursor cursor = textCursor();
	QTextDocument * pDocument = document();
	QTextBlock lastBlock = pDocument->findBlock(cursor.selectionEnd());
	cursor.setPosition(lastBlock.position() + lastBlock.text().length());
	setTextCursor(cursor);
	insertPlainText(QString("\n") + a_text);
}

// END OF void ScriptEditor::slotInsertTextAtNewLine(const QString & a_text)
//==============================================================================

void ScriptEditor::slotMoveTextBlockUp()
{
	QTextCursor cursor = textCursor();
	QTextDocument * pDocument = document();
	QTextBlock firstBlock = pDocument->findBlock(cursor.selectionStart());
	QTextBlock lastBlock = pDocument->findBlock(cursor.selectionEnd());
	int firstBlockNumber = firstBlock.blockNumber();
	int lastBlockNumber = lastBlock.blockNumber();

	if(firstBlockNumber == 0)
		return;

	cursor.beginEditBlock();

	QTextBlock block = pDocument->findBlockByNumber(firstBlockNumber - 1);
	QString blockText = block.text();
	cursor.setPosition(block.position());
	cursor.setPosition(firstBlock.position(), QTextCursor::KeepAnchor);
	cursor.removeSelectedText();

	block = pDocument->findBlockByNumber(lastBlockNumber);
	if(block.blockNumber() == lastBlockNumber)
	{
		cursor.setPosition(block.position());
		cursor.insertText(blockText + QString("\n"));
	}
	else
	{
		cursor.movePosition(QTextCursor::End);
		cursor.insertText(QString("\n") + blockText);
	}

	cursor.endEditBlock();

	// Workaround for weird behavior when selection is snapped to the end
	// of the document
	cursor = textCursor();
	if(cursor.selectionEnd() < pDocument->lastBlock().position())
		return;
	lastBlockNumber--;
	lastBlock = pDocument->findBlockByNumber(lastBlockNumber);
	cursor.setPosition(cursor.selectionStart());
	cursor.setPosition(lastBlock.position() + lastBlock.length() - 1,
		QTextCursor::KeepAnchor);
	setTextCursor(cursor);
}

// END OF void ScriptEditor::slotMoveTextBlockUp()
//==============================================================================

void ScriptEditor::slotMoveTextBlockDown()
{
	QTextCursor cursor = textCursor();
	QTextDocument * pDocument = document();
	QTextBlock firstBlock = pDocument->findBlock(cursor.selectionStart());
	QTextBlock lastBlock = pDocument->findBlock(cursor.selectionEnd());
	int lastBlockNumber = lastBlock.blockNumber();

	if((lastBlockNumber + 1) == pDocument->blockCount())
		return;

	cursor.beginEditBlock();

	QTextBlock block = lastBlock.next();
	QString blockText = block.text();
	if((block.blockNumber() + 1) == pDocument->blockCount())
	{
		cursor.setPosition(lastBlock.position() + lastBlock.length() - 1);
		cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
	}
	else
	{
		cursor.setPosition(block.position());
		cursor.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
	}
	cursor.removeSelectedText();

	cursor.setPosition(firstBlock.position());
	cursor.insertText(blockText + QString("\n"));

	cursor.endEditBlock();
}

// END OF void ScriptEditor::slotMoveTextBlockDown()
//==============================================================================

void ScriptEditor::slotToggleComment()
{
	QTextCursor cursor = textCursor();
	QTextDocument * pDocument = document();
	QTextBlock firstBlock = pDocument->findBlock(cursor.selectionStart());
	QTextBlock lastBlock = pDocument->findBlock(cursor.selectionEnd());
	int firstBlockNumber = firstBlock.blockNumber();
	int lastBlockNumber = lastBlock.blockNumber();
	QString token(COMMENT_TOKEN);
	int tokenLength = token.length();
	bool allCommented = true;
	cursor.beginEditBlock();

	for(int i = firstBlockNumber; i <= lastBlockNumber; ++i)
	{
		QTextBlock block = pDocument->findBlockByNumber(i);
		int position = block.position();
		cursor.setPosition(position);
		cursor.setPosition(std::min(position + tokenLength,
			pDocument->characterCount() - 1), QTextCursor::KeepAnchor);
		if(cursor.selectedText() != token)
		{
			allCommented = false;
			break;
		}
	}

	for(int i = firstBlockNumber; i <= lastBlockNumber; ++i)
	{
		QTextBlock block = pDocument->findBlockByNumber(i);
		int position = block.position();
		cursor.setPosition(position);
		cursor.setPosition(std::min(position + tokenLength,
			pDocument->characterCount() - 1), QTextCursor::KeepAnchor);
		if(allCommented)
			cursor.removeSelectedText();
		else if(cursor.selectedText() != token)
		{
			cursor.setPosition(position);
			cursor.insertText(token);
		}
	}

	cursor.endEditBlock();
}

// END OF void ScriptEditor::slotToggleComment()
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
	slotHighlightCurrentBlockAndMatches();
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

	if((key == Qt::Key_Home) &&
		((modifiers == Qt::NoModifier) || (modifiers == Qt::ShiftModifier)))
	{
		bool select = (modifiers == Qt::ShiftModifier);
		slotHome(select);
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

void ScriptEditor::dragEnterEvent(QDragEnterEvent * a_pEvent)
{
	if(!a_pEvent->mimeData()->hasUrls())
	{
		QPlainTextEdit::dragEnterEvent(a_pEvent);
		return;
	}

	QList<QUrl> urls = a_pEvent->mimeData()->urls();
	for(const QUrl & url : urls)
	{
		if(!url.isLocalFile())
		{
			a_pEvent->ignore();
			return;
		}
	}

	a_pEvent->acceptProposedAction();
}

// END OF void ScriptEditor::dragEnterEvent(QDragEnterEvent * a_pEvent)
//==============================================================================

void ScriptEditor::dragMoveEvent(QDragMoveEvent * a_pEvent)
{
	if(!a_pEvent->mimeData()->hasUrls())
	{
		QPlainTextEdit::dragMoveEvent(a_pEvent);
		return;
	}

	QTextCursor cursor = cursorForPosition(a_pEvent->pos());
	setTextCursor(cursor);
	a_pEvent->acceptProposedAction();
}

// END OF void ScriptEditor::dragMoveEvent(QDragMoveEvent * a_pEvent)
//==============================================================================

void ScriptEditor::dropEvent(QDropEvent * a_pEvent)
{
	if(!a_pEvent->mimeData()->hasUrls())
	{
		QPlainTextEdit::dropEvent(a_pEvent);
		return;
	}

	QList<QUrl> urls = a_pEvent->mimeData()->urls();
	Q_ASSERT(urls.size() > 0);

	if(urls.size() == 1)
	{
		QString filePath = urls[0].toLocalFile();
		QRegExp matcher;
		matcher.setPatternSyntax(QRegExp::Wildcard);
		matcher.setCaseSensitivity(Qt::CaseInsensitive);
		matcher.setPattern("*.vpy");
		if(matcher.exactMatch(filePath))
		{
			bool handled = false;
			emit signalScriptFileDropped(filePath, &handled);
			if(handled)
			{
				a_pEvent->acceptProposedAction();
				return;
			}
		}
	}

	QStringList textList;
	for(m_droppedFileNumber = 0; m_droppedFileNumber < urls.size();
		++m_droppedFileNumber)
	{
		m_droppedFilePath =
			QDir::cleanPath(urls[m_droppedFileNumber].toLocalFile());
		m_droppedFilePath = QDir::toNativeSeparators(m_droppedFilePath);
		QString sourceTemplate =
			m_pSettingsManager->getDropFileTemplate(m_droppedFilePath);
		for(const vsedit::VariableToken & variable : m_variables)
			sourceTemplate = sourceTemplate.replace(variable.token,
				variable.evaluate());
		textList += sourceTemplate;
	}
	slotInsertTextAtNewLine(textList.join("\n"));

	a_pEvent->acceptProposedAction();
}

// END OF void ScriptEditor::dropEvent(QDropEvent * a_pEvent)
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

void ScriptEditor::slotHighlightCurrentBlockAndMatches()
{
	QTextDocument * pDocument = document();
	QList<QTextEdit::ExtraSelection> extraTextSelections;
	QTextCursor cursor = textCursor();

	QString selectedText = cursor.selectedText();

	cursor.movePosition(QTextCursor::StartOfBlock);
	int linesInBlock = cursor.block().lineCount();
	for(int i = 0; i < linesInBlock; ++i)
	{
		QTextEdit::ExtraSelection selection;
		selection.format.setBackground(m_activeLineColor);
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);
		selection.cursor = cursor;
		selection.cursor.clearSelection();
		extraTextSelections.append(selection);
		cursor.movePosition(QTextCursor::EndOfLine);
		cursor.movePosition(QTextCursor::NextCharacter);
	}

	if(m_highlightSelectionMatches &&
		(selectedText.length() >= m_highlightSelectionMatchesMinLength))
	{
		cursor = QTextCursor(pDocument);
		while(true)
		{
			cursor = pDocument->find(selectedText, cursor);
			if(cursor.isNull())
				break;
			QTextEdit::ExtraSelection selection;
			selection.format.setBackground(m_selectionMatchesColor);
			selection.cursor = cursor;
			extraTextSelections.append(selection);
		}
	}

	setExtraSelections(extraTextSelections);
}

// END OF void ScriptEditor::slotHighlightCurrentBlockAndMatches()
//==============================================================================

void ScriptEditor::slotShowCustomMenu(const QPoint & a_position)
{
	if(m_pContextMenu)
		delete m_pContextMenu;

	m_pContextMenu = createStandardContextMenu();

	if(m_pSettingsManager)
	{
		m_pContextMenu->addSeparator();
		std::vector<QAction *> actionsList = actionsForMenu();
		for(QAction * pAction : actionsList)
			m_pContextMenu->addAction(pAction);
	}

	QPoint globalPosition = mapToGlobal(a_position);
    m_pContextMenu->popup(globalPosition);
}

// END OF void ScriptEditor::slotShowCustomMenu(const QPoint & a_position)
//==============================================================================

void ScriptEditor::createActionsAndMenus()
{
	if(!m_pSettingsManager)
		return;

	struct ActionToCreate
	{
		QAction ** ppAction;
		const char * id;
		const char * slotToConnect;
	};

	ActionToCreate actionsToCreate[] =
	{
		{&m_pActionDuplicateSelection, ACTION_ID_DUPLICATE_SELECTION,
			SLOT(slotDuplicateSelection())},
		{&m_pActionCommentSelection, ACTION_ID_COMMENT_SELECTION,
			SLOT(slotCommentSelection())},
		{&m_pActionUncommentSelection, ACTION_ID_UNCOMMENT_SELECTION,
			SLOT(slotUncommentSelection())},
		{&m_pActionReplaceTabWithSpaces, ACTION_ID_REPLACE_TAB_WITH_SPACES,
			SLOT(slotReplaceTabWithSpaces())},
		{&m_pActionAutocomplete, ACTION_ID_AUTOCOMPLETE,
			SLOT(slotComplete())},
		{&m_pActionMoveTextBlockUp, ACTION_ID_MOVE_TEXT_BLOCK_UP,
			SLOT(slotMoveTextBlockUp())},
		{&m_pActionMoveTextBlockDown, ACTION_ID_MOVE_TEXT_BLOCK_DOWN,
			SLOT(slotMoveTextBlockDown())},
		{&m_pActionToggleComment, ACTION_ID_TOGGLE_COMMENT,
			SLOT(slotToggleComment())},
	};

	for(ActionToCreate & item : actionsToCreate)
	{
		if(*item.ppAction)
			continue;

		QAction * pAction = m_pSettingsManager->createStandardAction(
			item.id, this);
		*item.ppAction = pAction;
		addAction(pAction);
		m_settableActionsList.push_back(pAction);
		connect(pAction, SIGNAL(triggered()), this, item.slotToConnect);
	}
}

// END OF void ScriptEditor::createActionsAndMenus()
//==============================================================================

QString ScriptEditor::getVapourSynthCoreName() const
{
	QString vapourSynthName;
	QString vsCoreName = "core";
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
		return vsCoreName;

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

	return vsCoreName;
}

// END OF QString ScriptEditor::getVapourSynthCoreName() const
//==============================================================================

void ScriptEditor::setChildrenCoreName(const QString & a_coreName)
{
	m_pCompleterModel->setCoreName(a_coreName);
	m_pSyntaxHighlighter->setCoreName(a_coreName);
}

// END OF void ScriptEditor::setChildrenCoreName(const QString & a_coreName)
//		const
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

// END OF int ScriptEditor::sideBoxWidth() const
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
	Q_ASSERT(blockNumber != 0);
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
		cursor.setPosition(std::min(position + tokenLength,
			pDocument->characterCount() - 1), QTextCursor::KeepAnchor);
		if(cursor.selectedText() == a_text)
			cursor.removeSelectedText();
	}
	cursor.endEditBlock();
}

// END OF void ScriptEditor::removeSelectedLinesBegin(const QString & a_text)
//==============================================================================

void ScriptEditor::fillVariables()
{
	m_variables =
	{
		{"{f}", tr("file path"),
			[&]()
			{
				return m_droppedFilePath;
			}
		},

		{"{d}", tr("file directory"),
			[&]()
			{
				QFileInfo file(m_droppedFilePath);
				return QDir::toNativeSeparators(file.path());
			}
		},

		{"{n}", tr("file name"),
			[&]()
			{
				QFileInfo file(m_droppedFilePath);
				return file.completeBaseName();
			}
		},

		{"{x}", tr("file extension"),
			[&]()
			{
				QFileInfo file(m_droppedFilePath);
				return file.suffix();
			}
		},

		{"{i}", tr("file number in the list of dropped files"),
			[&]()
			{
				return m_droppedFileNumber > 0 ?
					QString::number(m_droppedFileNumber + 1) : QString();
			}
		},
	};

	std::sort(m_variables.begin(), m_variables.end(),
		[&](const vsedit::VariableToken & a_first,
			const vsedit::VariableToken & a_second) -> bool
		{
			return (a_first.token.length() > a_second.token.length());
		});
}

// END OF void ScriptEditor::fillVariables()
//==============================================================================
