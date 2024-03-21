#include "./settings/settings_manager.h"
#include "./vapoursynth/vs_script_library.h"
#include "./vapoursynth/vapoursynth_script_processor.h"
#include "./log/vs_editor_log_definitions.h"
#include "./helpers.h"
#include "./version_info.h"

#include "./preview/preview_dialog.h"

#include <QFileInfo>
#include <QFontDatabase>
#include <QMessageBox>
#include <QResource>

#include <iostream>
#include <map>
#include <string>

Q_DECLARE_OPAQUE_POINTER(const VSFrame *)
Q_DECLARE_OPAQUE_POINTER(VSNode *)

SettingsManager * pSettings = nullptr;
VSScriptLibrary * pVSSLibrary = nullptr;
PreviewDialog * pPreviewDialog = nullptr;

void writeLogMessageByTypename(const QString & a_msg, const QString & a_style)
{
	QString debugTypes[] = {
		LOG_STYLE_DEBUG,
		LOG_STYLE_QT_DEBUG,
		LOG_STYLE_VS_DEBUG,
	};

	if(pSettings->getShowDebugMessages() || !vsedit::contains(debugTypes, a_style))
	{
		std::cerr << "[" << a_style.toUpper().toStdString() << "] "
			<< std::string(a_msg.toLocal8Bit()) << std::endl;
	}

	QString breakingTypes[] = {
		LOG_STYLE_VS_CRITICAL,
		LOG_STYLE_QT_CRITICAL,
		LOG_STYLE_ERROR,
		LOG_STYLE_VS_FATAL,
		LOG_STYLE_QT_FATAL,
	};

	if(vsedit::contains(breakingTypes, a_style))
	{
		QMessageBox * msgBox = new QMessageBox(pPreviewDialog);
		msgBox->setText(a_msg);
		msgBox->setWindowTitle(a_style.toUpper());
		vsedit::disableFontKerning(msgBox);
		msgBox->setTextInteractionFlags(Qt::TextSelectableByMouse);
		msgBox->exec();

		// Handle fatal errors and save to current dir
		QString fatalTypes[] = {LOG_STYLE_VS_FATAL, LOG_STYLE_QT_FATAL};
		if(vsedit::contains(fatalTypes, a_style))
		{
			QDateTime now = QDateTime::currentDateTime();
			QString timeString = now.toString("hh:mm:ss.zzz");
			QString dateString = now.toString("yyyy-MM-dd");
			QString caption = QObject::tr("VSE-Previewer fatal error!");
			QString fullMessage = dateString + QString(" ") + timeString +
				QString("\n") + caption + QString("\n") + a_msg;

			QString applicationDir = QCoreApplication::applicationDirPath();
			QString errorLogFilePath = applicationDir + QString("/") +
				QString("VSE-Previwer-crashlog-") +	dateString + QString("-") +
				timeString.replace(':', '-') + QString(".txt");

			QFile errorLogFile(errorLogFilePath);
			if(errorLogFile.open(QIODevice::WriteOnly))
			{
				errorLogFile.write(fullMessage.toUtf8());
				errorLogFile.close();
			}
		}

		qApp->exit(-1);
	}
}

void writeLogMessage(int a_msgType, const QString & a_msg)
{
	QString style = vsMessageTypeToStyleName(a_msgType);
	writeLogMessageByTypename(a_msg, style);
}

void handleQtMessage(QtMsgType a_type,
	const QMessageLogContext & a_context, const QString & a_message)
{
	QString style = LOG_STYLE_DEFAULT;

	switch(a_type)
	{
	case QtDebugMsg:
		style = LOG_STYLE_QT_DEBUG;
		break;
	case QtInfoMsg:
		style = LOG_STYLE_QT_INFO;
		break;
	case QtWarningMsg:
		style = LOG_STYLE_QT_WARNING;
		break;
	case QtCriticalMsg:
		style = LOG_STYLE_QT_CRITICAL;
		break;
	case QtFatalMsg:
		style = LOG_STYLE_QT_FATAL;
		break;
	default:
		Q_ASSERT(false);
	}

	QString fullMessage = QString("%1: %2").arg(style.toUpper()).arg(a_message);

	QString fileString(a_context.file);
	QString lineString = QString::number(a_context.line);
	QString functionString(a_context.function);

	QString lineInfo = QString("\n(%1:%2").arg(fileString).arg(lineString);
	if(!functionString.isEmpty())
		lineInfo += QString(", %1").arg(functionString);
	lineInfo += QString(")");
	if(!fileString.isEmpty())
		fullMessage += lineInfo;

	writeLogMessageByTypename(fullMessage, style);
}

int main(int argc, char *argv[])
{
	QString scriptFilePath = "";
	std::map<std::string, std::string> scriptArgs = {};
	bool launchInPortableMode = false;

	if (argc <= 1)
	{
		std::cout << "VSE-Previewer " << VSE_PREVIEWER_VERSION << std::endl;
		std::cout << "Usage: vse-previewer [-a key1=value1 -a key2=value2 ...] <script>" << std::endl;
		return 0;
	}

	for(int arg = 1; arg < argc; ++arg)
	{
		QString argString = QString::fromLocal8Bit(argv[arg], -1);
		// Version
		if(argString == "-v" || argString == "--version")
		{
			if (argc > 2)
			{
				std::cerr << "VSE-Previewer: please use `-v` or `--version` without other options for version information." << std::endl;
				return 1;
			}
			else
			{
				std::cout << "VSE-Previewer " << VSE_PREVIEWER_VERSION << std::endl;
				return 0;
			}
		}
		else if(argString == "-a" || argString == "--arg")
		{
			if (argc <= arg + 1)
			{
				std::cerr << "VSE-Previewer: no argument specified." << std::endl;
				return 1;
			}
			QString line = QString::fromLocal8Bit(argv[arg + 1], -1);
			if(line.contains('='))
			{
				auto pos = line.indexOf('=');
				std::string v1 = line.left(pos).toStdString();
				std::string v2 = line.mid(pos + 1).toStdString();
				scriptArgs[v1] = v2;
				++arg;
			}
			else
			{
				std::cerr << "VSE-Previewer: no value specified for argument " << line.toStdString().c_str() << std::endl;
				return 1;
			}
		}
		else if (argString == "-fp" || argString == "--force-portable")
		{
			launchInPortableMode = true;
		}
		else if(scriptFilePath.isEmpty() && !argString.isEmpty())
		{
			scriptFilePath = argString;
		}
		else
		{
			std::cerr << "VSE-Previewer: unknown argument " << argString.toStdString().c_str() << std::endl;
			return 1;
		}
	}

	if(scriptFilePath.isEmpty())
	{
		std::cerr << "VSE-Previewer: no script input found." << std::endl;
		return 1;
	}

	std::cerr << "VSE-Previewer " << VSE_PREVIEWER_VERSION << std::endl;

	QApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);
	QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
		Qt::HighDpiScaleFactorRoundingPolicy::Floor);
	QApplication application(argc, argv);

	qInstallMessageHandler(handleQtMessage);

	pSettings = new SettingsManager(qApp);
	if(launchInPortableMode)
		pSettings->setPortableMode(true);

	vsedit::disableFontKerning(qApp);

	// Make text in message box selectable
	application.setStyleSheet(
		"QToolTip { font-kerning: none; }"
		"QMessageBox { messagebox-text-interaction-flags: 5; }"
		"QLabel { padding: 0px; }");

	qRegisterMetaType<const VSFrame *>("const VSFrame *");
	qRegisterMetaType<VSNode *>("VSNode *");

	QResource digitalMiniFontResource(":/fonts/DigitalMini.ttf");
	QByteArray digitalMiniFontData(
		(const char *)digitalMiniFontResource.data(),
		digitalMiniFontResource.size());
	QFontDatabase::addApplicationFontFromData(digitalMiniFontData);

#ifdef Q_OS_WIN
	qApp->setStyle("fusion");
#endif

	QFileInfo fileInfo(scriptFilePath);
	QString scriptFileFullPath = fileInfo.absoluteFilePath();
	QFile scriptFile(scriptFileFullPath);
	bool loaded = scriptFile.open(QIODevice::ReadOnly | QIODevice::Text);
	if(!loaded)
	{
		QString errorMsg = QString("Failed to open script [%1]!")
			.arg(scriptFilePath);
		writeLogMessageByTypename(errorMsg, LOG_STYLE_ERROR);
		delete pSettings;
		return -1;
	}

	pVSSLibrary = new VSScriptLibrary(pSettings, qApp);
	QObject::connect(pVSSLibrary, &VSScriptLibrary::signalWriteLogMessage,
		writeLogMessage);
	pVSSLibrary->setArguments(scriptArgs);

	pPreviewDialog = new PreviewDialog(pSettings, pVSSLibrary);
	QObject::connect(pPreviewDialog, &PreviewDialog::signalWriteLogMessage,
		writeLogMessage);

	QString scriptText = QString::fromUtf8(scriptFile.readAll());

	pPreviewDialog->previewScript(scriptText, scriptFileFullPath);
	int exitCode = pPreviewDialog->exec();
	return exitCode;
}
