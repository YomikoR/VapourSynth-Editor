#ifndef TRUSTED_CLIENTS_ADDRESSES_DIALOG_H_INCLUDED
#define TRUSTED_CLIENTS_ADDRESSES_DIALOG_H_INCLUDED

#include <ui_trusted_clients_addresses_dialog.h>

#include <QStringList>

class TrustedClientsAddressesDialog : public QDialog
{
	Q_OBJECT

public:

	TrustedClientsAddressesDialog(QWidget * a_pParent = nullptr);
	virtual ~TrustedClientsAddressesDialog();

	int call(const QStringList & a_addresses);

	QStringList addresses() const;

private slots:

	void slotAddButtonClicked();
	void slotRemoveButtonClicked();

private:

	void checkAndAddAddress(const QString & a_address);

	Ui::TrustedClientsAddressesDialog m_ui;
};

#endif // TRUSTED_CLIENTS_ADDRESSES_DIALOG_H_INCLUDED
