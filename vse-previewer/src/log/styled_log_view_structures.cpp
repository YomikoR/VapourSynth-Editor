#include "styled_log_view_structures.h"

#include <QGuiApplication>
#include <QPalette>

//==============================================================================

TextBlockStyle::TextBlockStyle(const QString & a_name, const QString & a_title):
	  name(a_name)
	, title(a_title)
	, isAlias(false)
	, isVisible(true)
{
	QPalette palette = QGuiApplication::palette();
	textFormat.setBackground(palette.color(QPalette::Active, QPalette::Base));
	textFormat.setForeground(palette.color(QPalette::Active, QPalette::Text));
}

// END OFTextBlockStyle::TextBlockStyle(const QString & a_name,
//		const QString & a_title)
//==============================================================================

TextBlockStyle::TextBlockStyle(const QString & a_name, const QString & a_title,
	const QTextCharFormat & a_textFormat):
	  name(a_name)
	, title(a_title)
	, textFormat(a_textFormat)
	, isAlias(false)
	, isVisible(true)
{
}

// END OF TextBlockStyle::TextBlockStyle(const QString & a_name,
//		const QString & a_title, const QTextCharFormat & a_textFormat)
//==============================================================================

TextBlockStyle::TextBlockStyle(const QString & a_name, const QString & a_title,
	const QColor & a_backgroundColor, const QColor & a_textColor):
	  name(a_name)
	, title(a_title)
	, isAlias(false)
	, isVisible(true)
{
	textFormat.setForeground(a_textColor);
	textFormat.setBackground(a_backgroundColor);
}

// END OF TextBlockStyle::TextBlockStyle(const QString & a_name,
//		const QString & a_title, const QColor & a_backgroundColor,
//		const QColor & a_textColor)
//==============================================================================

TextBlockStyle::TextBlockStyle(const QString & a_aliasName,
	const QString & a_title, const QString & a_originalStyleName):
	  name(a_aliasName)
	, title(a_title)
	, isAlias(true)
	, originalStyleName(a_originalStyleName)
	, isVisible(true)
{
}

// END OF TextBlockStyle::TextBlockStyle(const QString & a_aliasName,
//		const QString & a_title, const QString & a_originalStyleName)
//==============================================================================
