#ifndef STYLED_LOG_VIEW_CORE_H_INCLUDED
#define STYLED_LOG_VIEW_CORE_H_INCLUDED

#include <QString>
#include <QDateTime>

//==============================================================================

extern const char LOG_STYLE_DEFAULT[];
extern const char LOG_STYLE_TITLE_DEFAULT[];

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

#endif // STYLED_LOG_VIEW_CORE_H_INCLUDED
