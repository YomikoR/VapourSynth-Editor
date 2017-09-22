#ifndef STYLED_LOG_VIEW_H_INCLUDED
#define STYLED_LOG_VIEW_H_INCLUDED

#include "styled_log_view_structures.h"

#include <QTextEdit>
#include <vector>

class StyledLogViewSettingsDialog;

class StyledLogView : public QTextEdit
{
	Q_OBJECT

public:

	static const size_t DEFAULT_MAX_ENTRIES_TO_SHOW;

	StyledLogView(QWidget * a_pParent = nullptr);
	virtual ~StyledLogView();

	virtual TextBlockStyle getStyle(const QString & a_styleName) const;

	virtual void addStyle(const TextBlockStyle & a_style,
		bool a_updateExisting = true);

	virtual void addStyle(const QString & a_aliasName,
		const QString & a_title,
		const QString & a_originalStyleName = LOG_STYLE_DEFAULT);

	virtual void startNewBlock();

	virtual qint64 millisecondsToDivideBlocks() const;

	virtual bool setMillisecondsToDivideBlocks(qint64 a_value);

	virtual QStringList styles(bool a_excludeAliases = false) const;

	virtual bool saveHtml(const QString & a_filePath,
		bool a_excludeFiltered = false);

	virtual bool saveHtml(bool a_excludeFiltered = false);

public slots:

	virtual void addEntry(const QString & a_text,
		const QString & a_style = LOG_STYLE_DEFAULT);

	virtual void addEntry(const LogEntry & a_entry);

	virtual void clear();

	virtual void slotSaveHtml();

	virtual void slotSaveHtmlFiltered();

protected slots:

	virtual void slotShowCustomMenu(const QPoint & a_position);

	virtual void slotLogSettings();

	virtual void slotLogSettingsChanged();

protected:

	virtual void updateHtml();

	virtual void createActionsAndMenus();

	virtual QString realHtml(bool a_excludeFiltered = false) const;

	std::vector<TextBlockStyle> m_styles;
	std::vector<LogEntry> m_entries;

	qint64 m_millisecondsToDivideBlocks;

	QMenu * m_pContextMenu;

	StyledLogViewSettingsDialog * m_pSettingsDialog;

	size_t m_maxEntriesToShow;
};

#endif // STYLED_LOG_VIEW_H_INCLUDED
