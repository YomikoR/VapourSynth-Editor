#include "theme_elements_model.h"

//==============================================================================

ThemeElementsModel::ThemeElementsModel(SettingsManager * a_pSettingsManager,
	QObject * a_pParent) : QAbstractItemModel(a_pParent)
	, m_pSettingsManager(a_pSettingsManager)
{

}

//==============================================================================

ThemeElementsModel::~ThemeElementsModel()
{

}

//==============================================================================


QModelIndex ThemeElementsModel::index(int a_row, int a_column,
	const QModelIndex & a_parent) const
{
	(void)a_parent;
	return createIndex(a_row, a_column);
}

//==============================================================================


QModelIndex ThemeElementsModel::parent(const QModelIndex & a_child) const
{
	(void)a_child;
	return QModelIndex();
}

//==============================================================================


Qt::ItemFlags ThemeElementsModel::flags(const QModelIndex & a_index) const
{
	if (!a_index.isValid())
	{
		return Qt::NoItemFlags;
	}

	Qt::ItemFlags cellFlags = Qt::NoItemFlags
		| Qt::ItemIsEnabled
		| Qt::ItemIsSelectable
	;

	return cellFlags;
}

//==============================================================================

QVariant ThemeElementsModel::data(const QModelIndex & a_index, int a_role) const
{
	if(!a_index.isValid())
		return QVariant();

	if((a_index.row() >= (int)m_themeElementsList.size()) ||
		(a_index.column() >= 1))
		return QVariant();

	if(a_role == Qt::DecorationRole)
		return m_themeElementsList[a_index.row()].icon;
	else if((a_role == Qt::DisplayRole) ||
		(a_role == Qt::ToolTipRole))
		return m_themeElementsList[a_index.row()].text;
	else if(a_role == Qt::UserRole)
		return m_themeElementsList[a_index.row()].id;

	return QVariant();
}

//==============================================================================


int ThemeElementsModel::rowCount(const QModelIndex & a_parent) const
{
	(void)a_parent;
	return m_themeElementsList.size();
}

//==============================================================================

int ThemeElementsModel::columnCount(const QModelIndex & a_parent) const
{
	(void)a_parent;
	return 2;
}

//==============================================================================

bool ThemeElementsModel::setData(const QModelIndex & a_index,
	const QVariant & a_value, int a_role)
{
	(void)a_index;
	(void)a_value;
	(void)a_role;
	return false;
}

//==============================================================================

void ThemeElementsModel::addThemeElement(
	const ThemeElementData & a_themeElementData)
{
	for(size_t i = 0; i < m_themeElementsList.size(); ++i)
	{
		if(m_themeElementsList[i].id == a_themeElementData.id)
			return;
	}

	m_themeElementsList.push_back(a_themeElementData);
	emit layoutChanged();
}

//==============================================================================

void ThemeElementsModel::addTextCharFormat(const QString & a_id,
	const QString & a_text)
{
	ThemeElementData newThemeElementData;
	newThemeElementData.type = ThemeElementType::TextCharFormat;
	newThemeElementData.id = a_id;
	newThemeElementData.text = a_text;
	newThemeElementData.icon = QIcon(QString(":font.png"));
	newThemeElementData.textCharFormat =
		m_pSettingsManager->getTextFormat(a_id);
	addThemeElement(newThemeElementData);
}

//==============================================================================

void ThemeElementsModel::addColor(const QString & a_id,
	const QString & a_text)
{
	ThemeElementData newThemeElementData;
	newThemeElementData.type = ThemeElementType::Color;
	newThemeElementData.id = a_id;
	newThemeElementData.text = a_text;
	newThemeElementData.icon = QIcon(QString(":color_swatch.png"));
	newThemeElementData.color = m_pSettingsManager->getColor(a_id);
	addThemeElement(newThemeElementData);
}

//==============================================================================

void ThemeElementsModel::reloadThemeSettings()
{
	for(size_t i = 0; i < m_themeElementsList.size(); ++i)
	{
		if(m_themeElementsList[i].type == ThemeElementType::TextCharFormat)
		{
			m_themeElementsList[i].textCharFormat =
				m_pSettingsManager->getTextFormat(m_themeElementsList[i].id);
		}
		else if(m_themeElementsList[i].type == ThemeElementType::Color)
		{
			m_themeElementsList[i].color =
				m_pSettingsManager->getColor(m_themeElementsList[i].id);
		}
	}
}

//==============================================================================

ThemeElementData ThemeElementsModel::getThemeElementData(const QString & a_id)
{
	for(size_t i = 0; i < m_themeElementsList.size(); ++i)
	{
		if(m_themeElementsList[i].id == a_id)
			return m_themeElementsList[i];
	}

	return ThemeElementData();
}

//==============================================================================

bool ThemeElementsModel::saveThemeElementData(
	const ThemeElementData & a_themeElementData)
{
	for(size_t i = 0; i < m_themeElementsList.size(); ++i)
	{
		if(m_themeElementsList[i].id != a_themeElementData.id)
			continue;

		m_themeElementsList[i].textCharFormat =
			a_themeElementData.textCharFormat;
		m_themeElementsList[i].color = a_themeElementData.color;
		return true;
	}

	return false;
}

//==============================================================================

void ThemeElementsModel::slotSaveThemeSettings()
{
	for(size_t i = 0; i < m_themeElementsList.size(); ++i)
	{
		if(m_themeElementsList[i].type == ThemeElementType::TextCharFormat)
		{
			m_pSettingsManager->setTextFormat(m_themeElementsList[i].id,
				m_themeElementsList[i].textCharFormat);
		}
		else if(m_themeElementsList[i].type == ThemeElementType::Color)
		{
			m_pSettingsManager->setColor(m_themeElementsList[i].id,
				m_themeElementsList[i].color);
		}
	}
}

//==============================================================================
