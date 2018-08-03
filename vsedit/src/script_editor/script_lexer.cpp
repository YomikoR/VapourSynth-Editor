#include <utility>

#include "script_lexer.h"

#include "number_matcher.h"
#include "../../../common-src/settings/settings_manager.h"

#include <QTextDocument>
#include <QTextCursor>
#include <algorithm>

//==============================================================================

ScriptLexer::ScriptLexer(QTextDocument * a_pDocument,
	SettingsManager * a_pSettingsManager) : QObject(a_pDocument)
	, m_pDocument(a_pDocument)
	, m_pSettingsManager(a_pSettingsManager)
{
	Q_ASSERT(m_pDocument);
	connect(m_pDocument, SIGNAL(contentsChange(int, int, int)),
		this, SLOT(slotContentChanged(int, int, int)));
}

// END OF ScriptLexer::ScriptLexer(QTextDocument * a_pDocument,
//		SettingsManager * a_pSettingsManager, QObject * a_pParent)
//==============================================================================

void ScriptLexer::setSettingsManager(SettingsManager * a_pSettingsManager)
{
	m_pSettingsManager = a_pSettingsManager;
	if(m_pSettingsManager)
	{
		slotLoadSettings();
		highlight();
	}
}

// END OF void ScriptLexer::setSettingsManager(
//		SettingsManager * a_pSettingsManager)
//==============================================================================

void ScriptLexer::setPluginsList(VSPluginsList a_pluginsList)
{
	m_pluginsList = std::move(a_pluginsList);
	slotParseAndHighlight();
}

// END OF void ScriptLexer::setPluginsList(VSPluginsList a_pluginsList)
//==============================================================================

void ScriptLexer::slotLoadSettings()
{

}

// END OF void ScriptLexer::slotLoadSettings()
//==============================================================================

void ScriptLexer::slotParseAndHighlight()
{
	parse();
	highlight();
}

// END OF void ScriptLexer::slotParseAndHighlight()
//==============================================================================

void ScriptLexer::slotContentChanged(int a_position, int a_charsRemoved,
	int a_charsAdded)
{
	(void)a_charsRemoved;
	(void)a_charsAdded;
	parse(a_position);
	highlight(a_position);
}

// END OF void ScriptLexer::slotContentChanged(int a_position,
//		int a_charsRemoved, int a_charsAdded)
//==============================================================================

void ScriptLexer::parse(int a_from)
{
	TokenIterator it = tokenAt(a_from);
	m_tokens.erase(it, m_tokens.end());
}

// END OF void ScriptLexer::parse(int a_from)
//==============================================================================

void ScriptLexer::highlight(int a_from)
{
	(void)a_from;
}

// END OF void ScriptLexer::highlight(int a_from)
//==============================================================================

void ScriptLexer::format(int a_start, int a_count,
	const QTextCharFormat & a_format)
{
	QTextCursor cursor(m_pDocument);
	cursor.setPosition(a_start);
	cursor.setPosition(a_start + a_count, QTextCursor::KeepAnchor);
	cursor.setCharFormat(a_format);
}

// END OF void ScriptLexer::format(int a_start, int a_count,
//		const QTextCharFormat & a_format)
//==============================================================================

void ScriptLexer::format(const Token & a_token,
	const QTextCharFormat & a_format)
{
	format(a_token.start, a_token.length(), a_format);
}

// END OF void ScriptLexer::format(const Token & a_token,
//		const QTextCharFormat & a_format)
//==============================================================================

TokenIterator ScriptLexer::tokenAt(int a_textPosition)
{
	if(m_tokens.empty())
		return m_tokens.end();

	Token fakeToken(a_textPosition);
	TokenIterator it = std::lower_bound(m_tokens.begin(),
		m_tokens.end(), fakeToken);
	if(std::distance(m_tokens.begin(), it) > 0)
		it--;

	return it;
}

// END OF TokenIterator ScriptLexer::tokenAt(int a_textPosition)
//==============================================================================
