#include "main_window.h"

#include "../../common-src/helpers.h"
#include "../../common-src/ipc_defines.h"
#include "../../common-src/settings/settings_manager.h"

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

//==============================================================================

MainWindow::MainWindow() : QMainWindow()
	, m_pSettingsManager(nullptr)
	, m_pServerSocket(nullptr)
{
	m_ui.setupUi(this);

	setWindowIcon(QIcon(":vsedit.ico"));

	m_pSettingsManager = new SettingsManager(this);

	m_ui.logView->setName("job_server_watcher_main_log");
	m_ui.logView->setSettingsManager(m_pSettingsManager);
	m_ui.logView->loadSettings();

	m_pServerSocket = new QWebSocket(QString(),
		QWebSocketProtocol::VersionLatest, this);
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

	connect(m_ui.jobNewButton, SIGNAL(clicked()),
		this, SLOT(slotJobNewButtonClicked()));

	createActionsAndMenus();

	QByteArray newGeometry = m_pSettingsManager->getJobServerWatcherGeometry();
	if(!newGeometry.isEmpty())
		restoreGeometry(newGeometry);

	if(m_pSettingsManager->getJobServerWatcherMaximized())
		showMaximized();
}

// END OF MainWindow::MainWindow()
//==============================================================================

MainWindow::~MainWindow()
{
	qInstallMessageHandler(0);
}

// END OF MainWindow::~MainWindow()
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
	QApplication::processEvents();
	if(!isMaximized())
		m_pSettingsManager->setJobServerWatcherGeometry(saveGeometry());
}

// END OF void MainWindow::moveEvent(QMoveEvent * a_pEvent)
//==============================================================================

void MainWindow::resizeEvent(QResizeEvent * a_pEvent)
{
	QMainWindow::resizeEvent(a_pEvent);
	QApplication::processEvents();
	if(!isMaximized())
		m_pSettingsManager->setJobServerWatcherGeometry(saveGeometry());
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
	if(m_pServerSocket->state() == QAbstractSocket::ConnectedState)
		m_pServerSocket->sendTextMessage(MSG_GET_JOBS_INFO);
	else
		m_pServerSocket->open(QString("ws://127.0.0.1:%1")
			.arg(JOB_SERVER_PORT));
}

// END OF void MainWindow::slotJobNewButtonClicked()
//==============================================================================

void MainWindow::slotJobEditButtonClicked()
{

}

// END OF void MainWindow::slotJobEditButtonClicked()
//==============================================================================

void MainWindow::slotJobMoveUpButtonClicked()
{

}

// END OF void MainWindow::slotJobMoveUpButtonClicked()
//==============================================================================

void MainWindow::slotJobMoveDownButtonClicked()
{

}

// END OF void MainWindow::slotJobMoveDownButtonClicked()
//==============================================================================

void MainWindow::slotJobDeleteButtonClicked()
{

}

// END OF void MainWindow::slotJobDeleteButtonClicked()
//==============================================================================

void MainWindow::slotJobResetStateButtonClicked()
{

}

// END OF void MainWindow::slotJobResetStateButtonClicked()
//==============================================================================

void MainWindow::slotStartButtonClicked()
{

}

// END OF void MainWindow::slotStartButtonClicked()
//==============================================================================

void MainWindow::slotPauseButtonClicked()
{

}

// END OF void MainWindow::slotPauseButtonClicked()
//==============================================================================

void MainWindow::slotResumeButtonClicked()
{

}

// END OF void MainWindow::slotResumeButtonClicked()
//==============================================================================

void MainWindow::slotAbortButtonClicked()
{

}

// END OF void MainWindow::slotAbortButtonClicked()
//==============================================================================

void MainWindow::slotJobDoubleClicked(const QModelIndex & a_index)
{

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

}

// END OF void MainWindow::slotSaveHeaderState()
//==============================================================================

void MainWindow::slotJobsHeaderContextMenu(const QPoint & a_point)
{

}

// END OF void MainWindow::slotJobsHeaderContextMenu(const QPoint & a_point)
//==============================================================================

void MainWindow::slotShowJobsHeaderSection(bool a_show)
{

}

// END OF void MainWindow::slotShowJobsHeaderSection(bool a_show)
//==============================================================================

void MainWindow::slotJobsStateChanged(int a_job, int a_jobsTotal,
	JobState a_state, int a_progress, int a_progressMax)
{

}

// END OF void MainWindow::slotJobsStateChanged(int a_job, int a_jobsTotal,
//		JobState a_state, int a_progress, int a_progressMax)
//==============================================================================

void MainWindow::slotServerConnected()
{
	m_pServerSocket->sendTextMessage(MSG_GET_JOBS_INFO);
}

// END OF void MainWindow::slotServerConnected()
//==============================================================================

void MainWindow::slotServerDisconnected()
{

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
}

// END OF void MainWindow::slotTextMessageReceived(const QString & a_message)
//==============================================================================

void MainWindow::slotServerError(QAbstractSocket::SocketError a_error)
{
	m_ui.logView->addEntry(m_pServerSocket->errorString(), LOG_STYLE_ERROR);
}

// END OF void MainWindow::slotServerError(QAbstractSocket::SocketError a_error)
//==============================================================================

void MainWindow::createActionsAndMenus()
{

}

// END OF void MainWindow::createActionsAndMenus()
//==============================================================================

void MainWindow::saveGeometrySettings()
{

}

// END OF void MainWindow::saveGeometrySettings()
//==============================================================================

void MainWindow::editJob(const QModelIndex & a_index)
{

}

// END OF void MainWindow::editJob(const QModelIndex & a_index)
//==============================================================================

bool MainWindow::updateJob(int a_index)
{
	return false;
}

// END OF bool MainWindow::updateJob(int a_index)
//==============================================================================
