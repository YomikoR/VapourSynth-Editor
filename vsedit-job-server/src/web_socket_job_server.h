#ifndef WEB_SOCKET_JOB_SERVER_H_INCLUDED
#define WEB_SOCKET_JOB_SERVER_H_INCLUDED

#include <QWebSocketServer>

class VSWebSocketJobServer : public QWebSocketServer
{
	Q_OBJECT

public:

	VSWebSocketJobServer(const QString & a_name, SslMode a_secureMode,
		QObject * a_pParent = nullptr);
	virtual ~VSWebSocketJobServer();

signals:

	void finish();

private slots:

	void slotNewConnection();
	void slotBinaryMessageReceived(const QByteArray & a_message);
	void slotTextMessageReceived(const QString & a_message);
	void slotSocketDisconnected();

private:

	void processMessage(QWebSocket * a_pClient, const QString & a_message);
};

#endif // WEB_SOCKET_JOB_SERVER_H_INCLUDED
