#include "main_window.h"

#include "jobs/jobs_model.h"
#include "jobs/job_state_delegate.h"
#include "jobs/job_dependencies_delegate.h"
#include "jobs/job_edit_dialog.h"

#include "../../common-src/helpers.h"
#include "../../common-src/ipc_defines.h"
#include "../../common-src/settings/settings_definitions.h"
#include "../../common-src/settings/settings_manager.h"
#include "../../common-src/vapoursynth/vs_script_library.h"

#include <QCoreApplication>
#include <QMenu>
#include <QAction>
#include <QCloseEvent>
#include <QMoveEvent>
#include <QResizeEvent>
#include <QSocketNotifier>
#include <QToolTip>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QStandardPaths>
#include <QStringList>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QDesktopServices>
#include <QWebSocket>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>
#include <QProcess>

#ifdef Q_OS_WIN
	#include <QWinTaskbarButton>
	#include <QWinTaskbarProgress>
#endif

//==============================================================================

const char MainWindow::WINDOW_TITLE[] = "VapourSynth jobs server watcher";

//==============================================================================

MainWindow::MainWindow() : QMainWindow()
	, m_pSettingsManager(nullptr)
	, m_pJobsModel(nullptr)
	, m_pJobStateDelegate(nullptr)
	, m_pJobDependenciesDelegate(nullptr)
	, m_pVSScriptLibrary(nullptr)
	, m_pJobEditDialog(nullptr)
	, m_pServerSocket(nullptr)
	, m_connectionAttempts(0)
	, m_maxConnectionAttempts(DEFAULT_MAX_WATCHER_CONNECTION_ATTEMPTS)
#ifdef Q_OS_WIN
	, m_pWinTaskbarButton(nullptr)
	, m_pWinTaskbarProgress(nullptr)
#endif
{
	m_ui.setupUi(this);
	setWindowTitle(trUtf8(WINDOW_TITLE));

	setWindowIcon(QIcon(":vsedit.ico"));

	m_pSettingsManager = new SettingsManager(this);
	m_pVSScriptLibrary = new VSScriptLibrary(m_pSettingsManager, this);
	m_pJobEditDialog = new JobEditDialog(m_pSettingsManager,
		m_pVSScriptLibrary, this);

	m_pJobsModel = new JobsModel(m_pSettingsManager, this);
	m_ui.jobsTableView->setModel(m_pJobsModel);
	m_pJobStateDelegate = new JobStateDelegate(this);
	m_ui.jobsTableView->setItemDelegateForColumn(
		JobsModel::STATE_COLUMN, m_pJobStateDelegate);
	m_pJobDependenciesDelegate = new JobDependenciesDelegate(this);
	m_ui.jobsTableView->setItemDelegateForColumn(
		JobsModel::DEPENDS_ON_COLUMN, m_pJobDependenciesDelegate);

	QHeaderView * pHorizontalHeader = m_ui.jobsTableView->horizontalHeader();
	pHorizontalHeader->setSectionsMovable(true);

	QHeaderView * pVerticalHeader = m_ui.jobsTableView->verticalHeader();
	pVerticalHeader->setSectionResizeMode(QHeaderView::ResizeToContents);

	m_ui.logView->setName("job_server_watcher_main_log");
	m_ui.logView->setSettingsManager(m_pSettingsManager);
	m_ui.logView->loadSettings();

	m_pServerSocket = new QWebSocket(QString(),
		QWebSocketProtocol::VersionLatest, this);

	QByteArray newGeometry = m_pSettingsManager->getJobServerWatcherGeometry();
	if(!newGeometry.isEmpty())
		restoreGeometry(newGeometry);

	QByteArray headerState = m_pSettingsManager->getJobsHeaderState();
	if(!headerState.isEmpty())
		pHorizontalHeader->restoreState(headerState);

	pHorizontalHeader->setContextMenuPolicy(Qt::CustomContextMenu);
	m_pJobsHeaderMenu = new QMenu(pHorizontalHeader);
	for(int i = 0; i < m_pJobsModel->columnCount(); ++i)
	{
		QAction * pAction = new QAction(m_pJobsHeaderMenu);
		pAction->setText(
			m_pJobsModel->headerData(i, Qt::Horizontal).toString());
		pAction->setData(i);
		pAction->setCheckable(true);
		pAction->setChecked(!pHorizontalHeader->isSectionHidden(i));
		m_pJobsHeaderMenu->addAction(pAction);
		connect(pAction, SIGNAL(toggled(bool)),
			this, SLOT(slotShowJobsHeaderSection(bool)));
	}

	connect(m_ui.jobNewButton, SIGNAL(clicked()),
		this, SLOT(slotJobNewButtonClicked()));
	connect(m_ui.jobEditButton, SIGNAL(clicked()),
		this, SLOT(slotJobEditButtonClicked()));
	connect(m_ui.jobMoveUpButton, SIGNAL(clicked()),
		this, SLOT(slotJobMoveUpButtonClicked()));
	connect(m_ui.jobMoveDownButton, SIGNAL(clicked()),
		this, SLOT(slotJobMoveDownButtonClicked()));
	connect(m_ui.jobDeleteButton, SIGNAL(clicked()),
		this, SLOT(slotJobDeleteButtonClicked()));
	connect(m_ui.jobResetStateButton, SIGNAL(clicked()),
		this, SLOT(slotJobResetStateButtonClicked()));
	connect(m_ui.startButton, SIGNAL(clicked()),
		this, SLOT(slotStartButtonClicked()));
	connect(m_ui.pauseButton, SIGNAL(clicked()),
		this, SLOT(slotPauseButtonClicked()));
	connect(m_ui.resumeButton, SIGNAL(clicked()),
		this, SLOT(slotResumeButtonClicked()));
	connect(m_ui.abortButton, SIGNAL(clicked()),
		this, SLOT(slotAbortButtonClicked()));
	connect(m_ui.startServerButton, SIGNAL(clicked()),
		this, SLOT(slotStartLocalServer()));
	connect(m_ui.shutdownServerButton, SIGNAL(clicked()),
		this, SLOT(slotShutdownServer()));
	connect(m_ui.jobsTableView, SIGNAL(doubleClicked(const QModelIndex &)),
		this, SLOT(slotJobDoubleClicked(const QModelIndex &)));
	connect(pHorizontalHeader, SIGNAL(sectionResized(int, int, int)),
		this, SLOT(slotSaveHeaderState()));
	connect(pHorizontalHeader, SIGNAL(sectionMoved(int, int, int)),
		this, SLOT(slotSaveHeaderState()));
	connect(pHorizontalHeader, SIGNAL(sectionCountChanged(int, int)),
		this, SLOT(slotSaveHeaderState()));
	connect(pHorizontalHeader, SIGNAL(geometriesChanged()),
		this, SLOT(slotSaveHeaderState()));
	connect(pHorizontalHeader,
		SIGNAL(customContextMenuRequested(const QPoint &)),
		this, SLOT(slotJobsHeaderContextMenu(const QPoint &)));
	connect(m_pServerSocket, &QWebSocket::connected,
		this, &MainWindow::slotServerConnected);
	connect(m_pServerSocket, &QWebSocket::disconnected,
		this, &MainWindow::slotServerDisconnected);
	connect(m_pServerSocket, &QWebSocket::binaryMessageReceived,
		this, &MainWindow::slotBinaryMessageReceived);
	connect(m_pServerSocket, &QWebSocket::textMessageReceived,
		this, &MainWindow::slotTextMessageReceived);
	connect(m_pServerSocket, SIGNAL(error(QAbstractSocket::SocketError)),
		this, SLOT(slotServerError(QAbstractSocket::SocketError)));

	createActionsAndMenus();
}

// END OF MainWindow::MainWindow()
//==============================================================================

MainWindow::~MainWindow()
{
	m_pServerSocket->close(QWebSocketProtocol::CloseCodeNormal,
		trUtf8("Closing watcher."));
	qInstallMessageHandler(0);
}

// END OF MainWindow::~MainWindow()
//==============================================================================

void MainWindow::showAndConnect()
{
	show();
	if(m_pServerSocket->state() == QAbstractSocket::ConnectedState)
		return;

	m_pServerSocket->open(QString("ws://127.0.0.1:%1").arg(JOB_SERVER_PORT));
}

// END OF MainWindow::showAndConnect()
//==============================================================================

void MainWindow::show()
{
	if(m_pSettingsManager->getJobServerWatcherMaximized())
		showMaximized();
	else
		showNormal();
}

// END OF MainWindow::show()
//==============================================================================

void MainWindow::slotWriteLogMessage(int a_messageType,
	const QString & a_message)
{
	QString style = vsMessageTypeToStyleName(a_messageType);
	slotWriteLogMessage(a_message, style);
}

// END OF void MainWindow::slotWriteLogMessage(int a_messageType,
//		const QString & a_message)
//==============================================================================

void MainWindow::slotWriteLogMessage(const QString & a_message,
	const QString & a_style)
{
	m_ui.logView->addEntry(a_message, a_style);

	QString fatalTypes[] = {LOG_STYLE_VS_FATAL, LOG_STYLE_QT_FATAL};
	if(!vsedit::contains(fatalTypes, a_style))
		return;

	QDateTime now = QDateTime::currentDateTime();
	QString timeString = now.toString("hh:mm:ss.zzz");
	QString dateString = now.toString("yyyy-MM-dd");
	QString caption = QObject::trUtf8("VapourSynth fatal error!");
	QString fullMessage = dateString + QString(" ") + timeString +
		QString("\n") + caption + QString("\n") + a_message;

    QString tempPath =
		QStandardPaths::writableLocation(QStandardPaths::TempLocation);
	if(tempPath.isEmpty())
	{
		QMessageBox::critical(nullptr, caption, fullMessage);
		return;
	}

	QString filePath = tempPath + QString("/") +
		QString("VapourSynth-Editor-Job-Server-Watcher-crashlog-") +
		dateString + QString("-") + timeString.replace(':', '-') +
		QString(".html");

	bool saved = m_ui.logView->saveHtml(filePath);
	if(!saved)
	{
		QMessageBox::critical(nullptr, caption, fullMessage);
		return;
	}

	QUrl fileUrl = QUrl::fromLocalFile(filePath);
	QDesktopServices::openUrl(fileUrl);
}

// END OF void MainWindow::slotWriteLogMessage(const QString & a_message,
//		const QString & a_style);
//==============================================================================

void MainWindow::moveEvent(QMoveEvent * a_pEvent)
{
	QMainWindow::moveEvent(a_pEvent);
	saveGeometrySettings();
}

// END OF void MainWindow::moveEvent(QMoveEvent * a_pEvent)
//==============================================================================

void MainWindow::resizeEvent(QResizeEvent * a_pEvent)
{
	QMainWindow::resizeEvent(a_pEvent);
	saveGeometrySettings();
}

// END OF void MainWindow::resizeEvent(QResizeEvent * a_pEvent)
//==============================================================================

void MainWindow::changeEvent(QEvent * a_pEvent)
{
	if(a_pEvent->type() == QEvent::WindowStateChange)
	{
		if(isMaximized())
			m_pSettingsManager->setJobServerWatcherMaximized(true);
		else
			m_pSettingsManager->setJobServerWatcherMaximized(false);
	}
	QMainWindow::changeEvent(a_pEvent);
}

// END OF void MainWindow::changeEvent(QEvent * a_pEvent)
//==============================================================================

void MainWindow::slotJobNewButtonClicked()
{
	int result = m_pJobEditDialog->call(trUtf8("New job"), JobProperties());
	if(result == QDialog::Rejected)
		return;
	JobProperties newJobProperties = m_pJobEditDialog->jobProperties();
    m_pServerSocket->sendTextMessage(
		vsedit::jsonMessage(MSG_CREATE_JOB, newJobProperties.toJson()));
}

// END OF void MainWindow::slotJobNewButtonClicked()
//==============================================================================

void MainWindow::slotJobEditButtonClicked()
{
	QItemSelectionModel * pSelectionModel =
		m_ui.jobsTableView->selectionModel();
	QModelIndexList selection = pSelectionModel->selectedRows();
	if(selection.size() != 1)
		return;
	editJob(selection[0]);
}

// END OF void MainWindow::slotJobEditButtonClicked()
//==============================================================================

void MainWindow::slotJobMoveUpButtonClicked()
{
	std::vector<int> selection = selectedIndexes();
	if(selection.size() != 1)
		return;
	if(selection[0] == 0)
		return;
	QJsonArray jsSwap;
	jsSwap << m_pJobsModel->jobProperties(selection[0] - 1).id.toString();
	jsSwap << m_pJobsModel->jobProperties(selection[0]).id.toString();
	m_pServerSocket->sendTextMessage(
		vsedit::jsonMessage(MSG_SWAP_JOBS, jsSwap));
}

// END OF void MainWindow::slotJobMoveUpButtonClicked()
//==============================================================================

void MainWindow::slotJobMoveDownButtonClicked()
{
	std::vector<int> selection = selectedIndexes();
	if(selection.size() != 1)
		return;
	if(selection[0] >= (int)m_pJobsModel->jobs().size())
		return;
	QJsonArray jsSwap;
	jsSwap << m_pJobsModel->jobProperties(selection[0]).id.toString();
	jsSwap << m_pJobsModel->jobProperties(selection[0] + 1).id.toString();
	m_pServerSocket->sendTextMessage(
		vsedit::jsonMessage(MSG_SWAP_JOBS, jsSwap));
}

// END OF void MainWindow::slotJobMoveDownButtonClicked()
//==============================================================================

void MainWindow::slotJobDeleteButtonClicked()
{
	std::vector<int> selection = selectedIndexes();
	if(selection.empty())
		return;
	QJsonArray jsIds;
	for(int index : selection)
		jsIds << m_pJobsModel->jobProperties(index).id.toString();
	m_pServerSocket->sendTextMessage(
		vsedit::jsonMessage(MSG_DELETE_JOBS, jsIds));
}

// END OF void MainWindow::slotJobDeleteButtonClicked()
//==============================================================================

void MainWindow::slotJobResetStateButtonClicked()
{
	std::vector<int> selection = selectedIndexes();
	if(selection.empty())
		return;
	QJsonArray jsIds;
	for(int index : selection)
		jsIds << m_pJobsModel->jobProperties(index).id.toString();
	m_pServerSocket->sendTextMessage(
		vsedit::jsonMessage(MSG_RESET_JOBS, jsIds));
}

// END OF void MainWindow::slotJobResetStateButtonClicked()
//==============================================================================

void MainWindow::slotStartButtonClicked()
{
	m_pServerSocket->sendTextMessage(MSG_START_ALL_WAITING_JOBS);
}

// END OF void MainWindow::slotStartButtonClicked()
//==============================================================================

void MainWindow::slotPauseButtonClicked()
{
	m_pServerSocket->sendTextMessage(MSG_PAUSE_ACTIVE_JOBS);
}

// END OF void MainWindow::slotPauseButtonClicked()
//==============================================================================

void MainWindow::slotResumeButtonClicked()
{
	m_pServerSocket->sendTextMessage(MSG_RESUME_PAUSED_JOBS);
}

// END OF void MainWindow::slotResumeButtonClicked()
//==============================================================================

void MainWindow::slotAbortButtonClicked()
{
	m_pServerSocket->sendTextMessage(MSG_ABORT_ACTIVE_JOBS);
}

// END OF void MainWindow::slotAbortButtonClicked()
//==============================================================================

void MainWindow::slotJobDoubleClicked(const QModelIndex & a_index)
{
	if(a_index.column() == JobsModel::DEPENDS_ON_COLUMN)
		return;
	editJob(a_index);
}

// END OF void MainWindow::slotJobDoubleClicked(const QModelIndex & a_index)
//==============================================================================

void MainWindow::slotSelectionChanged()
{

}

// END OF void MainWindow::slotSelectionChanged()
//==============================================================================

void MainWindow::slotSaveHeaderState()
{
	QHeaderView * pHeader = m_ui.jobsTableView->horizontalHeader();
	m_pSettingsManager->setJobsHeaderState(pHeader->saveState());
}

// END OF void MainWindow::slotSaveHeaderState()
//==============================================================================

void MainWindow::slotJobsHeaderContextMenu(const QPoint & a_point)
{
	(void)a_point;
	m_pJobsHeaderMenu->exec(QCursor::pos());
}

// END OF void MainWindow::slotJobsHeaderContextMenu(const QPoint & a_point)
//==============================================================================

void MainWindow::slotShowJobsHeaderSection(bool a_show)
{
	QAction * pAction = qobject_cast<QAction *>(sender());
	if(!pAction)
		return;
	int section = pAction->data().toInt();
	QHeaderView * pHeader = m_ui.jobsTableView->horizontalHeader();
	pHeader->setSectionHidden(section, !a_show);
}

// END OF void MainWindow::slotShowJobsHeaderSection(bool a_show)
//==============================================================================

void MainWindow::slotJobsStateChanged(int a_job, int a_jobsTotal,
	JobState a_state, int a_progress, int a_progressMax)
{
	(void)a_job;
	(void)a_jobsTotal;
	(void)a_state;
	(void)a_progress;
	(void)a_progressMax;
}

// END OF void MainWindow::slotJobsStateChanged(int a_job, int a_jobsTotal,
//		JobState a_state, int a_progress, int a_progressMax)
//==============================================================================

void MainWindow::slotServerConnected()
{
	m_connectionAttempts = 0;
	m_pServerSocket->sendTextMessage(MSG_GET_JOBS_INFO);
	m_pServerSocket->sendTextMessage(MSG_GET_LOG);
	m_pServerSocket->sendTextMessage(MSG_SUBSCRIBE);
}

// END OF void MainWindow::slotServerConnected()
//==============================================================================

void MainWindow::slotServerDisconnected()
{
	m_ui.logView->addEntry(trUtf8("Disconnected from server"));
}

// END OF void MainWindow::slotServerDisconnected()
//==============================================================================

void MainWindow::slotBinaryMessageReceived(const QByteArray & a_message)
{
	slotTextMessageReceived(QString::fromUtf8(a_message));
}

// END OF void MainWindow::slotBinaryMessageReceived(
//		const QByteArray & a_message)
//==============================================================================

void MainWindow::slotTextMessageReceived(const QString & a_message)
{
	m_ui.logView->addEntry(a_message, LOG_STYLE_DEFAULT);

	QString command = a_message;
	QString arguments;
	int spaceIndex = a_message.indexOf(' ');
	if(spaceIndex >= 0)
	{
		command = a_message.left(spaceIndex);
		arguments = a_message.mid(spaceIndex + 1);
	}

	QJsonDocument jsArguments = QJsonDocument::fromJson(arguments.toUtf8());

	if(command == QString(SMSG_JOBS_INFO))
	{
		processSMsgJobInfo(arguments);
		return;
	}

	if(command == QString(SMSG_COMPLETE_LOG))
	{
		QJsonArray jsEntries = jsArguments.array();
		for(int i = 0; i < jsEntries.size(); ++i)
		{
			LogEntry entry = LogEntry::fromJson(jsEntries[i].toObject());
			m_ui.logView->addEntry(entry);
		}
		return;
	}

	if(command == QString(SMSG_LOG_MESSAGE))
	{
		LogEntry entry = LogEntry::fromJson(jsArguments.object());
		m_ui.logView->addEntry(entry);
		return;
	}

	if(command == QString(SMSG_JOB_CREATED))
	{
		QJsonObject jsJobProperties = jsArguments.object();
		m_pJobsModel->createJob(JobProperties::fromJson(jsJobProperties));
		return;
	}

	if(command == QString(SMSG_JOB_UPDATE))
	{
		QJsonObject jsJobProperties = jsArguments.object();
		JobProperties properties = JobProperties::fromJson(jsJobProperties);
		m_pJobsModel->updateJobProperties(properties);
		return;
	}

	if(command == QString(SMSG_JOB_STATE_UPDATE))
	{
		QJsonObject jsJob = jsArguments.object();
		if(!jsJob.contains(JP_ID))
			return;
		QUuid id(jsJob[JP_ID].toString());
		if(!jsJob.contains(JP_JOB_STATE))
			return;
		JobState state = (JobState)jsJob[JP_JOB_STATE].toInt();
		m_pJobsModel->setJobState(id, state);
		return;
	}

	if(command == QString(SMSG_JOB_PROGRESS_UPDATE))
	{
		QJsonObject jsJob = jsArguments.object();
		if(!jsJob.contains(JP_ID))
			return;
		QUuid id(jsJob[JP_ID].toString());
		if(!jsJob.contains(JP_FRAMES_PROCESSED))
			return;
		int progress = jsJob[JP_FRAMES_PROCESSED].toInt();
		if(!jsJob.contains(JP_FPS))
			return;
		double fps = jsJob[JP_FPS].toDouble();
		m_pJobsModel->setJobProgress(id, progress, fps);
		return;
	}

	if(command == QString(SMSG_JOBS_SWAPPED))
	{
		QJsonArray jsSwap = jsArguments.array();
		if(jsSwap.size() != 2)
			return;
		QUuid id1(jsSwap[0].toString());
		QUuid id2(jsSwap[1].toString());
		m_pJobsModel->swapJobs(id1, id2);
		return;
	}

	if(command == QString(SMSG_JOBS_DELETED))
	{
		QJsonArray jsIds = jsArguments.array();
		std::vector<QUuid> ids;
		for(int i = 0; i < jsIds.count(); ++i)
			ids.push_back(QUuid(jsIds[i].toString()));
		m_pJobsModel->deleteJobs(ids);
		return;
	}

	if(command == QString(SMSG_REFUSE))
	{
		return;
	}

	if(command == QString(SMSG_CLOSING_SERVER))
	{
		m_ui.logView->addEntry(trUtf8("Server is shutting down."),
			LOG_STYLE_DEFAULT);
		return;
	}

	m_ui.logView->addEntry(a_message, LOG_STYLE_DEFAULT);
}

// END OF void MainWindow::slotTextMessageReceived(const QString & a_message)
//==============================================================================

void MainWindow::slotServerError(QAbstractSocket::SocketError a_error)
{
	(void)a_error;
	m_ui.logView->addEntry(m_pServerSocket->errorString(), LOG_STYLE_ERROR);
}

// END OF void MainWindow::slotServerError(QAbstractSocket::SocketError a_error)
//==============================================================================

void MainWindow::slotStartLocalServer()
{
	bool started = QProcess::startDetached("vsedit-job-server");
	if(!started)
	{
		m_ui.logView->addEntry(trUtf8("Could not start server."),
			LOG_STYLE_ERROR);
		return;
	}
	m_pServerSocket->open(QString("ws://127.0.0.1:%1").arg(JOB_SERVER_PORT));
}

// END OF void MainWindow::slotStartLocalServer()
//==============================================================================

void MainWindow::slotShutdownServer()
{
	if(m_pServerSocket->state() != QAbstractSocket::ConnectedState)
		return;
	m_pServerSocket->sendTextMessage(MSG_CLOSE_SERVER);
}

// END OF void MainWindow::slotShutdownServer()
//==============================================================================

void MainWindow::createActionsAndMenus()
{

}

// END OF void MainWindow::createActionsAndMenus()
//==============================================================================

void MainWindow::saveGeometrySettings()
{
	QApplication::processEvents();
	if(!isMaximized())
		m_pSettingsManager->setJobServerWatcherGeometry(saveGeometry());
}

// END OF void MainWindow::saveGeometrySettings()
//==============================================================================

void MainWindow::editJob(const QModelIndex & a_index)
{
	JobProperties properties = m_pJobsModel->jobProperties(a_index.row());
	if(vsedit::contains(ACTIVE_JOB_STATES, properties.jobState))
	{
		m_ui.logView->addEntry(trUtf8("Can not edit active job."),
			LOG_STYLE_WARNING);
		return;
	}
	int result = m_pJobEditDialog->call(trUtf8("Edit Job %1")
		.arg(a_index.row() + 1), properties);
	if(result == QDialog::Rejected)
		return;
	QUuid id = properties.id;
	properties = m_pJobEditDialog->jobProperties();
	properties.id = id;
    m_pServerSocket->sendTextMessage(vsedit::jsonMessage(MSG_CHANGE_JOB,
		properties.toJson()));
}

// END OF void MainWindow::editJob(const QModelIndex & a_index)
//==============================================================================

void MainWindow::processSMsgJobInfo(const QString & a_message)
{
	if(a_message.isEmpty())
		return;

	QJsonDocument doc = QJsonDocument::fromJson(a_message.toUtf8());
	if(!doc.isArray())
		return;

	std::vector<JobProperties> propertiesVector;

	for(const QJsonValue & value : doc.array())
	{
		if(!value.isObject())
			continue;
		JobProperties properties = JobProperties::fromJson(value.toObject());
		propertiesVector.push_back(properties);
	}

	m_pJobsModel->setJobs(propertiesVector);
}

// END OF void MainWindow::processSMsgJobInfo(const QString & a_message)
//==============================================================================

std::vector<int> MainWindow::selectedIndexes()
{
	std::vector<int> indexes;
	QItemSelectionModel * pSelectionModel =
		m_ui.jobsTableView->selectionModel();
	QModelIndexList selection = pSelectionModel->selectedRows();
	for(const QModelIndex & index : selection)
		indexes.push_back(index.row());
	return indexes;
}

// END OF std::vector<int> MainWindow::selectedIndexes()
//==============================================================================
