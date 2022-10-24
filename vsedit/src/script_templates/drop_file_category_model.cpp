#include "drop_file_category_model.h"

#include <QRegExp>
#include <QSet>

//==============================================================================

const int COLUMNS_NUMBER = 2;
const int NAME_COLUMN = 0;
const int MASK_LIST_COLUMN = 1;

const char MASK_LIST_SPLITTER = ';';

//==============================================================================

DropFileCategoryModel::DropFileCategoryModel(QObject * a_pParent) :
	QAbstractItemModel(a_pParent)
{
}

// END OF DropFileCategoryModel::DropFileCategoryModel(QObject * a_pParent) :
//		QAbstractItemModel(a_pParent)
//==============================================================================

DropFileCategoryModel::~DropFileCategoryModel()
{

}

// END OF DropFileCategoryModel::~DropFileCategoryModel()
//==============================================================================

QModelIndex DropFileCategoryModel::index(int a_row, int a_column,
	const QModelIndex & a_parent) const
{
	(void)a_parent;
	return createIndex(a_row, a_column);
}

// END OF QModelIndex DropFileCategoryModel::index(int a_row, int a_column,
//		const QModelIndex & a_parent) const
//==============================================================================

QModelIndex DropFileCategoryModel::parent(const QModelIndex & a_child) const
{
	(void)a_child;
	return QModelIndex();
}

// END OF QModelIndex DropFileCategoryModel::parent(
//		const QModelIndex & a_child) const
//==============================================================================

Qt::ItemFlags DropFileCategoryModel::flags(const QModelIndex & a_index) const
{
	if (!a_index.isValid())
	{
		return Qt::NoItemFlags;
	}

	Qt::ItemFlags cellFlags = Qt::NoItemFlags
		| Qt::ItemIsEnabled
		| Qt::ItemIsSelectable
		| Qt::ItemIsEditable
		| Qt::ItemNeverHasChildren
	;

	return cellFlags;
}

// END OF Qt::ItemFlags DropFileCategoryModel::flags(
//		const QModelIndex & a_index) const
//==============================================================================

QVariant DropFileCategoryModel::data(const QModelIndex & a_index, int a_role)
	const
{
	if(!a_index.isValid())
		return QVariant();

	if((a_index.row() >= (int)m_categories.size()) ||
		(a_index.column() >= COLUMNS_NUMBER))
		return QVariant();

	if((a_index.column() == NAME_COLUMN) &&
		((a_role == Qt::DisplayRole) || (a_role == Qt::ToolTipRole) ||
		(a_role == Qt::EditRole)))
		return m_categories[a_index.row()].name;
	else if((a_index.column() == MASK_LIST_COLUMN) &&
		((a_role == Qt::DisplayRole) || (a_role == Qt::ToolTipRole) ||
		(a_role == Qt::EditRole)))
		return m_categories[a_index.row()].maskList.join(MASK_LIST_SPLITTER);

	return QVariant();
}

// END OF QVariant DropFileCategoryModel::data(const QModelIndex & a_index,
//		int a_role)
//==============================================================================

QVariant DropFileCategoryModel::headerData(int a_section,
	Qt::Orientation a_orientation, int a_role) const
{
	if(a_orientation != Qt::Horizontal)
		return QVariant();

	if(a_role != Qt::DisplayRole)
		return QVariant();

	if(a_section == NAME_COLUMN)
		return tr("Category");

	if(a_section == MASK_LIST_COLUMN)
		return tr("File name mask list");

	return QVariant();
}

// END OF QVariant DropFileCategoryModel::headerData(int a_section,
//		Qt::Orientation a_orientation, int a_role) const
//==============================================================================

int DropFileCategoryModel::rowCount(const QModelIndex & a_parent) const
{
	(void)a_parent;
	return (int)m_categories.size();
}

// END OF int DropFileCategoryModel::rowCount(const QModelIndex & a_parent)
//		const
//==============================================================================

int DropFileCategoryModel::columnCount(const QModelIndex & a_parent) const
{
	(void)a_parent;
	return COLUMNS_NUMBER;
}

// END OF int DropFileCategoryModel::columnCount(const QModelIndex & a_parent)
//		const
//==============================================================================

bool DropFileCategoryModel::setData(const QModelIndex & a_index,
	const QVariant & a_value, int a_role)
{
	if(a_role != Qt::EditRole)
		return false;

	if((a_index.row() >= (int)m_categories.size()) ||
		(a_index.column() >= COLUMNS_NUMBER))
		return false;

	if(a_index.column() == NAME_COLUMN)
	{
		for(int i = 0; i < (int)m_categories.size(); ++i)
		{
			if((i != a_index.row()) &&
				(m_categories[i].name == a_value.toString()))
				return false;
		}
		m_categories[a_index.row()].name = a_value.toString();
		return true;
	}

	if(a_index.column() == MASK_LIST_COLUMN)
	{
		QStringList maskList = a_value.toString().split(MASK_LIST_SPLITTER);
		for(int i = 0; i < (int)m_categories.size(); ++i)
		{
			if(i == a_index.row())
				continue;
			QSet<QString> set_curr(maskList.begin(), maskList.end());
			QSet<QString> set_i(m_categories[i].maskList.begin(),
				m_categories[i].maskList.end());
			QSet<QString> intersection = set_curr.intersect(set_i);
			if(intersection.size() > 0)
				return false;
		}

		QRegExp matcher;
		matcher.setPatternSyntax(QRegExp::Wildcard);

		for(const QString & mask : maskList)
		{
			matcher.setPattern(mask);
			if(!matcher.isValid())
				return false;
		}

		m_categories[a_index.row()].maskList = maskList;
		return true;
	}

	return false;
}

// END OF bool DropFileCategoryModel::setData(const QModelIndex & a_index,
//		const QVariant & a_value, int a_role)
//==============================================================================

std::vector<DropFileCategory> DropFileCategoryModel::getCategories() const
{
	return m_categories;
}

// END OF std::vector<DropFileCategory> DropFileCategoryModel::getCategories()
//		const
//==============================================================================

void DropFileCategoryModel::setCategories(
	const std::vector<DropFileCategory> & a_categories)
{
	m_categories = a_categories;
	emit layoutChanged();
}

// END OF void DropFileCategoryModel::setCategories(
//		const std::vector<DropFileCategory> & a_categories)
//==============================================================================

void DropFileCategoryModel::addCategory()
{
	beginInsertRows(QModelIndex(), (int)m_categories.size(),
		(int)m_categories.size());
	m_categories.emplace_back();
	endInsertRows();
}

// END OF void DropFileCategoryModel::addCategory()
//==============================================================================

void DropFileCategoryModel::deleteCategory(int a_index)
{
	if(a_index >= (int)m_categories.size() || a_index < 0)
		return;
	beginRemoveRows(QModelIndex(), a_index, a_index);
	m_categories.erase(m_categories.begin() + a_index);
	endRemoveRows();
}

// END OF void DropFileCategoryModel::deleteCategory(int a_index)
//==============================================================================

QString DropFileCategoryModel::sourceTemplate(int a_index) const
{
	if(a_index >= (int)m_categories.size() || a_index < 0)
		return QString();
	return m_categories[a_index].sourceTemplate;
}

// END OF QString DropFileCategoryModel::sourceTemplate(int a_index) const
//==============================================================================

void DropFileCategoryModel::setSourceTemplate(int a_index,
	const QString & a_text)
{
	if(a_index >= (int)m_categories.size() || a_index < 0)
		return;
	m_categories[a_index].sourceTemplate = a_text;
}

// END OF void DropFileCategoryModel::setSourceTemplate(int a_index,
//		const QString & a_text)
//==============================================================================
