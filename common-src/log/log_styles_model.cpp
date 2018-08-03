#include "log_styles_model.h"

//==============================================================================

LogStylesModel::LogStylesModel(QObject * a_pParent) :
	QAbstractItemModel(a_pParent)
{
}

// END OF LogStylesModel::LogStylesModel(QObject * a_pParent)
//==============================================================================

LogStylesModel::~LogStylesModel()
{
}

// END OF LogStylesModel::~LogStylesModel()
//==============================================================================

QModelIndex LogStylesModel::index(int a_row, int a_column,
	const QModelIndex & a_parent) const
{
	(void)a_parent;
	return createIndex(a_row, a_column);
}

// END OF QModelIndex LogStylesModel::index(int a_row, int a_column,
//		const QModelIndex & a_parent) const
//==============================================================================

QModelIndex LogStylesModel::parent(const QModelIndex & a_child) const
{
	(void)a_child;
	return QModelIndex();
}

// END OF QModelIndex LogStylesModel::parent(const QModelIndex & a_child)
//		const
//==============================================================================

Qt::ItemFlags LogStylesModel::flags(const QModelIndex & a_index) const
{
	if (!a_index.isValid())
	{
		return Qt::NoItemFlags;
	}

	Qt::ItemFlags cellFlags = Qt::NoItemFlags
		| Qt::ItemIsEnabled
		| Qt::ItemIsSelectable
		| Qt::ItemIsUserCheckable
	;

	return cellFlags;
}

// END OF Qt::ItemFlags LogStylesModel::flags(const QModelIndex & a_index)
//		const
//==============================================================================

QVariant LogStylesModel::data(const QModelIndex & a_index, int a_role) const
{
	if(!a_index.isValid())
		return QVariant();

	int row = a_index.row();
	int column = a_index.column();

	if((row >= (int)m_styles.size()) || (column >= 1))
		return QVariant();

	if((a_role == Qt::DisplayRole) || (a_role == Qt::ToolTipRole))
		return m_styles[row].title;
	else if(a_role == Qt::UserRole)
		return m_styles[row].name;
	else if(a_role == Qt::CheckStateRole)
		return m_styles[row].isVisible ? Qt::Checked : Qt::Unchecked;

	return QVariant();
}

// END OF QVariant LogStylesModel::data(const QModelIndex & a_index,
//		int a_role) const
//==============================================================================

int LogStylesModel::rowCount(const QModelIndex & a_parent) const
{
	(void)a_parent;
	return (int)m_styles.size();
}

// END OF int LogStylesModel::rowCount(const QModelIndex & a_parent) const
//==============================================================================

int LogStylesModel::columnCount(const QModelIndex & a_parent) const
{
	(void)a_parent;
	return 1;
}

// END OF int LogStylesModel::columnCount(const QModelIndex & a_parent)
//		const
//==============================================================================

bool LogStylesModel::setData(const QModelIndex & a_index,
	const QVariant & a_value, int a_role)
{
	if(!a_index.isValid())
		return false;

	int row = a_index.row();
	int column = a_index.column();

	if((row >= (int)m_styles.size()) || (column >= 1))
		return false;

	if(a_role == Qt::CheckStateRole)
	{
		m_styles[row].isVisible = (a_value == Qt::Checked) ? true : false;
		return true;
	}

	return false;
}

// END OF bool LogStylesModel::setData(const QModelIndex & a_index,
//		const QVariant & a_value, int a_role)
//==============================================================================

std::vector<TextBlockStyle> LogStylesModel::styles() const
{
	return m_styles;
}

// END OF std::vector<TextBlockStyle> LogStylesModel::styles() const
//==============================================================================

void LogStylesModel::setStyles(const std::vector<TextBlockStyle> & a_styles)
{
	beginResetModel();
	m_styles = a_styles;
	endResetModel();
}

// END OF void LogStylesModel::setStyles(
//		const std::vector<TextBlockStyle> & a_styles)
//==============================================================================

TextBlockStyle LogStylesModel::style(int a_index) const
{
	Q_ASSERT(styleIndexValid(a_index));
	return m_styles[a_index];
}

// END OF TextBlockStyle LogStylesModel::style(int a_index) const
//==============================================================================

TextBlockStyle LogStylesModel::style(const QModelIndex & a_index) const
{
	return style(a_index.row());
}

// END OF TextBlockStyle LogStylesModel::style(const QModelIndex & a_index)
//		const
//==============================================================================

bool LogStylesModel::setStyleFont(int a_index, const QFont & a_font)
{
	if(!styleIndexValid(a_index))
		return false;

	m_styles[a_index].textFormat.setFont(a_font);
	return true;
}

// END OF bool LogStylesModel::setStyleFont(int a_index, const QFont & a_font)
//==============================================================================

bool LogStylesModel::setStyleTextColor(int a_index, const QColor & a_color)
{
	if(!styleIndexValid(a_index))
		return false;

	m_styles[a_index].textFormat.setForeground(a_color);
	return true;
}

// END OF bool LogStylesModel::setStyleTextColor(int a_index,
//		const QColor & a_color)
//==============================================================================

bool LogStylesModel::setStyleBackgroundColor(int a_index,
	const QColor & a_color)
{
	if(!styleIndexValid(a_index))
		return false;

	m_styles[a_index].textFormat.setBackground(a_color);
	return true;
}

// END OF bool LogStylesModel::setStyleBackgroundColor(int a_index,
//		const QColor & a_color)
//==============================================================================

bool LogStylesModel::styleIndexValid(int a_index) const
{
	return ((a_index >= 0) && (a_index < (int)m_styles.size()));
}

// END OF bool LogStylesModel::styleIndexValid(int a_index) const
//==============================================================================
