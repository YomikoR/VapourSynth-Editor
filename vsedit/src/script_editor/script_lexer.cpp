#include "script_lexer.h"

#include "number_matcher.h"
#include "../../../common-src/settings/settings_manager.h"

//==============================================================================

ScriptLexer::ScriptLexer(QTextDocument * a_pDocument,
	SettingsManager * a_pSettingsManager, QObject * a_pParent) :
	  QObject(a_pParent)
	, m_pDocument(a_pDocument)
	, m_pSettingsManager(a_pSettingsManager)
{
	Q_ASSERT(a_pDocument);
}

// END OF ScriptLexer::ScriptLexer(QTextDocument * a_pDocument,
//		SettingsManager * a_pSettingsManager, QObject * a_pParent)
//==============================================================================

ScriptLexer::~ScriptLexer()
{

}

// END OF ScriptLexer::~ScriptLexer()
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
	m_pluginsList = a_pluginsList;
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

void ScriptLexer::parse()
{

}

// END OF void ScriptLexer::parse()
//==============================================================================

void ScriptLexer::highlight()
{

}

// END OF void ScriptLexer::highlight()
//==============================================================================
