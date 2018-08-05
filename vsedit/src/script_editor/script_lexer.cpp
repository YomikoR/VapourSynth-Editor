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

	fillTokenTypeFormatMap();

	m_keywordsList << "False" << "None" << "True" << "and" << "as" <<
		"assert" << "break" << "class" << "continue" << "def" << "del" <<
		"elif" << "else" << "except" << "finally" << "for" << "from" <<
		"global" << "if" << "import" << "in" << "is" << "lambda" <<
		"nonlocal" << "not" << "or" << "pass" << "raise" << "return" <<
		"try" << "while" << "with" << "yield";

	// MUST be sorted by length in descending order.
	m_operatorsList << "//=" << ">>=" << "<<=" << "**=" << "**" << "//" <<
		"<<" << ">>" << "<=" << ">=" << "==" << "!=" << "->" << "+=" << "-=" <<
		"*=" << "/=" << "%=" << "&=" << "|=" << "^=" << "+" << "-" << "*" <<
		"/" << "%" << "&" << "|" << "^" << "~" << "<" << ">" << "(" << ")" <<
		"[" << "]" << "{" << "}" << "," << ":" << "." << ";" << "@" << "=";
	// Don't trust yourself. Sort it.
	std::sort(m_operatorsList.begin(), m_operatorsList.end(),
		[&](const QString & a_first, const QString & a_second)->bool
		{
			return (a_first.length() > a_second.length());
		});

	slotLoadSettings();

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
	if(!m_pSettingsManager)
		return;

	fillTokenTypeFormatMap();
	highlight();
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

	std::list<ScriptContext> contextStack;
	int i = 0;
	int j = 0;
	if(!m_tokens.empty())
	{
		const Token & lastToken = m_tokens.back();
		contextStack = lastToken.contextStack;
		i = lastToken.start + lastToken.length();
	}

	const QString text = m_pDocument->toPlainText();
	const int textLength = text.length();
	while(i < textLength)
	{
		// TODO: actual implementation
		i++;
	}
}

// END OF void ScriptLexer::parse(int a_from)
//==============================================================================

void ScriptLexer::highlight(int a_from)
{
	for(TokenIterator it = tokenAt(a_from); it != m_tokens.end(); it++)
	{
		format(it->start, it->length(), m_tokenTypeFormatMap[it->type]);
	}
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

void ScriptLexer::fillTokenTypeFormatMap()
{
	static std::map<TokenType, const char *> tokenTypeToSettingsKeyMap;
	if(tokenTypeToSettingsKeyMap.empty())
	{
		std::map<TokenType, const char *> tempMap = {
			{TokenType::Undecided, TEXT_FORMAT_ID_COMMON_SCRIPT_TEXT},
			{TokenType::Module, TEXT_FORMAT_ID_COMMON_SCRIPT_TEXT},
			{TokenType::Keyword, TEXT_FORMAT_ID_KEYWORD},
			{TokenType::Operator, TEXT_FORMAT_ID_OPERATOR},
			{TokenType::String, TEXT_FORMAT_ID_STRING},
			{TokenType::Number, TEXT_FORMAT_ID_NUMBER},
			{TokenType::Comment, TEXT_FORMAT_ID_COMMENT},
			{TokenType::Variable, TEXT_FORMAT_ID_COMMON_SCRIPT_TEXT},
			{TokenType::Function, TEXT_FORMAT_ID_COMMON_SCRIPT_TEXT},
			{TokenType::FunctionArgument, TEXT_FORMAT_ID_COMMON_SCRIPT_TEXT},
			{TokenType::VSModule, TEXT_FORMAT_ID_COMMON_SCRIPT_TEXT},
			{TokenType::VSCore, TEXT_FORMAT_ID_VS_CORE},
			{TokenType::VSNamespace, TEXT_FORMAT_ID_VS_NAMESPACE},
			{TokenType::VSFunction, TEXT_FORMAT_ID_VS_FUNCTION},
			{TokenType::VSConstant, TEXT_FORMAT_ID_COMMON_SCRIPT_TEXT},
			{TokenType::VSClip, TEXT_FORMAT_ID_COMMON_SCRIPT_TEXT},
		};
		tokenTypeToSettingsKeyMap = std::move(tempMap);
	}
	for(const std::pair<TokenType, const char *> record :
		tokenTypeToSettingsKeyMap)
	{
		m_tokenTypeFormatMap[record.first] =
			m_pSettingsManager->getTextFormat(record.second);
	}
}

// END OF void ScriptLexer::fillTokenTypeFormatMap()
//==============================================================================
