#include "main_window.h"

#include "../../common-src/log/vs_editor_log.h"
#include "../../common-src/application_instance_file_guard/application_instance_file_guard.h"
#include "../../common-src/ipc_defines.h"

#include <QApplication>
#include <QLocalSocket>

MainWindow * pMainWindow = nullptr;

void handleQtMessage(QtMsgType a_type, const QMessageLogContext & a_context,
	const QString & a_message)
{
	QString prefix = "Qt debug";
	QString style = LOG_STYLE_DEFAULT;

    switch(a_type)
    {
    case QtDebugMsg:
		prefix = "Qt debug";
		style = LOG_STYLE_QT_DEBUG;
        break;
#if(QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
    case QtInfoMsg:
		prefix = "Qt info";
		style = LOG_STYLE_QT_INFO;
        break;
#endif
    case QtWarningMsg:
		prefix = "Qt warning";
		style = LOG_STYLE_QT_WARNING;
        break;
    case QtCriticalMsg:
		prefix = "Qt critical";
		style = LOG_STYLE_QT_CRITICAL;
        break;
    case QtFatalMsg:
		prefix = "Qt fatal";
		style = LOG_STYLE_QT_FATAL;
		break;
	default:
		Q_ASSERT(false);
    }

    QString fullMessage = QString("%1: %2").arg(prefix).arg(a_message);

	QString fileString(a_context.file);
	QString lineString = QString::number(a_context.line);
	QString functionString(a_context.function);

	QString lineInfo = QString("\n(%1:%2").arg(fileString).arg(lineString);
	if(!functionString.isEmpty())
		lineInfo += QString(", %1").arg(functionString);
	lineInfo += QString(")");
	if(!fileString.isEmpty())
		fullMessage += lineInfo;

    pMainWindow->slotWriteLogMessage(fullMessage, style);

    if(a_type == QtFatalMsg)
		abort();
}

int main(int argc, char *argv[])
{
	QApplication application(argc, argv);
	application.setQuitOnLastWindowClosed(false);

	ApplicationInstanceFileGuard guard("vsedit_job_server_watcher_running");
	if(!guard.isLocked())
	{
		QLocalSocket socket;
		socket.connectToServer(JOB_SERVER_WATCHER_LOCAL_SERVER_NAME,
			QIODevice::WriteOnly);
		bool connected = socket.waitForConnected(5000);
		if(connected)
		{
			socket.write(WMSG_SHOW_WINDOW);
			socket.waitForBytesWritten(5000);
		}
		else
			qCritical("Couldn't start the server watcher.");
		return 1;
	}

	// Make text in message box selectable
	application.setStyleSheet(
		"QMessageBox { messagebox-text-interaction-flags: 5; }");

	pMainWindow = new MainWindow();
	qInstallMessageHandler(handleQtMessage);
	pMainWindow->showAndConnect();
	int exitCode = application.exec();

	delete pMainWindow;
	if(!guard.unlock())
		qCritical("%s", guard.error().toLocal8Bit().data());

	return exitCode;
}
