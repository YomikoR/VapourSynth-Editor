#include "styled_log_view.h"

#include <QScrollBar>
#include <cassert>

//==============================================================================

const char LOG_STYLE_DEFAULT[] = "default";

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
		LOG_STYLE_DEFAULT,
		palette().color(QPalette::Active, QPalette::Base),
		QTextCharFormat()
	};
	return style;
}

// END OF TextBlockStyle StyledLogView::defaultStyle() const
//==============================================================================

TextBlockStyle StyledLogView::getStyle(const QString & a_styleName) const
{
	QString styleName = a_styleName;

	// resolve alias
	std::map<QString, QString>::const_iterator aliases_it =
		m_styleAliases.find(a_styleName);
	if(aliases_it != m_styleAliases.end())
		styleName = aliases_it->second;
	assert(!styleName.isEmpty());

	// get actual style
	std::vector<TextBlockStyle>::const_iterator it =
		std::find_if(m_styles.begin(), m_styles.end(),
			[&](const TextBlockStyle & a_style) -> bool
			{
				return (a_style.name == styleName);
			});

	if(it != m_styles.end())
		return *it;
	else
		return defaultStyle();
}

// END OF TextBlockStyle StyledLogView::getStyle(
//		const QString & a_styleName) const
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

	m_styleAliases.erase(a_style.name);
}

// END OF void StyledLogView::addStyle(const TextBlockStyle & a_style,
//		bool a_updateExisting)
//==============================================================================

void StyledLogView::addStyle(const QString & a_styleName,
		const QString & a_existingStyleName)
{
	// no aliasing default style name
	if(a_styleName == LOG_STYLE_DEFAULT)
		return;

	// check if style name already corresponds to real style
	std::vector<TextBlockStyle>::iterator it = std::find_if(m_styles.begin(),
		m_styles.end(), [&](const TextBlockStyle & a_style) -> bool
		{
			return (a_style.name == a_styleName);
		});

	if(it != m_styles.end())
		return;

	// aliasing
	QString existingStyleName = LOG_STYLE_DEFAULT;

	it = std::find_if(m_styles.begin(), m_styles.end(),
		[&](const TextBlockStyle & a_style) -> bool
		{
			return (a_style.name == a_existingStyleName);
		});

	if(it != m_styles.end())
		existingStyleName = a_existingStyleName;

	m_styleAliases[a_styleName] = existingStyleName;
}

// END OF void StyledLogView::addStyle(const QString & a_styleName,
//		const QString & a_existingStyleName)
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

	QString html = QString(
		"<body>\n"
		"<style type=\"text/css\">\n"
		"table {"
			"border-width: 1px;"
			"border-style: solid;"
			"border-color: {table-border-color};"
		"}\n"
		"div {"
			"margin-left: 2px;"
			"margin-top: 2px;"
			"margin-right: 2px;"
			"margin-bottom: 2px;"
		"}\n"
		"</style>"
		"<table width=\"100%\" cellspacing=\"-1\">\n"
		).replace("{table-border-color}",
		palette().color(QPalette::Dark).name());

	bool openBlock = false;
	QDateTime lastTime;
	QString lastStyle;

	for(const LogEntry & entry : m_entries)
	{
		if(openBlock)
		{
			if(entry.isDivider || (lastTime.msecsTo(entry.time) > 2000) ||
				(entry.style != lastStyle))
			{
				html += QString("</td></tr>\n");
				openBlock = false;
			}
		}

		lastStyle = entry.style;
		lastTime = entry.time;

		if(entry.isDivider)
			continue;

		TextBlockStyle style = getStyle(entry.style);
		QTextCharFormat format = style.textFormat;

		if(!openBlock)
		{
			html += QString("<tr bgcolor=\"%1\"><td>")
				.arg(style.backgroundColor.name());
			QString timeString = entry.time.toString("yyyy-MM-dd hh:mm:ss.zzz");
			html += QString("<div><font family=\"%1\" size=\"-2\" "
			"color=\"%2\">%3</font></div>").arg(format.fontFamily())
			.arg(format.foreground().color().name())
			.arg(timeString);
			openBlock = true;
		}

		QString entryHtml = entry.text;
		entryHtml.replace("\n", "<br>");
		html += QString("<div><font family=\"%1\" "
			"color=\"%2\">%3</font></div>").arg(format.fontFamily())
			.arg(format.foreground().color().name())
			.arg(entryHtml);
	}

	if(openBlock)
		html += QString("</td></tr>\n");

	html += QString("</table>\n</body>");

	setHtml(html);

	verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

// END OF void StyledLogView::updateHtml()
//==============================================================================
