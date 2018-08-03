#ifndef SCRIPT_LEXER_H
#define SCRIPT_LEXER_H

#include "script_lexer_definitions.h"
#include "../vapoursynth/vs_plugin_data.h"

#include <QObject>
#include <vector>

class SettingsManager;
class QTextDocument;

class ScriptLexer : public QObject
{
	Q_OBJECT

public:

	ScriptLexer(QTextDocument * a_pDocument,
		SettingsManager * a_pSettingsManager = nullptr,
		QObject * a_pParent = nullptr);
	virtual ~ScriptLexer();

	void setSettingsManager(SettingsManager * a_pSettingsManager);

	void setPluginsList(VSPluginsList a_pluginsList);

public slots:

	void slotLoadSettings();

	void slotParseAndHighlight();

private:

	void parse();
	void highlight();

	QTextDocument * m_pDocument;
	SettingsManager * m_pSettingsManager;

	VSPluginsList m_pluginsList;

	std::vector<Token> m_tokens;
};

#endif // SCRIPT_LEXER_H
