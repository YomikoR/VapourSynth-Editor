#include "job_server_watcher_socket.h"

#include "../../common-src/helpers.h"
#include "../../common-src/ipc_defines.h"
#include "../../common-src/log/vs_editor_log_definitions.h"

#include <QProcess>

//==============================================================================

JobServerWatcherSocket::JobServerWatcherSocket(QObject * a_pParent):
	  QObject(a_pParent)
	, m_pSocket(nullptr)
{
}

// END OF JobServerWatcherSocket::JobServerWatcherSocket(QObject * a_pParent)
//==============================================================================

JobServerWatcherSocket::~JobServerWatcherSocket()
{
}

// END OF JobServerWatcherSocket::~JobServerWatcherSocket()
//==============================================================================

bool JobServerWatcherSocket::sendMessage(const QByteArray & a_data)
{
	bool connected = connectToJobServerWatcher();
	if(!connected)
		return false;

	m_pSocket->write(a_data);
	return true;
}

// END OF bool JobServerWatcherSocket::sendMessage(const QByteArray & a_data)
//==============================================================================

bool JobServerWatcherSocket::connectToJobServerWatcher()
{
	// In Linux QLocalSocket wouldn't reconnect once disconnected.
	// So we recreate it before each connection attempt.
	resetSocket();

	// Must connect in Read/Write mode, or named pipe won't disconnect.
	const QIODevice::OpenMode openMode = QIODevice::ReadWrite;

	m_pSocket->connectToServer(openMode);
	bool connected = m_pSocket->waitForConnected(1000);
	if(connected)
		return true;

	QString watcherPath = vsedit::resolvePathFromApplication(
		"./vsedit-job-server-watcher");
	bool started = QProcess::startDetached(watcherPath);
	if(!started)
	{
		emit signalWriteLogMessage(tr("Could not start "
			"job server watcher."), LOG_STYLE_ERROR);
		return false;
	}

	for(int i = 0; i < 10; ++i)
	{
		resetSocket();
		m_pSocket->connectToServer(openMode);
		connected = m_pSocket->waitForConnected(1000);
		if(connected)
			break;
		vsedit::wait(1000);
	}

	if(!connected)
	{
		emit signalWriteLogMessage(tr("Started job server watcher, "
			"but could not connect."), LOG_STYLE_ERROR);
		return false;
	}

	return true;
}

// END OF bool JobServerWatcherSocket::connectToJobServerWatcher()
//==============================================================================

void JobServerWatcherSocket::resetSocket()
{
	if(m_pSocket)
		delete m_pSocket;

	m_pSocket = new QLocalSocket(this);
	m_pSocket->setServerName(JOB_SERVER_WATCHER_LOCAL_SERVER_NAME);
}

// END OF void JobServerWatcherSocket::resetSocket()
//==============================================================================
