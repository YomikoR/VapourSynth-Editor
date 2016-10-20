#ifndef DROP_FILE_CATEGORY_MODEL_H_INCLUDED
#define DROP_FILE_CATEGORY_MODEL_H_INCLUDED

#include "../settings/settingsmanager.h"

#include <QAbstractItemModel>

class DropFileCategoryModel : public QAbstractItemModel
{
	Q_OBJECT

public:

	DropFileCategoryModel(QObject * a_pParent = nullptr);

	virtual ~DropFileCategoryModel();

	virtual QModelIndex index(int a_row, int a_column,
		const QModelIndex & a_parent = QModelIndex()) const override;

	virtual QModelIndex parent(const QModelIndex & a_child) const override;

	virtual Qt::ItemFlags flags(const QModelIndex & a_index) const override;

	virtual QVariant data(const QModelIndex & a_index,
		int a_role = Qt::DisplayRole) const override;

	virtual QVariant headerData(int a_section, Qt::Orientation a_orientation,
		int a_role = Qt::DisplayRole) const override;

	virtual int rowCount(const QModelIndex & a_parent = QModelIndex())
		const override;

	virtual int columnCount(const QModelIndex & a_parent = QModelIndex())
		const override;

	virtual bool setData(const QModelIndex & a_index, const QVariant & a_value,
		int a_role = Qt::EditRole) override;

	std::vector<DropFileCategory> getCategories() const;

	void setCategories(const std::vector<DropFileCategory> & a_categories);

	void addCategory();

	void deleteCategory(int a_index);

	QString sourceTemplate(int a_index) const;

	void setSourceTemplate(int a_index, const QString & a_text);

private:

	std::vector<DropFileCategory> m_categories;
};

#endif // DROP_FILE_CATEGORY_MODEL_H_INCLUDED
