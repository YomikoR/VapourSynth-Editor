#include "styled_log_view.h"

#include "styled_log_view_settings_dialog.h"

#include <QMenu>
#include <QScrollBar>
#include <QDir>
#include <QFileDialog>
#include <QFile>

//==============================================================================

const size_t StyledLogView::DEFAULT_MAX_ENTRIES_TO_SHOW = 200;

//==============================================================================

StyledLogView::StyledLogView(QWidget * a_pParent) :
	  QTextEdit(a_pParent)
	, m_millisecondsToDivideBlocks(2000)
	, m_pContextMenu(nullptr)
	, m_pSettingsDialog(nullptr)
	, m_maxEntriesToShow(DEFAULT_MAX_ENTRIES_TO_SHOW)
{
	setReadOnly(true);
	setContextMenuPolicy(Qt::CustomContextMenu);
	addStyle(TextBlockStyle(LOG_STYLE_DEFAULT));
	createActionsAndMenus();

	m_pSettingsDialog = new StyledLogViewSettingsDialog();

	connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
		this, SLOT(slotShowCustomMenu(const QPoint &)));
	connect(m_pSettingsDialog, SIGNAL(signalSettingsChanged()),
		this, SLOT(slotLogSettingsChanged()));
}

// END OF StyledLogView::StyledLogView(QWidget * a_pParent)
//==============================================================================

StyledLogView::~StyledLogView()
{
	delete m_pSettingsDialog;
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
	QString fileName = tr("vapoursynth_editor_log_{time}.html")
		.replace("{time}", timeString);

	QString currentDir = QDir(".").absolutePath();

	QString filePath = currentDir + QString("/") + fileName;

	filePath = QFileDialog::getSaveFileName(this, tr("Save log"),
		filePath, tr("HTML files (*.html);;All files (*.*)"));

	return saveHtml(filePath, a_excludeFiltered);
}

// END OF bool StyledLogView::saveHtml(bool a_excludeFiltered)
//==============================================================================

void StyledLogView::addEntry(const QString & a_text, const QString & a_style)
{
	m_entries.push_back(LogEntry(a_text, a_style));
	updateHtml();
}

// END OF void StyledLogView::addEntry(const QString & a_text,
//		const QString & a_style)
//==============================================================================

void StyledLogView::addEntry(const LogEntry & a_entry)
{
	m_entries.push_back(a_entry);
	updateHtml();
}

// END OF void void StyledLogView::addEntry(const LogEntry & a_entry)
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

void StyledLogView::slotShowCustomMenu(const QPoint & a_position)
{
	createActionsAndMenus();
	QPoint globalPosition = mapToGlobal(a_position);
    m_pContextMenu->popup(globalPosition);
}

// END OF void StyledLogView::slotShowCustomMenu(const QPoint & a_position)
//==============================================================================

void StyledLogView::slotLogSettings()
{
	Q_ASSERT(m_pSettingsDialog);
	m_pSettingsDialog->setStyles(m_styles);
	m_pSettingsDialog->show();
}

// END OF void StyledLogView::slotLogSettings()
//==============================================================================

void StyledLogView::slotLogSettingsChanged()
{
	m_styles = m_pSettingsDialog->styles();
	updateHtml();
}

// END OF void StyledLogView::slotLogSettingsChanged()
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

	size_t firstEntryToShow = 0;

	if(m_entries.size() > m_maxEntriesToShow)
	{
		firstEntryToShow = m_entries.size() - m_maxEntriesToShow;
		html += tr("<tr><td align=\"center\">%1 entries not shown. "
			"Save the log to read.</td></tr>\n").arg(firstEntryToShow);
	}

	for(size_t i = firstEntryToShow; i < m_entries.size(); ++i)
	{
		const LogEntry & entry = m_entries[i];

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
		QFont styleFont = format.font();

		if(!openBlock)
		{
			html += QString("<tr bgcolor=\"%1\"><td>")
				.arg(format.background().color().name());
			QString timeString = entry.time.toString("yyyy-MM-dd hh:mm:ss.zzz");
			html += QString("<div><font size=\"-2\" "
				"color=\"%1\">%2</font></div>")
				.arg(format.foreground().color().name())
				.arg(timeString);
			openBlock = true;
		}

		QString entryHtml = entry.text;
		entryHtml.replace("\n", "<br>");

		if(styleFont.bold())
			entryHtml = QString("<b>%1</b>").arg(entryHtml);
		if(styleFont.italic())
			entryHtml = QString("<i>%1</i>").arg(entryHtml);
		if(styleFont.underline())
			entryHtml = QString("<u>%1</u>").arg(entryHtml);
		if(styleFont.strikeOut())
			entryHtml = QString("<s>%1</s>").arg(entryHtml);

		html += QString("<div><font face=\"%1\" "
			"color=\"%2\">%3</font></div>")
			.arg(styleFont.family())
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

	struct ActionToCreate
	{
		QString title;
		const char * slotToConnect;
		bool isSeparator;

		ActionToCreate(const QString & a_title, const char * a_slotToConnect) :
			title(a_title), slotToConnect(a_slotToConnect), isSeparator(false)
		{};

		ActionToCreate() : title(), slotToConnect(nullptr), isSeparator(true)
		{};
	};

	const ActionToCreate SEPARATOR;

	ActionToCreate actionsToCreate[] = {
		SEPARATOR,
		{tr("Log settings"), SLOT(slotLogSettings())},
		SEPARATOR,
		{tr("Save"), SLOT(slotSaveHtml())},
		{tr("Save filtered"), SLOT(slotSaveHtmlFiltered())},
		{tr("Clear"), SLOT(clear())},
	};

	for(const ActionToCreate & action : actionsToCreate)
	{
		if(action.isSeparator)
			m_pContextMenu->addSeparator();
		else
			m_pContextMenu->addAction(action.title, this, action.slotToConnect);
	}
}

// END OF void StyledLogView::createActionsAndMenus()
//==============================================================================

QString StyledLogView::realHtml(bool a_excludeFiltered) const
{
	QString title = tr("VapourSynth Editor log ") +
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
		QFont styleFont = format.font();

		if(!openBlock)
		{
			html += QString("<tr bgcolor=\"%1\"><td>\n")
				.arg(format.background().color().name());
			QString timeString = entry.time.toString("yyyy-MM-dd hh:mm:ss.zzz");
			html += QString("<p style=\"font-size: 70%; "
				"color: %1;\">%2</p>\n")
				.arg(format.foreground().color().name())
				.arg(timeString);
			openBlock = true;
		}

		QString entryHtml = entry.text;
		entryHtml.replace("\n", "<br>\n");

		if(styleFont.bold())
			entryHtml = QString("<b>%1</b>").arg(entryHtml);
		if(styleFont.italic())
			entryHtml = QString("<i>%1</i>").arg(entryHtml);
		if(styleFont.underline())
			entryHtml = QString("<u>%1</u>").arg(entryHtml);
		if(styleFont.strikeOut())
			entryHtml = QString("<s>%1</s>").arg(entryHtml);

		html += QString("<p style=\"font-family: %1; color: %2;\">%3</p>\n")
			.arg(styleFont.family())
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
