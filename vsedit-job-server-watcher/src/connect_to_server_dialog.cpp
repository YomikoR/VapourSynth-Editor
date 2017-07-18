#include "connect_to_server_dialog.h"

ConnectToServerDialog::ConnectToServerDialog(QWidget * a_pParent) :
	QDialog(a_pParent)
{
	m_ui.setupUi(this);

	connect(m_ui.cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

ConnectToServerDialog::~ConnectToServerDialog()
{
}
