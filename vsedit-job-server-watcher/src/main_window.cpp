#include "main_window.h"

#include "../../common-src/helpers.h"
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

//==============================================================================

MainWindow::MainWindow() : QMainWindow()
	, m_pSettingsManager(nullptr)
{
	m_ui.setupUi(this);

	setWindowIcon(QIcon(":vsedit.ico"));

	m_pSettingsManager = new SettingsManager(this);

	m_ui.logView->setName("job_server_watcher_main_log");
	m_ui.logView->setSettingsManager(m_pSettingsManager);
	m_ui.logView->loadSettings();

	createActionsAndMenus();

	QByteArray newGeometry = m_pSettingsManager->getMainWindowGeometry();
	if(!newGeometry.isEmpty())
		restoreGeometry(newGeometry);

	if(m_pSettingsManager->getMainWindowMaximized())
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
		QString("VapourSynth-Editor-crashlog-") + dateString + QString("-") +
		timeString.replace(':', '-') + QString(".html");

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
		m_pSettingsManager->setMainWindowGeometry(saveGeometry());
}

// END OF void MainWindow::moveEvent(QMoveEvent * a_pEvent)
//==============================================================================

void MainWindow::resizeEvent(QResizeEvent * a_pEvent)
{
	QMainWindow::resizeEvent(a_pEvent);
	QApplication::processEvents();
	if(!isMaximized())
		m_pSettingsManager->setMainWindowGeometry(saveGeometry());
}

// END OF void MainWindow::resizeEvent(QResizeEvent * a_pEvent)
//==============================================================================

void MainWindow::changeEvent(QEvent * a_pEvent)
{
	if(a_pEvent->type() == QEvent::WindowStateChange)
	{
		if(isMaximized())
			m_pSettingsManager->setMainWindowMaximized(true);
		else
			m_pSettingsManager->setMainWindowMaximized(false);
	}
	QMainWindow::changeEvent(a_pEvent);
}

// END OF void MainWindow::changeEvent(QEvent * a_pEvent)
//==============================================================================

void MainWindow::createActionsAndMenus()
{

}

// END OF void MainWindow::createActionsAndMenus()
//==============================================================================
