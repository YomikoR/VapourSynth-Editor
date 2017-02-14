#include "jobs_model.h"

#include "../settings/settings_manager.h"

#include <cassert>

//==============================================================================

const int COLUMNS_NUMBER = 4;
const int NAME_COLUMN = 0;
const int TYPE_COLUMN = 1;
const int STATUS_COLUMN = 2;
const int DEPENDS_ON_COLUMN = 3;

//==============================================================================

JobsModel::JobsModel(SettingsManager * a_pSettingsManager, QObject * a_pParent):
	  QAbstractItemModel(a_pParent)
	, m_pSettingsManager(a_pSettingsManager)
{
	assert(m_pSettingsManager);
}

// END OF JobsModel::JobsModel(SettingsManager * a_pSettingsManager,
//		QObject * a_pParent)
//==============================================================================

JobsModel::~JobsModel()
{
}

// END OF JobsModel::~JobsModel()
//==============================================================================

QModelIndex JobsModel::index(int a_row, int a_column,
	const QModelIndex & a_parent) const
{
	(void)a_parent;
	return createIndex(a_row, a_column);
}

// END OF QModelIndex JobsModel::index(int a_row, int a_column,
//		const QModelIndex & a_parent) const
//==============================================================================

QModelIndex JobsModel::parent(const QModelIndex & a_child) const
{
	(void)a_child;
	return QModelIndex();
}

// END OF QModelIndex JobsModel::parent(const QModelIndex & a_child) const
//==============================================================================

Qt::ItemFlags JobsModel::flags(const QModelIndex & a_index) const
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

// END OF Qt::ItemFlags JobsModel::flags(const QModelIndex & a_index) const
//==============================================================================

QVariant JobsModel::data(const QModelIndex & a_index, int a_role) const
{
	return QVariant();
}

// END OF QVariant JobsModel::data(const QModelIndex & a_index, int a_role)
//		const
//==============================================================================

int JobsModel::rowCount(const QModelIndex & a_parent) const
{
	(void)a_parent;
	return (int)m_jobs.size();
}

// END OF int JobsModel::rowCount(const QModelIndex & a_parent) const
//==============================================================================

int JobsModel::columnCount(const QModelIndex & a_parent) const
{
	(void)a_parent;
	return COLUMNS_NUMBER;
}

// END OF int JobsModel::columnCount(const QModelIndex & a_parent) const
//==============================================================================

bool JobsModel::setData(const QModelIndex & a_index, const QVariant & a_value,
	int a_role)
{
	(void)a_index;
	(void)a_value;
	(void)a_role;
	return false;
}

// END OF bool JobsModel::setData(const QModelIndex & a_index,
//		const QVariant & a_value, int a_role)
//==============================================================================
