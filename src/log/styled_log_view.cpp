#include "styled_log_view.h"

#include <QScrollBar>
#include <cassert>

//==============================================================================

StyledLogView::StyledLogView(QWidget * a_pParent) :
	  QTextEdit(a_pParent)
	, m_millisecondsToDivideBlocks(2000)
{
	setReadOnly(true);
	addStyle(TextBlockStyle(LOG_STYLE_DEFAULT));
}

// END OF StyledLogView::StyledLogView(QWidget * a_pParent)
//==============================================================================

StyledLogView::~StyledLogView()
{
}

// END OF StyledLogView::~StyledLogView()
//==============================================================================

TextBlockStyle StyledLogView::getStyle(const QString & a_styleName) const
{
	TextBlockStyle style(a_styleName);

	QString styleName = a_styleName;

	QStringList foundAliasesList;

	std::vector<TextBlockStyle>::const_iterator it = m_styles.end();

	while(true)
	{
		it = std::find_if(m_styles.begin(), m_styles.end(),
			[&](const TextBlockStyle & a_style) -> bool
			{
				return (a_style.name == styleName);
			});

		if(it == m_styles.end())
			return style;

		// Alias retains its own visibility and title
		if(it->name == a_styleName)
		{
			style.isVisible = it->isVisible;
			style.title = it->title;
		}

		if(!it->isAlias)
		{
			style.backgroundColor = it->backgroundColor;
			style.textFormat = it->textFormat;
			return style;
		}

		foundAliasesList += it->name;

		// Check for aliasing loop
		if(foundAliasesList.contains(it->originalStyleName))
			return style;

		styleName = it->originalStyleName;
	}

	return style;
}

// END OF TextBlockStyle StyledLogView::getStyle(
//		const QString & a_styleName) const
//==============================================================================

void StyledLogView::addStyle(const TextBlockStyle & a_style,
	bool a_updateExisting)
{
	TextBlockStyle newStyle(a_style);

	// Resolve original style for alias to prevent alias loop
	if(newStyle.isAlias)
	{
		TextBlockStyle originalStyle = getStyle(newStyle.originalStyleName);
		newStyle.originalStyleName = originalStyle.name;
	}

	std::vector<TextBlockStyle>::iterator it = std::find_if(m_styles.begin(),
		m_styles.end(), [&](const TextBlockStyle & la_style) -> bool
		{
			return (la_style.name == newStyle.name);
		});

	if(it == m_styles.end())
		m_styles.push_back(newStyle);
	else if(a_updateExisting)
		*it = newStyle;
}

// END OF void StyledLogView::addStyle(const TextBlockStyle & a_style,
//		bool a_updateExisting)
//==============================================================================

void StyledLogView::addStyle(const QString & a_aliasName,
	const QString & a_title, const QString & a_originalStyleName)
{
	// no aliasing default style name
	if(a_aliasName == LOG_STYLE_DEFAULT)
		return;

	addStyle(TextBlockStyle(a_aliasName, a_title, a_originalStyleName));
}

// END OF void StyledLogView::addStyle(const QString & a_aliasName,
//		const QString & a_title, const QString & a_originalStyleName)
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

qint64 StyledLogView::millisecondsToDivideBlocks() const
{
	return m_millisecondsToDivideBlocks;
}

// END OF qint64 StyledLogView::millisecondsToDivideBlocks() const
//==============================================================================

bool StyledLogView::setMillisecondsToDivideBlocks(qint64 a_value)
{
	if(a_value < 0)
		return false;

	m_millisecondsToDivideBlocks = a_value;
	updateHtml();
	return true;
}

// END OF bool StyledLogView::setMillisecondsToDivideBlocks(qint64 a_value)
//==============================================================================

QStringList StyledLogView::styles(bool a_excludeAliases) const
{
	QStringList stylesList;

	for(const TextBlockStyle & style : m_styles)
		if(!(a_excludeAliases && style.isAlias))
			stylesList += style.name;

	return stylesList;
}

// END OF QStringList StyledLogView::styles(bool a_excludeAliases) const
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
		TextBlockStyle style = getStyle(entry.style);

		if(!style.isVisible)
			continue;

		if(openBlock)
		{
			if(entry.isDivider ||
				(lastTime.msecsTo(entry.time) > m_millisecondsToDivideBlocks) ||
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
