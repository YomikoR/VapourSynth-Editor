#include "job_edit_dialog.h"

#include "../settings/settings_manager.h"

JobEditDialog::JobEditDialog(SettingsManager * a_pSettingsManager,
	QWidget * a_pParent) :
	  QDialog(a_pParent, (Qt::WindowFlags)0
		| Qt::Dialog
		| Qt::CustomizeWindowHint
		| Qt::WindowTitleHint
		| Qt::WindowMinimizeButtonHint
		| Qt::WindowMaximizeButtonHint
		| Qt::WindowCloseButtonHint)
	, m_pSettingsManager(a_pSettingsManager)
{
	m_ui.setupUi(this);
}

JobEditDialog::~JobEditDialog()
{
}
