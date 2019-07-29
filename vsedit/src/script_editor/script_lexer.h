#ifndef SCRIPT_LEXER_H
#define SCRIPT_LEXER_H

#include "script_lexer_definitions.h"
#include "../vapoursynth/vs_plugin_data.h"

#include <QObject>
#include <QTextCharFormat>
#include <map>

class SettingsManager;
class QTextDocument;

class ScriptLexer : public QObject
{
	Q_OBJECT

public:

	ScriptLexer(QTextDocument * a_pDocument,
		SettingsManager * a_pSettingsManager = nullptr);
	virtual ~ScriptLexer() = default ;

	void setSettingsManager(SettingsManager * a_pSettingsManager);

	void setPluginsList(VSPluginsList a_pluginsList);

public slots:

	void slotLoadSettings();

	void slotParseAndHighlight();

private slots:

	void slotContentChanged(int a_position, int a_charsRemoved,
		int a_charsAdded);

private:

	void parse(int a_from = 0);
	void highlight(int a_from = 0);
	void format(int a_start, int a_count, const QTextCharFormat & a_format);
	void format(const Token & a_token, const QTextCharFormat & a_format);
	TokenIterator tokenAt(int a_textPosition);
	void fillTokenTypeFormatMap();
	ScriptContext * contextAt(int a_textPosition);

	QTextDocument * m_pDocument;
	SettingsManager * m_pSettingsManager;

	VSPluginsList m_pluginsList;

	TokenVector m_tokens;

	std::map<TokenType, QTextCharFormat> m_tokenTypeFormatMap;

	QStringList m_keywordsList;
	QStringList m_operatorsList;

	std::unique_ptr<ScriptContext> m_pRootContext{new ScriptContext};
};

#endif // SCRIPT_LEXER_H
