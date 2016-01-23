#ifndef THEME_ELEMENTS_MODEL_H_INCLUDED
#define THEME_ELEMENTS_MODEL_H_INCLUDED


#include <QAbstractItemModel>
#include <QIcon>
#include <vector>

#include "settingsmanager.h"

class SettingsManager;

enum class ThemeElementType
{
	TextCharFormat,
	Color
};

struct ThemeElementData
{
	ThemeElementType type;
	QString id;
	QString text;
	QIcon icon;
	QTextCharFormat textCharFormat;
	QColor color;
};

typedef std::vector<ThemeElementData> ThemeElementsList;

class ThemeElementsModel : public QAbstractItemModel
{
	Q_OBJECT

	public:

		ThemeElementsModel(SettingsManager * a_pSettingsManager,
			QObject * a_pParent = nullptr);

		virtual ~ThemeElementsModel();

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

		void addThemeElement(const ThemeElementData & a_themeElementData);

		void addTextCharFormat(const QString & a_id, const QString & a_text);

		void addColor(const QString & a_id, const QString & a_text);

		void reloadThemeSettings();

		ThemeElementData getThemeElementData(const QString & a_id);

		bool saveThemeElementData(const ThemeElementData & a_themeElementData);

	public slots:

		void slotSaveThemeSettings();

	private:

		ThemeElementsList m_themeElementsList;

		SettingsManager * m_pSettingsManager;

};

#endif // THEME_ELEMENTS_MODEL_H_INCLUDED
