#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include "../vapoursynth/vs_plugin_data.h"

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QStringList>

class SettingsManager;

class SyntaxHighlighter : public QSyntaxHighlighter
{
	Q_OBJECT

public:

	SyntaxHighlighter(QTextDocument * a_pDocument,
		VSPluginsList a_pluginsList = VSPluginsList());
	virtual ~SyntaxHighlighter();

	void setSettingsManager(SettingsManager * a_pSettingsManager);

	void setCoreName(const QString & a_coreName);

	void setPluginsList(VSPluginsList a_pluginsList);

public slots:

	void slotLoadSettings();

protected:

	void highlightBlock(const QString & a_text);

private:

	SettingsManager * m_pSettingsManager;

	QString m_coreName;

	VSPluginsList m_pluginsList;

	QStringList m_keywordsList;
	QStringList m_operatorsList;

	QTextCharFormat m_keywordFormat;
	QTextCharFormat m_operatorFormat;
	QTextCharFormat m_stringFormat;
	QTextCharFormat m_numberFormat;
	QTextCharFormat m_commentFormat;
	QTextCharFormat m_vsCoreFormat;
	QTextCharFormat m_vsNamespaceFormat;
	QTextCharFormat m_vsFunctionFormat;
	QTextCharFormat m_vsArgumentFormat;
};

#endif // SYNTAXHIGHLIGHTER_H
