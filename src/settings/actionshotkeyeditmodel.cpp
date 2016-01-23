#include <QAction>

#include "actionshotkeyeditmodel.h"

//==============================================================================

ActionData::ActionData(const QAction * a_pAction):
	id()
	, text()
	, icon()
	, hotkey()
{
	if(a_pAction == nullptr)
		return;

	id = a_pAction->data().toString();
	text = a_pAction->text();
	icon = a_pAction->icon();
	hotkey = a_pAction->shortcut();
}

//==============================================================================

ActionsHotkeyEditModel::ActionsHotkeyEditModel(
	SettingsManager * a_pSettingsManager, QObject * a_pParent) :
	QAbstractItemModel(a_pParent)
	, m_pSettingsManager(a_pSettingsManager)
{

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

	if(a_index.column() == 1)
		cellFlags |= Qt::ItemIsEditable;

	return cellFlags;
}

//==============================================================================

QVariant ActionsHotkeyEditModel::data(const QModelIndex & a_index, int a_role)
	const
{
	if(!a_index.isValid())
		return QVariant();

	if((a_index.row() >= (int)m_actionsList.size()) || (a_index.column() >= 2))
		return QVariant();

	if((a_index.column() == 0) && (a_role == Qt::DecorationRole))
		return m_actionsList[a_index.row()].icon;
	else if((a_index.column() == 0) && ((a_role == Qt::DisplayRole) ||
		(a_role == Qt::ToolTipRole)))
		return m_actionsList[a_index.row()].text;
	else if((a_index.column() == 1) && ((a_role == Qt::DisplayRole) ||
		(a_role == Qt::ToolTipRole) || (a_role == Qt::EditRole)))
		return m_actionsList[a_index.row()].hotkey;

	return QVariant();
}

//==============================================================================

int ActionsHotkeyEditModel::rowCount(const QModelIndex & a_parent) const
{
	(void)a_parent;
	return m_actionsList.size();
}

//==============================================================================

int ActionsHotkeyEditModel::columnCount(const QModelIndex & a_parent) const
{
	(void)a_parent;
	return 2;
}

//==============================================================================

bool ActionsHotkeyEditModel::setData(const QModelIndex & a_index,
	const QVariant & a_value, int a_role)
{
	if((a_index.column() != 1) || (a_role != Qt::EditRole))
		return false;

	m_actionsList[a_index.row()].hotkey = a_value.value<QKeySequence>();
	return true;
}

//==============================================================================

void ActionsHotkeyEditModel::addActions(const ActionDataList & a_actionsList)
{
	emit layoutAboutToBeChanged();
	for(const ActionData & newActionData : a_actionsList)
	{
		bool actionExists = false;
		for(const ActionData & existingActionData : m_actionsList)
		{
			if(newActionData.id == existingActionData.id)
			{
				actionExists = true;
				break;
			}
		}
		if(actionExists)
			continue;

		m_actionsList.push_back(newActionData);
	}
	emit layoutChanged();
}

//==============================================================================

void ActionsHotkeyEditModel::reloadHotkeysSettings()
{
	for(ActionData & actionData : m_actionsList)
		actionData.hotkey = m_pSettingsManager->getHotkey(actionData.id);
	emit dataChanged(createIndex(2, 0),
		createIndex(2, m_actionsList.size() - 1));
}

//==============================================================================

void ActionsHotkeyEditModel::slotSaveActionsHotkeys()
{
	for(const ActionData & actionData : m_actionsList)
		m_pSettingsManager->setHotkey(actionData.id, actionData.hotkey);
}

//==============================================================================
