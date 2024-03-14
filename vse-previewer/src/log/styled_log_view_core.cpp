#include "styled_log_view_core.h"

#include <QVariant>

//==============================================================================

const char LOG_STYLE_DEFAULT[] = "info";
const char LOG_STYLE_TITLE_DEFAULT[] = "Info message";

const char LE_IS_DIVIDER[] = "isDivider";
const char LE_TIME[] = "time";
const char LE_TEXT[] = "text";
const char LE_STYLE[] = "style";

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

QJsonObject LogEntry::toJson() const
{
	QJsonObject jsLogEntry;
	jsLogEntry[LE_IS_DIVIDER] = isDivider;
	jsLogEntry[LE_TIME] = time.toMSecsSinceEpoch();
	if(isDivider)
		return jsLogEntry;
	jsLogEntry[LE_TEXT] = text;
	jsLogEntry[LE_STYLE] = style;
	return jsLogEntry;
}

// END OF QJsonObject LogEntry::toJson() const
//==============================================================================

LogEntry LogEntry::fromJson(const QJsonObject & a_object)
{
	LogEntry entry;
	if(a_object.contains(LE_IS_DIVIDER))
		entry.isDivider = a_object[LE_IS_DIVIDER].toBool();
	if(a_object.contains(LE_TIME))
		entry.time = QDateTime::fromMSecsSinceEpoch(
			a_object[LE_TIME].toVariant().toLongLong());
	if(a_object.contains(LE_TEXT))
		entry.text = a_object[LE_TEXT].toString();
	if(a_object.contains(LE_STYLE))
		entry.style = a_object[LE_STYLE].toString();
	return entry;
}

// END OF LogEntry LogEntry::fromJson(const QJsonObject & a_object)
//==============================================================================
