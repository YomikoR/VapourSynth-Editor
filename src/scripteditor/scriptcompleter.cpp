#include "scriptcompleter.h"

ScriptCompleter::ScriptCompleter(QAbstractItemModel * a_pModel,
	QObject * a_pParent) : QCompleter(a_pModel, a_pParent)
{

}

ScriptCompleter::~ScriptCompleter()
{

}

QString ScriptCompleter::pathFromIndex(const QModelIndex & a_index) const
{
	if(!a_index.isValid())
		return QString();

	QString path = model()->data(a_index, Qt::EditRole).toString();
	QModelIndex index = a_index;
	while(index.parent().isValid())
	{
		index = index.parent();
		path.prepend('.');
		path.prepend(model()->data(index, Qt::EditRole).toString());
	}

	return path;
}

QStringList ScriptCompleter::splitPath(const QString & a_path) const
{
	return a_path.split('.');
}
