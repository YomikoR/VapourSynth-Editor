#include "styled_log_view.h"

#include <QScrollBar>
#include <cassert>

//==============================================================================

const char DEFAULT_LOG_TEXT_BLOCK_STYLE_NAME[] = "default";

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

StyledLogView::StyledLogView(QWidget * a_pParent) :
	  QTextEdit(a_pParent)
{
	setReadOnly(true);
}

// END OF StyledLogView::StyledLogView(QWidget * a_pParent)
//==============================================================================

StyledLogView::~StyledLogView()
{
}

// END OF StyledLogView::~StyledLogView()
//==============================================================================

TextBlockStyle StyledLogView::defaultStyle() const
{
	TextBlockStyle style =
	{
		QString("default"),
		palette().color(QPalette::Active, QPalette::Base),
		QTextCharFormat()
	};
	return style;
}

// END OF TextBlockStyle StyledLogView::defaultStyle() const
//==============================================================================

void StyledLogView::addStyle(const TextBlockStyle & a_style,
	bool a_updateExisting)
{
	std::vector<TextBlockStyle>::iterator it = std::find_if(m_styles.begin(),
		m_styles.end(), [&](const TextBlockStyle & la_style) -> bool
		{
			return (la_style.name == a_style.name);
		});

	if(it == m_styles.end())
		m_styles.push_back(a_style);
	else if(a_updateExisting)
		*it = a_style;
}

// END OF void StyledLogView::addStyle(const TextBlockStyle & a_style,
//		bool a_updateExisting)
//==============================================================================

void StyledLogView::addEntry(const QString & a_text, const QString & a_style)
{
	m_entries.push_back(LogEntry(a_text, a_style));
	updateHtml();
}

// END OF void StyledLogView::addEntry(const QString & a_text,
//		const QString & a_style)
//==============================================================================

void StyledLogView::startNewBlock()
{
	if(m_entries.empty())
		return;
	if(m_entries.back().isDivider)
		return;
	m_entries.push_back(LogEntry::divider());
}

// END OF void StyledLogView::startNewBlock()
//==============================================================================

void StyledLogView::clear()
{
	m_entries.clear();
	QTextEdit::clear();
}

// END OF void StyledLogView::clear()
//==============================================================================

void StyledLogView::updateHtml()
{
	if(m_entries.empty())
	{
		QTextEdit::clear();
		return;
	}

	QString html;
	bool openBlock = false;
	QDateTime lastTime;
	QString lastStyle;

	html += QString("<table>");

	for(const LogEntry & entry : m_entries)
	{
		if(openBlock)
		{
			if(entry.isDivider || (lastTime.msecsTo(entry.time) > 2000) ||
				(entry.style != lastStyle))
			{
				html += QString("</td></tr>");
				openBlock = false;
			}
		}

		lastStyle = entry.style;
		lastTime = entry.time;

		if(entry.isDivider)
			continue;

		TextBlockStyle style = defaultStyle();
		std::vector<TextBlockStyle>::iterator it =
			std::find_if(m_styles.begin(), m_styles.end(),
				[&](const TextBlockStyle & a_style)
				{
					return (a_style.name == entry.style);
				});
		if(it != m_styles.end())
			style = *it;

		if(!openBlock)
		{
			html += QString("<tr bgcolor=\"%1\"><td>")
				.arg(style.backgroundColor.name());
			QString timeString = entry.time.toString("yyyy-MM-dd hh:mm:ss.zzz");
			html += QString("<p><font size=\"-2\">%1</font></p>")
				.arg(timeString);
			openBlock = true;
		}

		QString entryHtml = entry.text;
		entryHtml.replace("\n", "<br>");
		html += QString("<p>%1</p>").arg(entryHtml);
	}

	if(openBlock)
		html += QString("</td></tr>");

	html += QString("</table>");

	document()->setHtml(html);

	verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

// END OF void StyledLogView::updateHtml()
//==============================================================================
