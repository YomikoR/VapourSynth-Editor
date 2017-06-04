#ifndef WEB_SOCKET_JOB_SERVER_H_INCLUDED
#define WEB_SOCKET_JOB_SERVER_H_INCLUDED

#include "../../common-src/settings/settings_manager_core.h"
#include "../../common-src/log/styled_log_view_core.h"

#include <QObject>
#include <QJsonObject>
#include <list>
#include <vector>

class SettingsManagerCore;
class JobsManager;
class QWebSocketServer;
class QWebSocket;

class JobServer : public QObject
{
	Q_OBJECT

public:

	JobServer(QObject * a_pParent = nullptr);
	virtual ~JobServer();

	bool start();

signals:

	void finish();

private slots:

	void slotNewConnection();
	void slotBinaryMessageReceived(const QByteArray & a_message);
	void slotTextMessageReceived(const QString & a_message);
	void slotSocketDisconnected();

private:

	void processMessage(QWebSocket * a_pClient, const QString & a_message);

	QJsonObject jobPropertiesToJson(const JobProperties & a_properties) const;
	QString jobsInfoMessage() const;

	SettingsManagerCore * m_pSettingsManager;
	JobsManager * m_pJobsManager;
	QWebSocketServer * m_pWebSocketServer;

	std::vector<LogEntry> m_logEntries;

	std::list<QWebSocket *> m_clients;
	std::list<QWebSocket *> m_subscribers;
};

#endif // WEB_SOCKET_JOB_SERVER_H_INCLUDED
