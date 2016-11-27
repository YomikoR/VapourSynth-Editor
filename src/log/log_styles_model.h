#ifndef LOG_STYLES_MODEL_H_INCLUDED
#define LOG_STYLES_MODEL_H_INCLUDED

#include "styled_log_view_structures.h"

#include <QAbstractItemModel>

class LogStylesModel : public QAbstractItemModel
{
	Q_OBJECT

public:

	LogStylesModel(QObject * a_pParent = nullptr);

	virtual ~LogStylesModel();

	virtual QModelIndex index(int a_row, int a_column,
		const QModelIndex & a_parent = QModelIndex()) const override;

	virtual QModelIndex parent(const QModelIndex & a_child) const override;

	virtual Qt::ItemFlags flags(const QModelIndex & a_index) const override;

	virtual QVariant data(const QModelIndex & a_index,
		int a_role = Qt::DisplayRole) const override;

	virtual int rowCount(const QModelIndex & a_parent = QModelIndex()) const
		override;

	virtual int columnCount(const QModelIndex & a_parent = QModelIndex()) const
		override;

	virtual bool setData(const QModelIndex & a_index, const QVariant & a_value,
		int a_role = Qt::EditRole) override;

	virtual std::vector<TextBlockStyle> styles() const;

	virtual void setStyles(const std::vector<TextBlockStyle> & a_styles);

	virtual TextBlockStyle style(int a_index) const;
	virtual TextBlockStyle style(const QModelIndex & a_index) const;

	virtual bool setStyleFont(int a_index, const QFont & a_font);
	virtual bool setStyleTextColor(int a_index, const QColor & a_color);
	virtual bool setStyleBackgroundColor(int a_index, const QColor & a_color);

protected:

	virtual bool styleIndexValid(int a_index) const;

	std::vector<TextBlockStyle> m_styles;
};

#endif // LOG_STYLES_MODEL_H_INCLUDED
