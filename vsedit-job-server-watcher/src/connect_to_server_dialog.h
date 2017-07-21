#ifndef CONNECT_TO_SERVER_DIALOG_H_INCLUDED
#define CONNECT_TO_SERVER_DIALOG_H_INCLUDED

#include <ui_connect_to_server_dialog.h>

#include <QHostAddress>

class SettingsManager;

class ConnectToServerDialog : public QDialog
{
	Q_OBJECT

public:

	ConnectToServerDialog(SettingsManager * a_pSettingsManager,
		QWidget * a_pParent = nullptr);
	virtual ~ConnectToServerDialog();

	int call(const QHostAddress & a_address);

signals:

	void signalConnectToServer(const QHostAddress & a_address);

private slots:

	void slotConnectButtonClicked();

private:

	void saveServersList();

	Ui::ConnectToServerDialog m_ui;

	SettingsManager * m_pSettingsManager;
};

#endif // CONNECT_TO_SERVER_DIALOG_H_INCLUDED
