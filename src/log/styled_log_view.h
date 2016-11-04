#ifndef STYLED_LOG_VIEW_H_INCLUDED
#define STYLED_LOG_VIEW_H_INCLUDED

#include "styled_log_view_structures.h"

#include <QTextEdit>
#include <vector>

class StyledLogView : public QTextEdit
{
	Q_OBJECT

public:

	StyledLogView(QWidget * a_pParent = nullptr);
	virtual ~StyledLogView();

	virtual TextBlockStyle getStyle(const QString & a_styleName) const;

	virtual void addStyle(const TextBlockStyle & a_style,
		bool a_updateExisting = true);

	virtual void addStyle(const QString & a_aliasName,
		const QString & a_title,
		const QString & a_originalStyleName = LOG_STYLE_DEFAULT);

	virtual void addEntry(const QString & a_text,
		const QString & a_style = LOG_STYLE_DEFAULT);

	virtual void startNewBlock();

public slots:

	virtual void clear();

protected:

	virtual void updateHtml();

	std::vector<TextBlockStyle> m_styles;
	std::vector<LogEntry> m_entries;
};

#endif // STYLED_LOG_VIEW_H_INCLUDED
