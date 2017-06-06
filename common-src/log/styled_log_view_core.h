#ifndef STYLED_LOG_VIEW_CORE_H_INCLUDED
#define STYLED_LOG_VIEW_CORE_H_INCLUDED

#include <QString>
#include <QDateTime>
#include <QJsonObject>

//==============================================================================

extern const char LOG_STYLE_DEFAULT[];
extern const char LOG_STYLE_TITLE_DEFAULT[];

extern const char LE_IS_DIVIDER[];
extern const char LE_TIME[];
extern const char LE_TEXT[];
extern const char LE_STYLE[];

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

	QJsonObject toJson() const;
	static LogEntry fromJson(const QJsonObject & a_object);
};

//==============================================================================

#endif // STYLED_LOG_VIEW_CORE_H_INCLUDED
