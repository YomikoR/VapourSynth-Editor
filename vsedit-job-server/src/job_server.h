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
	void slotJobProgressChanged(const QUuid & a_jobID, int a_progress,
		double a_fps);
	void slotJobStartTimeChanged(const QUuid & a_jobID,
		const QDateTime & a_time);
	void slotJobEndTimeChanged(const QUuid & a_jobID,
		const QDateTime & a_time);
	void slotJobDependenciesChanged(const QUuid & a_jobID,
		const std::vector<QUuid> & a_dependencies);
	void slotJobsSwapped(const QUuid & a_jobID1, const QUuid & a_jobID2);
	void slotJobsDeleted(const std::vector<QUuid> & a_ids);

private:

	void processMessage(QWebSocket * a_pClient, const QString & a_message);
	QByteArray jobsInfoMessage() const;
	QByteArray completeLogMessage() const;

	void broadcastMessage(const QString & a_message,
		bool a_includeNonSubscribers = false, bool a_trustedOnly = false);
	void broadcastMessage(const char * a_message,
		bool a_includeNonSubscribers = false, bool a_trustedOnly = false);
	void broadcastMessage(const QByteArray & a_message,
		bool a_includeNonSubscribers = false, bool a_trustedOnly = false);

	SettingsManagerCore * m_pSettingsManager;
	JobsManager * m_pJobsManager;
	QWebSocketServer * m_pWebSocketServer;

	std::vector<LogEntry> m_logEntries;

	std::list<QWebSocket *> m_clients;
	std::list<QWebSocket *> m_subscribers;

	QStringList m_trustedClientsAddresses;
};

#endif // WEB_SOCKET_JOB_SERVER_H_INCLUDED
