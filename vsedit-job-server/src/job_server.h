#ifndef WEB_SOCKET_JOB_SERVER_H_INCLUDED
#define WEB_SOCKET_JOB_SERVER_H_INCLUDED

#include "../../common-src/settings/settings_manager_core.h"
#include "../../common-src/log/styled_log_view_core.h"

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
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

	void slotLogMessage(const QString & a_message, const QString & a_style);
	void slotJobCreated(const JobProperties & a_properties);
	void slotJobChanged(const JobProperties & a_properties);
	void slotJobStateChanged(const QUuid & a_jobID, JobState a_state);
	void slotJobProgressChanged(const QUuid & a_jobID, int a_progress);
	void slotJobsSwapped(const QUuid & a_jobID1, const QUuid & a_jobID2);
	void slotJobsDeleted(const std::vector<QUuid> a_ids);

private:

	void processMessage(QWebSocket * a_pClient, const QString & a_message);
	QString jobsInfoMessage() const;

	QString jsonMessage(const QString & a_command,
		const QJsonObject & a_jsonObject) const;
	QString jsonMessage(const QString & a_command,
		const QJsonArray & a_jsonArray) const;
	QString jsonMessage(const QString & a_command,
		const QJsonDocument & a_jsonDocument) const;

	SettingsManagerCore * m_pSettingsManager;
	JobsManager * m_pJobsManager;
	QWebSocketServer * m_pWebSocketServer;

	std::vector<LogEntry> m_logEntries;

	std::list<QWebSocket *> m_clients;
	std::list<QWebSocket *> m_subscribers;
};

#endif // WEB_SOCKET_JOB_SERVER_H_INCLUDED
