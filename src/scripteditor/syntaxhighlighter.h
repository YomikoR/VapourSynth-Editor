#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QStringList>

#include "../vapoursynth/vsplugindata.h"

class SyntaxHighlighter : public QSyntaxHighlighter
{
	public:

		SyntaxHighlighter(QTextDocument * a_pDocument,
			VSPluginsList a_pluginsList = VSPluginsList());
		virtual ~SyntaxHighlighter();

		void setCoreName(const QString & a_coreName);

		void setPluginsList(VSPluginsList a_pluginsList);

	protected:

		void highlightBlock(const QString & a_text);

	private:

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
