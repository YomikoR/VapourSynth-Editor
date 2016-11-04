#include "styled_log_view_structures.h"

#include <QGuiApplication>
#include <QPalette>

//==============================================================================

const char LOG_STYLE_DEFAULT[] = "default";

//==============================================================================

TextBlockStyle::TextBlockStyle(const QString & a_name):
	  name(a_name)
	, isAlias(false)
	, isVisible(true)
{
	QPalette palette = QGuiApplication::palette();
	backgroundColor = palette.color(QPalette::Active, QPalette::Base);
	textFormat.setForeground(palette.color(QPalette::Active, QPalette::Text));
}

// END OFTextBlockStyle::TextBlockStyle(const QString & a_name)
//==============================================================================

TextBlockStyle::TextBlockStyle(const QString & a_name,
	const QColor & a_backgroundColor, const QTextCharFormat & a_textFormat):
	  name(a_name)
	, backgroundColor(a_backgroundColor)
	, textFormat(a_textFormat)
	, isAlias(false)
	, isVisible(true)
{
}

// END OF TextBlockStyle::TextBlockStyle(const QString & a_name,
//		const QColor & a_backgroundColor, const QTextCharFormat & a_textFormat)
//==============================================================================

TextBlockStyle::TextBlockStyle(const QString & a_name,
	const QColor & a_backgroundColor, const QColor & a_textColor):
	  name(a_name)
	, backgroundColor(a_backgroundColor)
	, isAlias(false)
	, isVisible(true)
{
	textFormat.setForeground(a_textColor);
}

// END OF TextBlockStyle::TextBlockStyle(const QString & a_name,
//		const QColor & a_backgroundColor, const QColor & a_textColor)
//==============================================================================

TextBlockStyle::TextBlockStyle(const QString & a_aliasName,
	const QString & a_originalStyleName):
	  name(a_aliasName)
	, isAlias(true)
	, originalStyleName(a_originalStyleName)
	, isVisible(true)
{
}

// END OF TextBlockStyle::TextBlockStyle(const QString & a_aliasName,
//		const QString & a_originalStyleName)
//==============================================================================

LogEntry::LogEntry(bool a_isDivider, const QString & a_text,
	const QString & a_style) :
	  isDivider(a_isDivider)
	, time(QDateTime::currentDateTime())
	, text(a_text)
	, style(a_style)
{
}

// END OF LogEntry::LogEntry(bool a_isDivider, const QString & a_text,
//		const QString & a_style)
//==============================================================================

LogEntry::LogEntry(const QString & a_text, const QString & a_style) :
	  isDivider(false)
	, time(QDateTime::currentDateTime())
	, text(a_text)
	, style(a_style)
{
}

// END OF LogEntry::LogEntry(const QString & a_text, const QString & a_style)
//==============================================================================

LogEntry LogEntry::divider()
{
	return LogEntry(true, QString(), QString());
}

// END OF LogEntry LogEntry::divider()
//==============================================================================
