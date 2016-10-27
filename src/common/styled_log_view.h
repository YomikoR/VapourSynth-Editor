#ifndef STYLED_LOG_VIEW_H_INCLUDED
#define STYLED_LOG_VIEW_H_INCLUDED

#include <QTextEdit>
#include <QDateTime>
#include <vector>

struct TextBlockStyle
{
	QString name;
	QColor backgroundColor;
	QTextCharFormat textFormat;
};

extern const char DEFAULT_LOG_TEXT_BLOCK_STYLE_NAME[];

struct LogEntry
{
	bool isDivider;
	QDateTime time;
	QString text;
	QString style;

	LogEntry(bool a_isDivider = false, const QString & a_text = QString(),
		const QString & a_style = DEFAULT_LOG_TEXT_BLOCK_STYLE_NAME);

	LogEntry(const QString & a_text, const QString & a_style =
		DEFAULT_LOG_TEXT_BLOCK_STYLE_NAME);

	static LogEntry divider();
};

class StyledLogView : public QTextEdit
{
	Q_OBJECT

public:

	StyledLogView(QWidget * a_pParent = nullptr);
	virtual ~StyledLogView();

	virtual TextBlockStyle defaultStyle() const;

	virtual void addStyle(const TextBlockStyle & a_style,
		bool a_updateExisting = true);

	virtual void addEntry(const QString & a_text,
		const QString & a_style = DEFAULT_LOG_TEXT_BLOCK_STYLE_NAME);

	virtual void startNewBlock();

public slots:

	virtual void clear();

protected:

	virtual void updateHtml();

	std::vector<TextBlockStyle> m_styles;
	std::vector<LogEntry> m_entries;
};

#endif // STYLED_LOG_VIEW_H_INCLUDED
