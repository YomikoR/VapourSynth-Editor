#include "job_server.h"

#include "../../common-src/ipc_defines.h"
#include "../../common-src/helpers.h"
#include "jobs/jobs_manager.h"

#include <QWebSocketServer>
#include <QWebSocket>
#include <cassert>

//==============================================================================

JobServer::JobServer(QObject * a_pParent) : QObject(a_pParent)
	, m_pSettingsManager(nullptr)
	, m_pJobsManager(nullptr)
	, m_pWebSocketServer(nullptr)
{
	m_pSettingsManager = new SettingsManagerCore(this);

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
	connect(m_pJobsManager, &JobsManager::signalJobsSwapped,
		this, &JobServer::slotJobsSwapped);
	connect(m_pJobsManager, &JobsManager::signalJobsDeleted,
		this, &JobServer::slotJobsDeleted);

	m_pWebSocketServer = new QWebSocketServer(JOB_SERVER_NAME,
		QWebSocketServer::NonSecureMode, this);
	connect(m_pWebSocketServer, &QWebSocketServer::newConnection,
		this, &JobServer::slotNewConnection);
}

//==============================================================================

JobServer::~JobServer()
{
	for(QWebSocket * pClient : m_clients)
		delete pClient;
	m_clients.clear();
	m_subscribers.clear();
	m_pWebSocketServer->close();
	m_pJobsManager->saveJobs();
}

//==============================================================================

bool JobServer::start()
{
	assert(m_pWebSocketServer);
	return m_pWebSocketServer->listen(QHostAddress::Any, JOB_SERVER_PORT);
}

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
}

//==============================================================================

void JobServer::slotBinaryMessageReceived(
	const QByteArray & a_message)
{
	QWebSocket * pClient = qobject_cast<QWebSocket *>(sender());
	if(!pClient)
		return;
	QString messageString = trUtf8(a_message);
	processMessage(pClient, messageString);
}

//==============================================================================

void JobServer::slotTextMessageReceived(const QString & a_message)
{
	QWebSocket * pClient = qobject_cast<QWebSocket *>(sender());
	if(!pClient)
		return;
	processMessage(pClient, a_message);
}

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

//==============================================================================

void JobServer::slotLogMessage(const QString & a_message,
	const QString & a_style)
{
	LogEntry entry(a_message, a_style);
	m_logEntries.push_back(entry);
	broadcastMessage(vsedit::jsonMessage(SMSG_LOG_MESSAGE, entry.toJson()));
}

//==============================================================================

void JobServer::slotJobCreated(const JobProperties & a_properties)
{
	broadcastMessage(vsedit::jsonMessage(SMSG_JOB_CREATED,
		a_properties.toJson()));
}

//==============================================================================

void JobServer::slotJobChanged(const JobProperties & a_properties)
{
	broadcastMessage(vsedit::jsonMessage(SMSG_JOB_UPDATE,
		a_properties.toJson()));
}

//==============================================================================

void JobServer::slotJobStateChanged(const QUuid & a_jobID, JobState a_state)
{
	QJsonObject jsJob;
	jsJob[JP_ID] = a_jobID.toString();
	jsJob[JP_JOB_STATE] = (int)a_state;
	broadcastMessage(vsedit::jsonMessage(SMSG_JOB_STATE_UPDATE, jsJob));
}

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

//==============================================================================

void JobServer::slotJobsSwapped(const QUuid & a_jobID1, const QUuid & a_jobID2)
{
	QJsonArray jsSwap;
	jsSwap << a_jobID1.toString();
	jsSwap << a_jobID2.toString();
	broadcastMessage(vsedit::jsonMessage(SMSG_JOBS_SWAPPED, jsSwap));
}

//==============================================================================

void JobServer::slotJobsDeleted(const std::vector<QUuid> & a_ids)
{
	QJsonArray jsIdsArray;
	for(const QUuid & id : a_ids)
		jsIdsArray.push_back(id.toString());
	broadcastMessage(vsedit::jsonMessage(SMSG_JOBS_DELETED, jsIdsArray));
}

//==============================================================================

void JobServer::processMessage(QWebSocket * a_pClient,
	const QString & a_message)
{
	bool local = a_pClient->peerAddress().isLoopback();

	QString command = a_message;
	QString arguments;
	int spaceIndex = a_message.indexOf(' ');
	if(spaceIndex >= 0)
	{
		command = a_message.left(spaceIndex);
		arguments = a_message.mid(spaceIndex + 1);
	}

	QString localOnlyCommands[] = {MSG_CLOSE_SERVER, MSG_CREATE_JOB,
		MSG_CHANGE_JOB, MSG_SWAP_JOBS, MSG_RESET_JOBS, MSG_DELETE_JOBS,
		MSG_START_ALL_WAITING_JOBS, MSG_PAUSE_ACTIVE_JOBS,
		MSG_RESUME_PAUSED_JOBS, MSG_ABORT_ACTIVE_JOBS};

	if(vsedit::contains(localOnlyCommands, command) && (!local))
	{
		a_pClient->sendTextMessage("You're naughty! This command can not "
			"be executed remotely.");
		return;
	}

	QJsonDocument jsArguments = QJsonDocument::fromJson(arguments.toUtf8());

	if(command == QString(MSG_GET_JOBS_INFO))
	{
		a_pClient->sendTextMessage(jobsInfoMessage());
		return;
	}

	if(command == QString(MSG_GET_LOG))
	{
		a_pClient->sendTextMessage(completeLogMessage());
		return;
	}

	if(command == QString(MSG_SUBSCRIBE))
	{
		m_subscribers.push_back(a_pClient);
		a_pClient->sendTextMessage("Subscribed to jobs updates.");
		return;
	}

	if(command == QString(MSG_UNSUBSCRIBE))
	{
		m_subscribers.remove(a_pClient);
		a_pClient->sendTextMessage("Unsubscribed from jobs updates.");
		return;
	}

	if(command == QString(MSG_CLOSE_SERVER))
	{
		broadcastMessage(SMSG_CLOSING_SERVER, true);
		emit finish();
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

	a_pClient->sendTextMessage(QString("Received an unknown command: %1")
		.arg(a_message));
}

//==============================================================================

QString JobServer::jobsInfoMessage() const
{
	QJsonArray jsJobs;
	for(const JobProperties & properties : m_pJobsManager->jobsProperties())
		jsJobs.push_back(properties.toJson());
	QString message = vsedit::jsonMessage(SMSG_JOBS_INFO, jsJobs);
	return message;
}

//==============================================================================

QString JobServer::completeLogMessage() const
{
	QJsonArray jsEntries;
	for(const LogEntry & entry : m_logEntries)
		jsEntries.push_back(entry.toJson());
	QString message = vsedit::jsonMessage(SMSG_COMPLETE_LOG, jsEntries);
	return message;
}

//==============================================================================

void JobServer::broadcastMessage(const QString & a_message,
	bool a_includeNonSubscribers)
{
	std::list<QWebSocket *> & clients =
		a_includeNonSubscribers ? m_clients : m_subscribers;
	for(QWebSocket * pClient : clients)
		pClient->sendTextMessage(a_message);
}

//==============================================================================
