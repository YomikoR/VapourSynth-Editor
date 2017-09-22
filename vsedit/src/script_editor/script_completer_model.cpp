#include "script_completer_model.h"

#include <algorithm>

//==============================================================================

const char DEFAULT_CORE_NAME[] = "core";

//==============================================================================

ScriptCompleterModel::ScriptCompleterModel(QObject * a_pParent):
	QStandardItemModel(a_pParent)
{
	setCoreName(DEFAULT_CORE_NAME);
}

// END OF ScriptCompleterModel::ScriptCompleterModel(QObject * a_pParent)
//==============================================================================

ScriptCompleterModel::~ScriptCompleterModel()
{

}

// END OF ScriptCompleterModel::~ScriptCompleterModel()
//==============================================================================

void ScriptCompleterModel::setPluginsList(const VSPluginsList & a_pluginsList)
{
	if(invisibleRootItem()->rowCount() < 1)
		setCoreName(DEFAULT_CORE_NAME);

	QStandardItem * pCoreItem = invisibleRootItem()->child(0, 0);
	pCoreItem->removeRows(0, pCoreItem->rowCount());

	for(const VSData::Plugin & plugin : a_pluginsList)
	{
		QStandardItem * pPluginItem = new QStandardItem(plugin.pluginNamespace);
		pCoreItem->appendRow(pPluginItem);
		for(const VSData::Function & function : plugin.functions)
		{
			QStringList argumentsList;
			for(const VSData::FunctionArgument & argument : function.arguments)
				argumentsList << argument.name;
			QString signature = QString("%1(%2)").arg(function.name)
				.arg(argumentsList.join(", "));

			QStandardItem * pFunctionItem = new QStandardItem(signature);
			pPluginItem->appendRow(pFunctionItem);
		}
	}
}

// END OF void ScriptCompleterModel::setPluginsList(
//		const VSPluginsList & a_pluginsList)
//==============================================================================

void ScriptCompleterModel::setCoreName(const QString & a_coreName)
{
	QStandardItem * pRootItem = invisibleRootItem();
	if(pRootItem->rowCount() == 0)
	{
		QStandardItem * pCoreItem = new QStandardItem(a_coreName);
		pRootItem->appendRow(pCoreItem);
	}
	else
	{
		QStandardItem * pCoreItem = pRootItem->child(0, 0);
		pCoreItem->setText(a_coreName);
	}
}

// END OF void ScriptCompleterModel::setCoreName(const QString & a_coreName)
//==============================================================================
