#ifndef STYLED_LOG_VIEW_SETTINGS_DIALOG_H_INCLUDED
#define STYLED_LOG_VIEW_SETTINGS_DIALOG_H_INCLUDED

#include <ui_styled_log_view_settings_dialog.h>

#include "styled_log_view_structures.h"

class LogStylesModel;

class StyledLogViewSettingsDialog : public QDialog
{
	Q_OBJECT

public:

	StyledLogViewSettingsDialog(QWidget * a_pParent = nullptr);

	virtual ~StyledLogViewSettingsDialog();

	virtual std::vector<TextBlockStyle> styles() const;

	virtual void setStyles(const std::vector<TextBlockStyle> & a_styles);

signals:

	void signalSettingsChanged();

protected slots:

	virtual void slotOk();

	virtual void slotApply();

	virtual void slotStyleSelected(const QModelIndex & a_index);

	virtual void slotFontButtonClicked();

	virtual void slotTextColorButtonClicked();

	virtual void slotBackgroundColorButtonClicked();

protected:

	virtual void enableStyleSettingsControls(bool a_enable = true);

	Ui::StyledLogViewSettingsDialog m_ui;

	LogStylesModel * m_pLogStylesModel;
};

#endif // STYLED_LOG_VIEW_SETTINGS_DIALOG_H_INCLUDED
