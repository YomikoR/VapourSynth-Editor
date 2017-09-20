#include "job_server_watcher_socket.h"

#include "../../common-src/helpers.h"
#include "../../common-src/ipc_defines.h"
#include "../../common-src/log/vs_editor_log_definitions.h"

#include <QProcess>

//==============================================================================

JobServerWatcherSocket::JobServerWatcherSocket(QObject * a_pParent):
	QLocalSocket(a_pParent)
{
	setServerName(JOB_SERVER_WATCHER_LOCAL_SERVER_NAME);
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

	write(a_data);
	return true;
}

// END OF bool JobServerWatcherSocket::sendMessage(const QByteArray & a_data)
//==============================================================================

bool JobServerWatcherSocket::connectToJobServerWatcher()
{
	if(state() == QLocalSocket::ConnectedState)
		return true;

	// Must connect in Read/Write mode, or named pipe won't disconnect.
	const QIODevice::OpenMode openMode = QIODevice::ReadWrite;

	connectToServer(openMode);
	bool connected = waitForConnected(1000);
	if(connected)
		return true;

	QString watcherPath = vsedit::resolvePathFromApplication(
		"./vsedit-job-server-watcher");
	bool started = QProcess::startDetached(watcherPath);
	if(!started)
	{
		emit signalWriteLogMessage(trUtf8("Could not start "
			"job server watcher."), LOG_STYLE_ERROR);
		return false;
	}

	for(int i = 0; i < 10; ++i)
	{
		connectToServer(openMode);
		connected = waitForConnected(1000);
		if(connected)
			break;
		vsedit::wait(1000);
	}

	if(!connected)
	{
		emit signalWriteLogMessage(trUtf8("Started job server watcher, "
			"but could not connect."), LOG_STYLE_ERROR);
		return false;
	}

	return true;
}

// END OF bool JobServerWatcherSocket::connectToJobServerWatcher()
//==============================================================================
