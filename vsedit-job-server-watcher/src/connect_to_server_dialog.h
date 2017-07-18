#ifndef CONNECT_TO_SERVER_DIALOG_H_INCLUDED
#define CONNECT_TO_SERVER_DIALOG_H_INCLUDED

#include <ui_connect_to_server_dialog.h>

class ConnectToServerDialog : public QDialog
{
	Q_OBJECT

public:

	ConnectToServerDialog(QWidget * a_pParent = nullptr);
	virtual ~ConnectToServerDialog();

private:

	Ui::ConnectToServerDialog m_ui;
};

#endif // CONNECT_TO_SERVER_DIALOG_H_INCLUDED
