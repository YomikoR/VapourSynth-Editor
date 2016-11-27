#ifndef STYLED_LOG_VIEW_STRUCTURES_H_INCLUDED
#define STYLED_LOG_VIEW_STRUCTURES_H_INCLUDED

#include <QString>
#include <QColor>
#include <QTextCharFormat>
#include <QDateTime>

//==============================================================================

extern const char LOG_STYLE_DEFAULT[];
extern const char LOG_STYLE_TITLE_DEFAULT[];

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

struct LogEntry
{
	bool isDivider;
	QDateTime time;
	QString text;
	QString style;

	LogEntry(bool a_isDivider = false, const QString & a_text = QString(),
		const QString & a_style = LOG_STYLE_DEFAULT);

	LogEntry(const QString & a_text, const QString & a_style =
		LOG_STYLE_DEFAULT);

	static LogEntry divider();
};

//==============================================================================

#endif // STYLED_LOG_VIEW_STRUCTURES_H_INCLUDED
