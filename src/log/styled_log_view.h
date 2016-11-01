#ifndef STYLED_LOG_VIEW_H_INCLUDED
#define STYLED_LOG_VIEW_H_INCLUDED

#include <QTextEdit>
#include <QDateTime>
#include <vector>
#include <map>

//==============================================================================

struct TextBlockStyle
{
	QString name;
	QColor backgroundColor;
	QTextCharFormat textFormat;
};

//==============================================================================

extern const char LOG_STYLE_DEFAULT[];

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
};

//==============================================================================

class StyledLogView : public QTextEdit
{
	Q_OBJECT

public:

	StyledLogView(QWidget * a_pParent = nullptr);
	virtual ~StyledLogView();

	virtual TextBlockStyle defaultStyle() const;

	virtual TextBlockStyle getStyle(const QString & a_styleName) const;

	virtual void addStyle(const TextBlockStyle & a_style,
		bool a_updateExisting = true);

	virtual void addStyle(const QString & a_styleName,
		const QString & a_existingStyleName = LOG_STYLE_DEFAULT);

	virtual void addEntry(const QString & a_text,
		const QString & a_style = LOG_STYLE_DEFAULT);

	virtual void startNewBlock();

public slots:

	virtual void clear();

protected:

	virtual void updateHtml();

	std::vector<TextBlockStyle> m_styles;
	std::vector<LogEntry> m_entries;

	std::map<QString, QString> m_styleAliases;
};

//==============================================================================

#endif // STYLED_LOG_VIEW_H_INCLUDED
