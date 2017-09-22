#include "trusted_clients_addresses_dialog.h"

#include <QHostAddress>

//==============================================================================

TrustedClientsAddressesDialog::TrustedClientsAddressesDialog(
	QWidget * a_pParent) : QDialog(a_pParent)
{
	m_ui.setupUi(this);

	connect(m_ui.addButton, &QToolButton::clicked,
		this, &TrustedClientsAddressesDialog::slotAddButtonClicked);
	connect(m_ui.removeButton, &QToolButton::clicked,
		this, &TrustedClientsAddressesDialog::slotRemoveButtonClicked);
	connect(m_ui.okButton, &QPushButton::clicked,
		this, &TrustedClientsAddressesDialog::accept);
	connect(m_ui.cancelButton, &QPushButton::clicked,
		this, &TrustedClientsAddressesDialog::reject);
}

// END OF TrustedClientsAddressesDialog::TrustedClientsAddressesDialog(
//		QWidget * a_pParent)
//==============================================================================

TrustedClientsAddressesDialog::~TrustedClientsAddressesDialog()
{
}

// END OF TrustedClientsAddressesDialog::~TrustedClientsAddressesDialog()
//==============================================================================

int TrustedClientsAddressesDialog::call(const QStringList & a_addresses)
{
	m_ui.addressesList->clear();

	QStringList clientAddresses = a_addresses;
	clientAddresses.removeDuplicates();

	for(const QString & address : clientAddresses)
		checkAndAddAddress(address);

	return exec();
}

// END OF int TrustedClientsAddressesDialog::call(
//		const QStringList & a_addresses)
//==============================================================================

QStringList TrustedClientsAddressesDialog::addresses() const
{
	QStringList clientAddresses;
	for(int i = 0; i < m_ui.addressesList->count(); ++i)
		clientAddresses << m_ui.addressesList->item(i)->text();
	return clientAddresses;
}

// END OF QStringList TrustedClientsAddressesDialog::addresses() const
//==============================================================================

void TrustedClientsAddressesDialog::slotAddButtonClicked()
{
	checkAndAddAddress(m_ui.addressEdit->text());
}

// END OF void TrustedClientsAddressesDialog::slotAddButtonClicked()
//==============================================================================

void TrustedClientsAddressesDialog::slotRemoveButtonClicked()
{
	QList<QListWidgetItem *> items = m_ui.addressesList->selectedItems();
	for(QListWidgetItem * pItem : items)
		delete pItem;
}

// END OF void TrustedClientsAddressesDialog::slotRemoveButtonClicked()
//==============================================================================

void TrustedClientsAddressesDialog::checkAndAddAddress(
	const QString & a_address)
{
	QHostAddress hostAddress(a_address);
	if(hostAddress.isNull() && hostAddress.isLoopback())
		return;
	m_ui.addressesList->addItem(a_address);
	m_ui.addressesList->sortItems();
}

// END OF void TrustedClientsAddressesDialog::checkAndAddAddress(
//		const QString & a_address)
//==============================================================================
