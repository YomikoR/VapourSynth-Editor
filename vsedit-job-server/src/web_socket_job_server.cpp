#include "web_socket_job_server.h"

#include "../../common-src/ipc_defines.h"

//==============================================================================

VSWebSocketJobServer::VSWebSocketJobServer(const QString & a_name,
	SslMode a_secureMode, QObject * a_pParent) :
	QWebSocketServer(a_name, a_secureMode, a_pParent)
{
	connect(this, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));
}

//==============================================================================

VSWebSocketJobServer::~VSWebSocketJobServer()
{
}

//==============================================================================

void VSWebSocketJobServer::slotNewConnection()
{
	QWebSocket * pSocket = nextPendingConnection();

	connect(pSocket, &QWebSocket::binaryMessageReceived,
		this, &VSWebSocketJobServer::slotBinaryMessageReceived);
	connect(pSocket, &QWebSocket::textMessageReceived,
		this, &VSWebSocketJobServer::slotTextMessageReceived);
	connect(pSocket, &QWebSocket::disconnected,
		this, &VSWebSocketJobServer::slotSocketDisconnected);
}

//==============================================================================

void VSWebSocketJobServer::slotBinaryMessageReceived(
	const QByteArray & a_message)
{
	QWebSocket * pClient = qobject_cast<QWebSocket *>(sender());
	if(!pClient)
		return;
	QString messageString = trUtf8(a_message);
	processMessage(pClient, messageString);
}

//==============================================================================

void VSWebSocketJobServer::slotTextMessageReceived(const QString & a_message)
{
	QWebSocket * pClient = qobject_cast<QWebSocket *>(sender());
	if(!pClient)
		return;
	processMessage(pClient, a_message);
}

//==============================================================================

void VSWebSocketJobServer::slotSocketDisconnected()
{
	QWebSocket * pClient = qobject_cast<QWebSocket *>(sender());
	if(!pClient)
		return;
	pClient->deleteLater();
}

//==============================================================================

void VSWebSocketJobServer::processMessage(QWebSocket * a_pClient,
	const QString & a_message)
{
	bool local = a_pClient->peerAddress().isLoopback();

	if(a_message == QString(MSG_GET_JOBS_INFO))
	{
		a_pClient->sendTextMessage("Here is your info!");
		return;
	}

	if(a_message == QString(MSG_CLOSE_SERVER))
	{
		if(local)
		{
			a_pClient->sendTextMessage("Closing server.");
			emit finish();
		}
		else
			a_pClient->sendTextMessage("Can not close server remotely.");
		return;
	}

	a_pClient->sendTextMessage(QString("Received an unknown command: %1")
		.arg(a_message));
}

//==============================================================================
