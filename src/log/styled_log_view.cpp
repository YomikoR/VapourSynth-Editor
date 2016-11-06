#include "styled_log_view.h"

#include <QMenu>
#include <QScrollBar>
#include <QDir>
#include <QFileDialog>
#include <QFile>
#include <cassert>

//==============================================================================

StyledLogView::StyledLogView(QWidget * a_pParent) :
	  QTextEdit(a_pParent)
	, m_millisecondsToDivideBlocks(2000)
	, m_pContextMenu(nullptr)
{
	setReadOnly(true);
	setContextMenuPolicy(Qt::CustomContextMenu);
	addStyle(TextBlockStyle(LOG_STYLE_DEFAULT));
	createActionsAndMenus();

	connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
		this, SLOT(slotShowCustomMenu(const QPoint &)));
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

	createActionsAndMenus();
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

bool StyledLogView::saveHtml(const QString & a_filePath,
	bool a_excludeFiltered)
{
	if(a_filePath.isEmpty())
		return false;

	QFile file(a_filePath);

	bool result = file.open(QIODevice::WriteOnly);
	if(!result)
		return false;

	QByteArray htmlData = realHtml(a_excludeFiltered).toUtf8();
	qint64 bytesWritten = file.write(htmlData);
	file.close();

	if(bytesWritten != htmlData.size())
		return false;

	return true;
}

// END OF bool StyledLogView::saveHtml(const QString & a_filePath,
//		bool a_excludeFiltered)
//==============================================================================

bool StyledLogView::saveHtml(bool a_excludeFiltered)
{
	QString timeString =
		QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss-zzz");
	QString fileName = trUtf8("vapoursynth_editor_log_{time}.html")
		.replace("{time}", timeString);

	QString currentDir = QDir(".").absolutePath();

	QString filePath = currentDir + QString("/") + fileName;

	filePath = QFileDialog::getSaveFileName(this, trUtf8("Save log"),
		filePath, trUtf8("HTML files (*.html);;All files (*.*)"));

	return saveHtml(filePath, a_excludeFiltered);
}

// END OF bool StyledLogView::saveHtml(bool a_excludeFiltered)
//==============================================================================

void StyledLogView::clear()
{
	m_entries.clear();
	QTextEdit::clear();
}

// END OF void StyledLogView::clear()
//==============================================================================

void StyledLogView::slotSaveHtml()
{
	saveHtml();
}

// END OF void StyledLogView::slotSaveHtml()
//==============================================================================

void StyledLogView::slotSaveHtmlFiltered()
{
	saveHtml(true);
}

// END OF void StyledLogView::slotSaveHtmlFiltered()
//==============================================================================

void StyledLogView::slotToggleStyleVisibility(bool a_visible)
{
	QAction * pAction = qobject_cast<QAction *>(sender());
	if(!pAction)
		return;

	QString styleName = pAction->data().toString();

	std::vector<TextBlockStyle>::iterator it = std::find_if(m_styles.begin(),
		m_styles.end(), [&](const TextBlockStyle & a_style) -> bool
		{
			return (a_style.name == styleName);
		});

	if(it == m_styles.end())
		return;

	it->isVisible = a_visible;
	updateHtml();
}

// END OF void StyledLogView::slotToggleStyleVisibility(bool a_visible)
//==============================================================================

void StyledLogView::slotShowCustomMenu(const QPoint & a_position)
{
	QPoint globalPosition = mapToGlobal(a_position);
    m_pContextMenu->exec(globalPosition);
}

// END OF void StyledLogView::slotShowCustomMenu(const QPoint & a_position)
//==============================================================================

void StyledLogView::updateHtml()
{
	if(m_entries.empty())
	{
		QTextEdit::clear();
		return;
	}

	QString borderColor = palette().color(QPalette::Dark).name();

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
		"</style>\n"
		"<table width=\"100%\" cellspacing=\"-1\">\n"
		).replace("{table-border-color}", borderColor);

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
			html += QString("<div><font face=\"%1\" size=\"-2\" "
			"color=\"%2\">%3</font></div>").arg(format.fontFamily())
			.arg(format.foreground().color().name())
			.arg(timeString);
			openBlock = true;
		}

		QString entryHtml = entry.text;
		entryHtml.replace("\n", "<br>");
		html += QString("<div><font face=\"%1\" "
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

void StyledLogView::createActionsAndMenus()
{
	if(m_pContextMenu)
		delete m_pContextMenu;

	m_pContextMenu = createStandardContextMenu();
	m_pContextMenu->addSeparator();

	QMenu * pStyleFiltersMenu =
		m_pContextMenu->addMenu(trUtf8("Filter messages"));

	for(const TextBlockStyle & style : m_styles)
	{
		QAction * pStyleFilterAction =
			pStyleFiltersMenu->addAction(style.title);
		pStyleFilterAction->setData(style.name);
		pStyleFilterAction->setCheckable(true);
		pStyleFilterAction->setChecked(style.isVisible);
		connect(pStyleFilterAction, SIGNAL(toggled(bool)),
			this, SLOT(slotToggleStyleVisibility(bool)));
	}

	m_pContextMenu->addSeparator();

	struct ActionToCreate
	{
		QString title;
		const char * slotToConnect;
	};

	ActionToCreate actionsToCreate[] = {
		{trUtf8("Save"), SLOT(slotSaveHtml())},
		{trUtf8("Save filtered"), SLOT(slotSaveHtmlFiltered())},
		{trUtf8("Clear"), SLOT(clear())},
	};

	for(const ActionToCreate & action : actionsToCreate)
		m_pContextMenu->addAction(action.title, this, action.slotToConnect);
}

// END OF void StyledLogView::createActionsAndMenus()
//==============================================================================

QString StyledLogView::realHtml(bool a_excludeFiltered) const
{
	QString title = trUtf8("VapourSynth Editor log ") +
		QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");

	QString borderColor = palette().color(QPalette::Dark).name();

	QString styleText = QString(
		"<style type=\"text/css\">\n"
		"table, td {border: 1px solid {table-border-color};}\n"
		"table {border-collapse: collapse; width: 100%;}\n"
		"p {padding: 0; margin: 0;}\n"
	).replace("{table-border-color}", borderColor);

	styleText += "</style>\n";

	QString html = QString(
		"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" "
		"\"http://www.w3.org/TR/html4/strict.dtd\">\n"
		"<html>\n"
		"<head>\n"
		"<title>{title}</title>\n"
		"{style}"
		"</head>\n"
		"<body>\n"
		"<table>\n"
	).replace("{title}", title)
		.replace("{style}", styleText);

	bool openBlock = false;
	QDateTime lastTime;
	QString lastStyle;

	for(const LogEntry & entry : m_entries)
	{
		TextBlockStyle style = getStyle(entry.style);

		if(a_excludeFiltered && (!style.isVisible))
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
			html += QString("<tr bgcolor=\"%1\"><td>\n")
				.arg(style.backgroundColor.name());
			QString timeString = entry.time.toString("yyyy-MM-dd hh:mm:ss.zzz");
			html += QString("<p style=\"font-family: %1; "
				"font-size: 0.7em; color: %2;\">%3</p>\n")
				.arg(format.fontFamily())
				.arg(format.foreground().color().name())
				.arg(timeString);
			openBlock = true;
		}

		QString entryHtml = entry.text;
		entryHtml.replace("\n", "<br>\n");
		html += QString("<p style=\"font-family: %1; color: %2;\">%3</p>\n")
			.arg(format.fontFamily())
			.arg(format.foreground().color().name())
			.arg(entryHtml);
	}

	if(openBlock)
		html += QString("</td>\n</tr>\n");

	html += "</table>\n</body>\n</html>\n";

	return html;
}

// END OF QString StyledLogView::realHtml(bool a_excludeFiltered) const
//==============================================================================
