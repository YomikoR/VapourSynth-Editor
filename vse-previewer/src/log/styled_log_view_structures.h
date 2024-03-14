#ifndef STYLED_LOG_VIEW_STRUCTURES_H_INCLUDED
#define STYLED_LOG_VIEW_STRUCTURES_H_INCLUDED

#include "styled_log_view_core.h"

#include <QColor>
#include <QTextCharFormat>

//==============================================================================

struct TextBlockStyle
{
	QString name;
	QString title;
	QTextCharFormat textFormat;
	bool isAlias;
	QString originalStyleName;
	bool isVisible;

	TextBlockStyle(const QString & a_name = LOG_STYLE_DEFAULT,
		const QString & a_title = LOG_STYLE_TITLE_DEFAULT);
	TextBlockStyle(const QString & a_name, const QString & a_title,
		const QTextCharFormat & a_textFormat);
	TextBlockStyle(const QString & a_name, const QString & a_title,
		const QColor & a_backgroundColor, const QColor & a_textColor);
	TextBlockStyle(const QString & a_aliasName, const QString & a_title,
		const QString & a_originalStyleName);
};

//==============================================================================

#endif // STYLED_LOG_VIEW_STRUCTURES_H_INCLUDED
