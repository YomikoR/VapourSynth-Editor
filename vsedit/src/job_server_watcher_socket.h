#ifndef JOB_SERVER_WATCHER_SOCKET_H_INCLUDED
#define JOB_SERVER_WATCHER_SOCKET_H_INCLUDED

#include <QLocalSocket>

class JobServerWatcherSocket : public QLocalSocket
{
	Q_OBJECT

public:

	JobServerWatcherSocket(QObject * a_pParent = nullptr);
	virtual ~JobServerWatcherSocket();

	bool sendMessage(const QByteArray & a_data);

signals:

	void signalWriteLogMessage(const QString & a_message,
		const QString & a_style);

private:

	bool connectToJobServerWatcher();
};

#endif // JOB_SERVER_WATCHER_SOCKET_H_INCLUDED
