#ifndef ACTIONSHOTKEYEDITMODEL_H
#define ACTIONSHOTKEYEDITMODEL_H

#include "../../../common-src/settings/settings_manager.h"

#include <QAbstractItemModel>

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

	void reloadHotkeysSettings();

public slots:

	void slotSaveActionsHotkeys();

private:

	std::vector<StandardAction> m_actions;

	SettingsManager * m_pSettingsManager;
};

#endif // ACTIONSHOTKEYEDITMODEL_H
