#include "job_server.h"

#include "../../common-src/ipc_defines.h"
#include "../../common-src/helpers.h"
#include "jobs/jobs_manager.h"

#include <QWebSocketServer>
#include <QWebSocket>

//==============================================================================

JobServer::JobServer(QObject * a_pParent) : QObject(a_pParent)
	, m_pSettingsManager(nullptr)
	, m_pJobsManager(nullptr)
	, m_pWebSocketServer(nullptr)
{
	m_pSettingsManager = new SettingsManagerCore(this);

	m_trustedClientsAddresses =
		m_pSettingsManager->getTrustedClientsAddresses();

	m_pJobsManager = new JobsManager(m_pSettingsManager, this);
	m_pJobsManager->loadJobs();
	connect(m_pJobsManager, &JobsManager::signalLogMessage,
		this, &JobServer::slotLogMessage);
	connect(m_pJobsManager, &JobsManager::signalJobCreated,
		this, &JobServer::slotJobCreated);
	connect(m_pJobsManager, &JobsManager::signalJobChanged,
		this, &JobServer::slotJobChanged);
	connect(m_pJobsManager, &JobsManager::signalJobStateChanged,
		this, &JobServer::slotJobStateChanged);
	connect(m_pJobsManager, &JobsManager::signalJobProgressChanged,
		this, &JobServer::slotJobProgressChanged);
	connect(m_pJobsManager, &JobsManager::signalJobStartTimeChanged,
		this, &JobServer::slotJobStartTimeChanged);
	connect(m_pJobsManager, &JobsManager::signalJobEndTimeChanged,
		this, &JobServer::slotJobEndTimeChanged);
	connect(m_pJobsManager, &JobsManager::signalJobDependenciesChanged,
		this, &JobServer::slotJobDependenciesChanged);
	connect(m_pJobsManager, &JobsManager::signalJobsSwapped,
		this, &JobServer::slotJobsSwapped);
	connect(m_pJobsManager, &JobsManager::signalJobsDeleted,
		this, &JobServer::slotJobsDeleted);

	m_pWebSocketServer = new QWebSocketServer(JOB_SERVER_NAME,
		QWebSocketServer::NonSecureMode, this);
	connect(m_pWebSocketServer, &QWebSocketServer::newConnection,
		this, &JobServer::slotNewConnection);
}

// END OF JobServer::JobServer(QObject * a_pParent)
//==============================================================================

JobServer::~JobServer()
{
	for(QWebSocket * pClient : m_clients)
	{
		disconnect(pClient, &QWebSocket::disconnected,
			this, &JobServer::slotSocketDisconnected);
		delete pClient;
	}
	m_clients.clear();
	m_subscribers.clear();
	m_pWebSocketServer->close();
	m_pJobsManager->saveJobs();
}

// END OF JobServer::~JobServer()
//==============================================================================

bool JobServer::start()
{
	Q_ASSERT(m_pWebSocketServer);
	return m_pWebSocketServer->listen(QHostAddress::Any, JOB_SERVER_PORT);
}

// END OF bool JobServer::start()
//==============================================================================

void JobServer::slotNewConnection()
{
	QWebSocket * pSocket = m_pWebSocketServer->nextPendingConnection();
	m_clients.push_back(pSocket);

	connect(pSocket, &QWebSocket::binaryMessageReceived,
		this, &JobServer::slotBinaryMessageReceived);
	connect(pSocket, &QWebSocket::textMessageReceived,
		this, &JobServer::slotTextMessageReceived);
	connect(pSocket, &QWebSocket::disconnected,
		this, &JobServer::slotSocketDisconnected);

	if(trustedClientAddress(pSocket->peerAddress()))
	{
		QByteArray message = vsedit::jsonMessage(SMSG_TRUSTED_CLIENTS_INFO,
			QJsonArray::fromStringList(m_trustedClientsAddresses));
		pSocket->sendBinaryMessage(message);
	}
}

// END OF void JobServer::slotNewConnection()
//==============================================================================

void JobServer::slotBinaryMessageReceived(
	const QByteArray & a_message)
{
	QWebSocket * pClient = qobject_cast<QWebSocket *>(sender());
	if(!pClient)
		return;
	QString messageString = tr(a_message);
	processMessage(pClient, messageString);
}

// END OF void JobServer::slotBinaryMessageReceived(
//		const QByteArray & a_message)
//==============================================================================

void JobServer::slotTextMessageReceived(const QString & a_message)
{
	QWebSocket * pClient = qobject_cast<QWebSocket *>(sender());
	if(!pClient)
		return;
	processMessage(pClient, a_message);
}

// END OF void JobServer::slotTextMessageReceived(const QString & a_message)
//==============================================================================

void JobServer::slotSocketDisconnected()
{
	QWebSocket * pClient = qobject_cast<QWebSocket *>(sender());
	if(!pClient)
		return;
	m_clients.remove(pClient);
	m_subscribers.remove(pClient);
	pClient->deleteLater();
}

// END OF void JobServer::slotSocketDisconnected()
//==============================================================================

void JobServer::slotLogMessage(const QString & a_message,
	const QString & a_style)
{
	LogEntry entry(a_message, a_style);
	m_logEntries.push_back(entry);
	broadcastMessage(vsedit::jsonMessage(SMSG_LOG_MESSAGE, entry.toJson()));
}

// END OF void JobServer::slotLogMessage(const QString & a_message,
//		const QString & a_style)
//==============================================================================

void JobServer::slotJobCreated(const JobProperties & a_properties)
{
	broadcastMessage(vsedit::jsonMessage(SMSG_JOB_CREATED,
		a_properties.toJson()));
}

// END OF void JobServer::slotJobCreated(const JobProperties & a_properties)
//==============================================================================

void JobServer::slotJobChanged(const JobProperties & a_properties)
{
	broadcastMessage(vsedit::jsonMessage(SMSG_JOB_UPDATE,
		a_properties.toJson()));
}

// END OF void JobServer::slotJobChanged(const JobProperties & a_properties)
//==============================================================================

void JobServer::slotJobStateChanged(const QUuid & a_jobID, JobState a_state)
{
	QJsonObject jsJob;
	jsJob[JP_ID] = a_jobID.toString();
	jsJob[JP_JOB_STATE] = (int)a_state;
	broadcastMessage(vsedit::jsonMessage(SMSG_JOB_STATE_UPDATE, jsJob));
}

// END OF void JobServer::slotJobStateChanged(const QUuid & a_jobID,
//		JobState a_state)
//==============================================================================

void JobServer::slotJobProgressChanged(const QUuid & a_jobID, int a_progress,
	double a_fps)
{
	QJsonObject jsJob;
	jsJob[JP_ID] = a_jobID.toString();
	jsJob[JP_FRAMES_PROCESSED] = a_progress;
	jsJob[JP_FPS] = a_fps;
	broadcastMessage(vsedit::jsonMessage(SMSG_JOB_PROGRESS_UPDATE, jsJob));
}

// END OF void JobServer::slotJobProgressChanged(const QUuid & a_jobID,
//		int a_progress, double a_fps)
//==============================================================================

void JobServer::slotJobStartTimeChanged(const QUuid & a_jobID,
	const QDateTime & a_time)
{
	QJsonObject jsJob;
	jsJob[JP_ID] = a_jobID.toString();
	jsJob[JP_TIME_STARTED] = a_time.toMSecsSinceEpoch();
	broadcastMessage(vsedit::jsonMessage(SMSG_JOB_START_TIME_UPDATE, jsJob));
}

// END OF void JobServer::slotJobStartTimeChanged(const QUuid & a_jobID,
//		const QDateTime & a_time)
//==============================================================================

void JobServer::slotJobEndTimeChanged(const QUuid & a_jobID,
	const QDateTime & a_time)
{
	QJsonObject jsJob;
	jsJob[JP_ID] = a_jobID.toString();
	jsJob[JP_TIME_ENDED] = a_time.toMSecsSinceEpoch();
	broadcastMessage(vsedit::jsonMessage(SMSG_JOB_END_TIME_UPDATE, jsJob));
}

// END OF void JobServer::slotJobEndTimeChanged(const QUuid & a_jobID,
//		const QDateTime & a_time)
//==============================================================================

void JobServer::slotJobDependenciesChanged(const QUuid & a_jobID,
	const std::vector<QUuid> & a_dependencies)
{
	QJsonObject jsJob;
	jsJob[JP_ID] = a_jobID.toString();
	QJsonArray jsDependencies;
	for(const QUuid & id : a_dependencies)
		jsDependencies << id.toString();
	jsJob[JP_DEPENDS_ON_JOB_IDS] = jsDependencies;
	broadcastMessage(vsedit::jsonMessage(SMSG_JOB_DEPENDENCIES_UPDATE, jsJob));
}

// END OF void JobServer::slotJobDependenciesChanged(const QUuid & a_jobID,
//		const std::vector<QUuid> & a_dependencies)
//==============================================================================

void JobServer::slotJobsSwapped(const QUuid & a_jobID1, const QUuid & a_jobID2)
{
	QJsonArray jsSwap;
	jsSwap << a_jobID1.toString();
	jsSwap << a_jobID2.toString();
	broadcastMessage(vsedit::jsonMessage(SMSG_JOBS_SWAPPED, jsSwap));
}

// END OF void JobServer::slotJobsSwapped(const QUuid & a_jobID1,
//		const QUuid & a_jobID2)
//==============================================================================

void JobServer::slotJobsDeleted(const std::vector<QUuid> & a_ids)
{
	QJsonArray jsIdsArray;
	for(const QUuid & id : a_ids)
		jsIdsArray.push_back(id.toString());
	broadcastMessage(vsedit::jsonMessage(SMSG_JOBS_DELETED, jsIdsArray));
}

// END OF void JobServer::slotJobsDeleted(const std::vector<QUuid> & a_ids)
//==============================================================================

void JobServer::processMessage(QWebSocket * a_pClient,
	const QString & a_message)
{
	bool trustedClient = trustedClientAddress(a_pClient->peerAddress());

	QString command = a_message;
	QString arguments;
	int spaceIndex = a_message.indexOf(' ');
	if(spaceIndex >= 0)
	{
		command = a_message.left(spaceIndex);
		arguments = a_message.mid(spaceIndex + 1);
	}

	QString trustedOnlyCommands[] = {MSG_CLOSE_SERVER, MSG_CREATE_JOB,
		MSG_CHANGE_JOB, MSG_SWAP_JOBS, MSG_RESET_JOBS, MSG_DELETE_JOBS,
		MSG_START_ALL_WAITING_JOBS, MSG_PAUSE_ACTIVE_JOBS,
		MSG_RESUME_PAUSED_JOBS, MSG_ABORT_ACTIVE_JOBS, MSG_GET_TRUSTED_CLIENTS,
		MSG_SET_TRUSTED_CLIENTS};

	if(vsedit::contains(trustedOnlyCommands, command) && (!trustedClient))
	{
		a_pClient->sendBinaryMessage("You're naughty! This command can not "
			"be executed remotely.");
		return;
	}

	QJsonDocument jsArguments = QJsonDocument::fromJson(arguments.toUtf8());

	if(command == QString(MSG_GET_JOBS_INFO))
	{
		a_pClient->sendBinaryMessage(jobsInfoMessage());
		return;
	}

	if(command == QString(MSG_GET_LOG))
	{
		a_pClient->sendBinaryMessage(completeLogMessage());
		return;
	}

	if(command == QString(MSG_SUBSCRIBE))
	{
		m_subscribers.push_back(a_pClient);
		a_pClient->sendBinaryMessage("Subscribed to jobs updates.");
		return;
	}

	if(command == QString(MSG_UNSUBSCRIBE))
	{
		m_subscribers.remove(a_pClient);
		a_pClient->sendBinaryMessage("Unsubscribed from jobs updates.");
		return;
	}

	if(command == QString(MSG_CLOSE_SERVER))
	{
		broadcastMessage(SMSG_CLOSING_SERVER, true);
		emit finish();
		return;
	}

	if(command == QString(MSG_GET_TRUSTED_CLIENTS))
	{
		QByteArray message = vsedit::jsonMessage(SMSG_TRUSTED_CLIENTS_INFO,
			QJsonArray::fromStringList(m_trustedClientsAddresses));
		a_pClient->sendBinaryMessage(message);
		return;
	}

	if(command == QString(MSG_SET_TRUSTED_CLIENTS))
	{
		QStringList trustedClientsAddresses;
		for(QJsonValue value : jsArguments.array())
		{
			QString address = value.toString();
			QHostAddress hostAddress(address);
			if(hostAddress.isLoopback())
				continue;
			trustedClientsAddresses << address;
		}
		trustedClientsAddresses.removeDuplicates();
        m_trustedClientsAddresses = trustedClientsAddresses;
        m_pSettingsManager->setTrustedClientsAddresses(
			m_trustedClientsAddresses);
		QByteArray message = vsedit::jsonMessage(SMSG_TRUSTED_CLIENTS_INFO,
			QJsonArray::fromStringList(m_trustedClientsAddresses));
		broadcastMessage(message, true, true);
		return;
	}

	if(command == QString(MSG_CREATE_JOB))
	{
		JobProperties properties =
			JobProperties::fromJson(jsArguments.object());
		m_pJobsManager->createJob(properties);
		return;
	}

	if(command == QString(MSG_CHANGE_JOB))
	{
		JobProperties properties =
			JobProperties::fromJson(jsArguments.object());
		m_pJobsManager->changeJob(properties);
		return;
	}

	if(command == QString(MSG_SET_JOB_DEPENDENCIES))
	{
		QJsonObject jsJob = jsArguments.object();
		if(!jsJob.contains(JP_ID))
			return;
		QUuid id(jsJob[JP_ID].toString());
		if(!jsJob.contains(JP_DEPENDS_ON_JOB_IDS))
			return;
		QJsonArray jsDependencies = jsJob[JP_DEPENDS_ON_JOB_IDS].toArray();
		std::vector<QUuid> dependencies;
		for(int i = 0; i < jsDependencies.count(); ++i)
			dependencies.push_back(QUuid(jsDependencies[i].toString()));
		m_pJobsManager->setJobDependsOnIds(id, dependencies);
		return;
	}

	if(command == QString(MSG_SWAP_JOBS))
	{
		QJsonArray jsIDs = jsArguments.array();
		if(jsIDs.count() != 2)
			return;
		m_pJobsManager->swapJobs(QUuid(jsIDs[0].toString()),
			QUuid(jsIDs[1].toString()));
		return;
	}

	if(command == QString(MSG_RESET_JOBS))
	{
		QJsonArray jsIDs = jsArguments.array();
		std::vector<QUuid> ids;
		for(int i = 0; i < jsIDs.count(); ++i)
			ids.push_back(QUuid(jsIDs[i].toString()));
		m_pJobsManager->resetJobs(ids);
		return;
	}

	if(command == QString(MSG_DELETE_JOBS))
	{
		QJsonArray jsIDs = jsArguments.array();
		std::vector<QUuid> ids;
		for(int i = 0; i < jsIDs.count(); ++i)
			ids.push_back(QUuid(jsIDs[i].toString()));
		m_pJobsManager->deleteJobs(ids);
		return;
	}

	if(command == QString(MSG_START_ALL_WAITING_JOBS))
	{
		m_pJobsManager->startWaitingJobs();
		return;
	}

	if(command == QString(MSG_PAUSE_ACTIVE_JOBS))
	{
		m_pJobsManager->pauseActiveJobs();
		return;
	}

	if(command == QString(MSG_RESUME_PAUSED_JOBS))
	{
		m_pJobsManager->resumePausedJobs();
		return;
	}

	if(command == QString(MSG_ABORT_ACTIVE_JOBS))
	{
		m_pJobsManager->abortActiveJobs();
		return;
	}

	a_pClient->sendBinaryMessage(QString("Received an unknown command: %1")
		.arg(a_message).toUtf8());
}

// END OF void JobServer::processMessage(QWebSocket * a_pClient,
//		const QString & a_message)
//==============================================================================

QByteArray JobServer::jobsInfoMessage() const
{
	QJsonArray jsJobs;
	for(const JobProperties & properties : m_pJobsManager->jobsProperties())
		jsJobs.push_back(properties.toJson());
	QByteArray message = vsedit::jsonMessage(SMSG_JOBS_INFO, jsJobs);
	return message;
}

// END OF QByteArray JobServer::jobsInfoMessage() const
//==============================================================================

QByteArray JobServer::completeLogMessage() const
{
	QJsonArray jsEntries;
	for(const LogEntry & entry : m_logEntries)
		jsEntries.push_back(entry.toJson());
	QByteArray message = vsedit::jsonMessage(SMSG_COMPLETE_LOG, jsEntries);
	return message;
}

// END OF QByteArray JobServer::completeLogMessage() const
//==============================================================================

void JobServer::broadcastMessage(const QString & a_message,
	bool a_includeNonSubscribers, bool a_trustedOnly)
{
	broadcastMessage(a_message.toUtf8(), a_includeNonSubscribers,
		a_trustedOnly);
}

// END OF void JobServer::broadcastMessage(const QString & a_message,
//		bool a_includeNonSubscribers, bool a_trustedOnly)
//==============================================================================

void JobServer::broadcastMessage(const char * a_message,
	bool a_includeNonSubscribers, bool a_trustedOnly)
{
	broadcastMessage(QByteArray(a_message), a_includeNonSubscribers,
		a_trustedOnly);
}

// END OF void JobServer::broadcastMessage(const char * a_message,
//		bool a_includeNonSubscribers, bool a_trustedOnly)
//==============================================================================

void JobServer::broadcastMessage(const QByteArray & a_message,
	bool a_includeNonSubscribers, bool a_trustedOnly)
{
	std::list<QWebSocket *> & clients =
		a_includeNonSubscribers ? m_clients : m_subscribers;
	for(QWebSocket * pClient : clients)
	{
		if((!a_trustedOnly) || trustedClientAddress(pClient->peerAddress()))
			pClient->sendBinaryMessage(a_message);
	}
}

// END OF void JobServer::broadcastMessage(const QByteArray & a_message,
//		bool a_includeNonSubscribers, bool a_trustedOnly)
//==============================================================================

bool JobServer::trustedClientAddress(const QHostAddress & a_address)
{
	return (a_address.isLoopback() ||
		m_trustedClientsAddresses.contains(a_address.toString()) ||
		m_trustedClientsAddresses.contains(
		a_address.toString().remove("::ffff:")));
}

// END OF bool JobServer::trustedClientAddress(const QHostAddress & a_address)
//==============================================================================
