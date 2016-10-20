#include "actionshotkeyeditmodel.h"

#include <cassert>

//==============================================================================

const int COLUMNS_NUMBER = 3;
const int ICON_COLUMN = 0;
const int TITLE_COLUMN = 1;
const int HOTKEY_COLUMN = 2;

//==============================================================================

ActionsHotkeyEditModel::ActionsHotkeyEditModel(
	SettingsManager * a_pSettingsManager, QObject * a_pParent) :
	QAbstractItemModel(a_pParent)
	, m_pSettingsManager(a_pSettingsManager)
{
	assert(m_pSettingsManager);
	m_actions = m_pSettingsManager->getStandardActions();
	reloadHotkeysSettings();
}

//==============================================================================

ActionsHotkeyEditModel::~ActionsHotkeyEditModel()
{

}

//==============================================================================

QModelIndex ActionsHotkeyEditModel::index(int a_row, int a_column,
	const QModelIndex & a_parent) const
{
	(void)a_parent;
	return createIndex(a_row, a_column);
}

//==============================================================================

QModelIndex ActionsHotkeyEditModel::parent(const QModelIndex & a_child) const
{
	(void)a_child;
	return QModelIndex();
}

//==============================================================================

Qt::ItemFlags ActionsHotkeyEditModel::flags(const QModelIndex & a_index) const
{
	if (!a_index.isValid())
	{
		return Qt::NoItemFlags;
	}

	Qt::ItemFlags cellFlags = Qt::NoItemFlags
		| Qt::ItemIsEnabled
		| Qt::ItemIsSelectable
	;

	if(a_index.column() == HOTKEY_COLUMN)
		cellFlags |= Qt::ItemIsEditable;

	return cellFlags;
}

//==============================================================================

QVariant ActionsHotkeyEditModel::data(const QModelIndex & a_index, int a_role)
	const
{
	if(!a_index.isValid())
		return QVariant();

	if((a_index.row() >= (int)m_actions.size()) ||
		(a_index.column() >= COLUMNS_NUMBER))
		return QVariant();

	if((a_index.column() == ICON_COLUMN) && (a_role == Qt::DecorationRole))
		return m_actions[a_index.row()].icon;
	else if((a_index.column() == TITLE_COLUMN) &&
		((a_role == Qt::DisplayRole) || (a_role == Qt::ToolTipRole)))
		return m_actions[a_index.row()].title;
	else if((a_index.column() == HOTKEY_COLUMN) &&
		((a_role == Qt::DisplayRole) || (a_role == Qt::ToolTipRole) ||
		(a_role == Qt::EditRole)))
		return m_actions[a_index.row()].hotkey;

	return QVariant();
}

//==============================================================================

int ActionsHotkeyEditModel::rowCount(const QModelIndex & a_parent) const
{
	(void)a_parent;
	return (int)m_actions.size();
}

//==============================================================================

int ActionsHotkeyEditModel::columnCount(const QModelIndex & a_parent) const
{
	(void)a_parent;
	return COLUMNS_NUMBER;
}

//==============================================================================

bool ActionsHotkeyEditModel::setData(const QModelIndex & a_index,
	const QVariant & a_value, int a_role)
{
	if((a_index.column() != HOTKEY_COLUMN) || (a_role != Qt::EditRole))
		return false;

	m_actions[a_index.row()].hotkey = a_value.value<QKeySequence>();
	return true;
}

//==============================================================================

void ActionsHotkeyEditModel::reloadHotkeysSettings()
{
	for(StandardAction & action : m_actions)
		action.hotkey = m_pSettingsManager->getHotkey(action.id);
	emit dataChanged(createIndex(HOTKEY_COLUMN, 0),
		createIndex(HOTKEY_COLUMN, (int)m_actions.size() - 1));
}

//==============================================================================

void ActionsHotkeyEditModel::slotSaveActionsHotkeys()
{
	for(const StandardAction & action : m_actions)
		m_pSettingsManager->setHotkey(action.id, action.hotkey);
}

//==============================================================================
