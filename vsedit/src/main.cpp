#include "main_window.h"

#include "../../common-src/log/vs_editor_log.h"
#include "../../common-src/settings/settings_manager.h"

#include <QApplication>

Q_DECLARE_OPAQUE_POINTER(const VSFrame *)
Q_DECLARE_OPAQUE_POINTER(VSNode *)

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
	QApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);
#if (QT_VERSION_MAJOR < 6)
	QApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
#else
	QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
		Qt::HighDpiScaleFactorRoundingPolicy::Floor);
#endif
	QApplication application(argc, argv);

	SettingsManager *settings = new SettingsManager(qApp);

	vsedit::disableFontKerning(qApp);

	// Make text in message box selectable
	application.setStyleSheet(
		"QToolTip { font-kerning: none; }"
		"QMessageBox { messagebox-text-interaction-flags: 5; }"
		"QLabel { padding: 0px; }");

	qRegisterMetaType<const VSFrame *>("const VSFrame *");
	qRegisterMetaType<VSNode *>("VSNode *");

	pMainWindow = new MainWindow(settings);
	qInstallMessageHandler(handleQtMessage);
	pMainWindow->show();
	int exitCode = application.exec();
	delete pMainWindow;
	delete settings;
	return exitCode;
}
