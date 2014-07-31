#ifndef SCRIPTCOMPLETERMODEL_H
#define SCRIPTCOMPLETERMODEL_H

#include <QStandardItemModel>

#include "../vapoursynth/vsplugindata.h"

class ScriptCompleterModel : public QStandardItemModel
{
	Q_OBJECT

	public:

		ScriptCompleterModel(QObject * a_pParent = nullptr);

		virtual ~ScriptCompleterModel();

		void setPluginsList(const VSPluginsList & a_pluginsList);

		void setCoreName(const QString & a_coreName);

	protected:

	private:

};

#endif // SCRIPTCOMPLETERMODEL_H
