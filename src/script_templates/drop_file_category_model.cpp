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

//==============================================================================

DropFileCategoryModel::~DropFileCategoryModel()
{

}

//==============================================================================

QModelIndex DropFileCategoryModel::index(int a_row, int a_column,
	const QModelIndex & a_parent) const
{
	(void)a_parent;
	return createIndex(a_row, a_column);
}

//==============================================================================

QModelIndex DropFileCategoryModel::parent(const QModelIndex & a_child) const
{
	(void)a_child;
	return QModelIndex();
}

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

//==============================================================================

QVariant DropFileCategoryModel::headerData(int a_section,
	Qt::Orientation a_orientation, int a_role) const
{
	if(a_orientation != Qt::Horizontal)
		return QVariant();

	if(a_role != Qt::DisplayRole)
		return QVariant();

	if(a_section == NAME_COLUMN)
		return trUtf8("Category");

	if(a_section == MASK_LIST_COLUMN)
		return trUtf8("File name mask list");

	return QVariant();
}

//==============================================================================

int DropFileCategoryModel::rowCount(const QModelIndex & a_parent) const
{
	(void)a_parent;
	return (int)m_categories.size();
}

//==============================================================================

int DropFileCategoryModel::columnCount(const QModelIndex & a_parent) const
{
	(void)a_parent;
	return COLUMNS_NUMBER;
}

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
			QSet<QString> intersection =
				maskList.toSet().intersect(m_categories[i].maskList.toSet());
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

//==============================================================================

std::vector<DropFileCategory> DropFileCategoryModel::getCategories() const
{
	return m_categories;
}

//==============================================================================

void DropFileCategoryModel::setCategories(
	const std::vector<DropFileCategory> & a_categories)
{
	m_categories = a_categories;
	emit layoutChanged();
}

//==============================================================================

void DropFileCategoryModel::addCategory()
{
	beginInsertRows(QModelIndex(), (int)m_categories.size(),
		(int)m_categories.size());
	m_categories.emplace_back();
	endInsertRows();
}

//==============================================================================


void DropFileCategoryModel::deleteCategory(int a_index)
{
	if(a_index >= (int)m_categories.size())
		return;
	beginRemoveRows(QModelIndex(), a_index, a_index);
	m_categories.erase(m_categories.begin() + a_index);
	endRemoveRows();
}

//==============================================================================

QString DropFileCategoryModel::sourceTemplate(int a_index) const
{
	if(a_index >= (int)m_categories.size())
		return QString();
	return m_categories[a_index].sourceTemplate;
}

//==============================================================================

void DropFileCategoryModel::setSourceTemplate(int a_index,
	const QString & a_text)
{
	if(a_index >= (int)m_categories.size())
		return;
	m_categories[a_index].sourceTemplate = a_text;
}

//==============================================================================
