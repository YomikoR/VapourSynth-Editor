#include "connect_to_server_dialog.h"

#include "../../common-src/settings/settings_manager.h"

#include <QComboBox>

ConnectToServerDialog::ConnectToServerDialog(
	SettingsManager * a_pSettingsManager, QWidget * a_pParent) :
	  QDialog(a_pParent)
	, m_pSettingsManager(a_pSettingsManager)
{
	Q_ASSERT(m_pSettingsManager);
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
	m_ui.serverAddressComboBox->clear();
	QStringList recentServers = m_pSettingsManager->getRecentJobServers();
	m_ui.serverAddressComboBox->addItems(recentServers);
	m_ui.serverAddressComboBox->setCurrentText(a_address.toString());
    return exec();
}

void ConnectToServerDialog::slotConnectButtonClicked()
{
	QString address = m_ui.serverAddressComboBox->currentText();
	QHostAddress host(address);
	if(host.isNull())
		return;
	int index = -1;
	while((index = m_ui.serverAddressComboBox->findText(address)) >= 0)
	{
		m_ui.serverAddressComboBox->removeItem(index);
	}
	m_ui.serverAddressComboBox->insertItem(0, address);
	saveServersList();
	accept();
	emit signalConnectToServer(host);
}

void ConnectToServerDialog::saveServersList()
{
	QStringList recentServers;
	for(int i = 0; i < m_ui.serverAddressComboBox->count(); ++i)
	{
		QString address = m_ui.serverAddressComboBox->itemText(i);
		if(QHostAddress(address).isNull())
			continue;
		recentServers << address;
	}
	m_pSettingsManager->setRecentJobServers(recentServers);
}
