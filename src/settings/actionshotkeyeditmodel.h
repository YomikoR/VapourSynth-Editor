#ifndef ACTIONSHOTKEYEDITMODEL_H
#define ACTIONSHOTKEYEDITMODEL_H

#include <QAbstractItemModel>
#include <QKeySequence>
#include <QString>
#include <QIcon>
#include <vector>

#include "settingsmanager.h"

class SettingsManager;
class QAction;

struct ActionData
{
	ActionData(const QAction * a_pAction);
	QString id;
	QString text;
	QIcon icon;
	QKeySequence hotkey;
};

typedef std::vector<ActionData> ActionDataList;

class ActionsHotkeyEditModel : public QAbstractItemModel
{
	Q_OBJECT

	public:

		ActionsHotkeyEditModel(SettingsManager * a_pSettingsManager,
			QObject * a_pParent = nullptr);

		virtual ~ActionsHotkeyEditModel();

		QModelIndex index(int a_row, int a_column,
			const QModelIndex & a_parent = QModelIndex()) const override;

		QModelIndex parent(const QModelIndex & a_child) const override;

		Qt::ItemFlags flags(const QModelIndex & a_index) const override;

		QVariant data(const QModelIndex & a_index, int a_role = Qt::DisplayRole)
			const override;

		int rowCount(const QModelIndex & a_parent = QModelIndex()) const
			override;

		int columnCount(const QModelIndex & a_parent = QModelIndex()) const
			override;

		bool setData(const QModelIndex & a_index, const QVariant & a_value,
			int a_role = Qt::EditRole) override;

		void addActions(const ActionDataList & a_actionsList);

		void reloadHotkeysSettings();

	public slots:

		void slotSaveActionsHotkeys();

	private:

		ActionDataList m_actionsList;

		SettingsManager * m_pSettingsManager;

};

#endif // ACTIONSHOTKEYEDITMODEL_H
