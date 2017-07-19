#include "connect_to_server_dialog.h"

#include "../../common-src/settings/settings_manager.h"

#include <QComboBox>
#include <cassert>

ConnectToServerDialog::ConnectToServerDialog(
	SettingsManager * a_pSettingsManager, QWidget * a_pParent) :
	  QDialog(a_pParent)
	, m_pSettingsManager(a_pSettingsManager)
{
	assert(m_pSettingsManager);
	m_ui.setupUi(this);

	connect(m_ui.connectButton, SIGNAL(clicked()),
		this, SLOT(slotConnectButtonClicked()));
	connect(m_ui.cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

ConnectToServerDialog::~ConnectToServerDialog()
{
}

int ConnectToServerDialog::call(const QHostAddress & a_address)
{
	m_ui.serverAddressComboBox->setCurrentText(a_address.toString());
    return exec();
}

void ConnectToServerDialog::slotConnectButtonClicked()
{
	QHostAddress host(m_ui.serverAddressComboBox->currentText());
	if(host.isNull())
		return;
	emit signalConnectToServer(host);
	accept();
}
