#include <QApplication>
#include <cassert>
#include <vapoursynth/VSScript.h>

#include "mainwindow.h"

Q_DECLARE_OPAQUE_POINTER(const VSFrameRef *)
Q_DECLARE_OPAQUE_POINTER(VSNodeRef *)

MainWindow * pMainWindow = nullptr;

void handleQtMessage(QtMsgType a_type, const QMessageLogContext & a_context,
	const QString & a_message)
{
	int messageType = mtDebug;
	QString prefix = "Qt debug";

    switch(a_type)
    {
    case QtDebugMsg:
		messageType = mtDebug;
		prefix = "Qt debug";
        break;
#if(QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
    case QtInfoMsg:
		messageType = mtDebug;
		prefix = "Qt info";
        break;
#endif
    case QtWarningMsg:
		messageType = mtDebug;
		prefix = "Qt warning";
        break;
    case QtCriticalMsg:
		messageType = mtCritical;
		prefix = "Qt critical";
        break;
    case QtFatalMsg:
    	messageType = mtFatal;
		prefix = "Qt fatal";
		break;
	default:
		assert(false);
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

    pMainWindow->slotWriteLogMessage(messageType, fullMessage);

    if(a_type == QtFatalMsg)
		abort();
}

int main(int argc, char *argv[])
{
	QApplication application(argc, argv);

	qRegisterMetaType<const VSFrameRef *>("const VSFrameRef *");
	qRegisterMetaType<VSNodeRef *>("VSNodeRef *");

	pMainWindow = new MainWindow();
	qInstallMessageHandler(handleQtMessage);
	pMainWindow->show();
	int exitCode = application.exec();
	delete pMainWindow;
	return exitCode;
}
