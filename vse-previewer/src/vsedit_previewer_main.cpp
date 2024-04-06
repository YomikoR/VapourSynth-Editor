#include "./settings/settings_manager.h"
#include "./vapoursynth/vs_script_library.h"
#include "./vapoursynth/vapoursynth_script_processor.h"
#include "./log/vs_editor_log_definitions.h"
#include "./helpers.h"
#include "./version_info.h"

#include "./preview/preview_dialog.h"
#include "./settings/settings_dialog.h"

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
	QString breakingTypes[] = {
		LOG_STYLE_VS_CRITICAL,
		LOG_STYLE_QT_CRITICAL,
		LOG_STYLE_ERROR,
		LOG_STYLE_VS_FATAL,
		LOG_STYLE_QT_FATAL,
	};

	QString debugTypes[] = {
		LOG_STYLE_DEBUG,
		LOG_STYLE_QT_DEBUG,
		LOG_STYLE_VS_DEBUG,
	};

	QString infoTypes[] = {
		LOG_STYLE_QT_INFO,
		LOG_STYLE_VS_INFO,
	};

	if(pSettings->getShowDebugMessages() || !vsedit::contains(debugTypes, a_style))
	{
		if(vsedit::contains(breakingTypes, a_style))
			std::cerr << "\033[1;97;41m";
		else if(vsedit::contains(debugTypes, a_style))
			std::cerr << "\033[1;97;44m";
		else if(vsedit::contains(infoTypes, a_style))
			std::cerr << "\033[1;97m";
		else
			std::cerr << "\033[1;93m";
		std::cerr << "[" << a_style.toUpper().toStdString() << "]\033[0m "
			<< std::string(a_msg.toLocal8Bit()) << std::endl;
	}

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

struct VSEPreviewerArgs
{
	QString scriptFilePath = "";
	std::map<std::string, std::string> scriptArgs = {};
	bool launchInPortableMode = false;
	QString librarySearchPath = "";
	int defaultOutputIndex = 0;
	int defaultFrameNumber = -1;
};

int main(int argc, char *argv[])
{
	VSEPreviewerArgs args;

	bool gotoHelp = false;
	bool gotoSettings = false;

	if(argc <= 1)
		gotoHelp = true;
	else if(argc == 2)
	{
		QString argString = QString::fromLocal8Bit(argv[1], -1);
		if(argString == "-h" || argString == "--help")
			gotoHelp = true;
		else if(argString == "-s" || argString == "--settings")
			gotoSettings = true;
	}

	if(gotoHelp)
	{
		std::cout << "VSE-Previewer " << VSE_PREVIEWER_VERSION << std::endl;
		std::cout << "Usage: vse-previewer [options] <script>" << std::endl;
		std::cout << "Options:" << std::endl;
		std::cout << "  -h, --help                       Show help messages" << std::endl;
		std::cout << "  -s, --settings                   Show (non-preview) settings dialog" << std::endl;
		std::cout << "  -a, --arg key=value              Argument to pass to the script environment, same as vspipe" << std::endl;
		std::cout << "  -o, --outputindex N              Select output index to start with, should be between 0 and 9" << std::endl;
		std::cout << "  -f, --frame N                    Select frame number to start with" << std::endl;
		std::cout << "  -p, --portable                   Force launching in portable mode (to create or move config next to the executable)" << std::endl;
		std::cout << "  -l, --lib directory              Force searching vsscript library from given directory (won't save to settings)" << std::endl;
		return 0;
	}

	if(!gotoSettings)
	{
		for(int arg = 1; arg < argc; ++arg)
		{
			QString argString = QString::fromLocal8Bit(argv[arg], -1);
			if(argString == "-v" || argString == "--version")
			{
				if(argc > 2)
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
			else if(argString == "-s" || argString == "--settings")
			{
				std::cerr << "VSE-Previewer: please use `-s` or `--settings` without other options for settings dialog." << std::endl;
				return 1;
			}
			else if(argString == "-a" || argString == "--arg")
			{
				if(argc <= arg + 1)
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
					args.scriptArgs[v1] = v2;
					++arg;
				}
				else
				{
					std::cerr << "VSE-Previewer: no value specified for argument " << line.toStdString().c_str() << std::endl;
					return 1;
				}
			}
			else if(argString == "-o" || argString == "--outputindex")
			{
				if(argc <= arg + 1)
				{
					std::cerr << "VSE-Previewer: no output index specified." << std::endl;
					return 1;
				}
				QString indexStr = QString::fromLocal8Bit(argv[arg + 1], -1);
				bool isInt = false;
				int index = indexStr.toInt(&isInt, 10);
				if(!isInt || index < 0 || index > 9)
				{
					std::cerr << "VSE-Previewer: please specify a default output index between 0 and 9." << std::endl;
					return 1;
				}
				args.defaultOutputIndex = index;
				++arg;
			}
			else if(argString == "-f" || argString == "--frame")
			{
				if(argc <= arg + 1)
				{
					std::cerr << "VSE-Previewer: no frame number specified." << std::endl;
					return 1;
				}
				QString indexStr = QString::fromLocal8Bit(argv[arg + 1], -1);
				bool isInt = false;
				int number = indexStr.toInt(&isInt, 10);
				if(!isInt || number < 0)
				{
					std::cerr << "VSE-Previewer: please specify a valid frame number." << std::endl;
					return 1;
				}
				args.defaultFrameNumber = number;
				++arg;
			}
			else if(argString == "-p" || argString == "--portable")
			{
				args.launchInPortableMode = true;
			}
			else if(argString == "-l" || argString == "--lib")
			{
				if(argc <= arg + 1)
				{
					std::cerr << "VSE-Previewer: no library search path specified." << std::endl;
					return 1;
				}
				args.librarySearchPath = QString::fromLocal8Bit(argv[arg + 1], -1);
				++arg;
			}
			else if(args.scriptFilePath.isEmpty() && !argString.isEmpty())
			{
				args.scriptFilePath = argString;
			}
			/* Let Qt process arguments as well
			else
			{
				std::cerr << "VSE-Previewer: unknown argument " << argString.toStdString().c_str() << std::endl;
				return 1;
			}
			 */
		}

		if(args.scriptFilePath.isEmpty())
		{
			std::cerr << "VSE-Previewer: no script input found." << std::endl;
			return 1;
		}
	}
	std::cerr << "VSE-Previewer " << VSE_PREVIEWER_VERSION << std::endl;

	QApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);
	QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
		Qt::HighDpiScaleFactorRoundingPolicy::Floor);
	QApplication application(argc, argv);

	qInstallMessageHandler(handleQtMessage);

	pSettings = new SettingsManager(qApp);
	if(args.launchInPortableMode)
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

	if(gotoSettings)
	{
		SettingsDialog * pSettingsDialog = new SettingsDialog(pSettings);
		pSettingsDialog->setAttribute(Qt::WA_DeleteOnClose, true);
		pSettingsDialog->slotCall(false);
		int exitCode = pSettingsDialog->exec();
		return exitCode;
	}

	QFileInfo fileInfo(args.scriptFilePath);
	QString scriptFileFullPath = fileInfo.absoluteFilePath();
	QFile scriptFile(scriptFileFullPath);
	bool loaded = scriptFile.open(QIODevice::ReadOnly | QIODevice::Text);
	if(!loaded)
	{
		QString errorMsg = QString("Failed to open script [%1]!")
			.arg(args.scriptFilePath);
		writeLogMessageByTypename(errorMsg, LOG_STYLE_ERROR);
		delete pSettings;
		return -1;
	}

	pVSSLibrary = new VSScriptLibrary(pSettings, qApp, args.librarySearchPath);
	QObject::connect(pVSSLibrary, &VSScriptLibrary::signalWriteLogMessage,
		writeLogMessage);
	pVSSLibrary->setDefaultOutputIndex(args.defaultOutputIndex);
	pVSSLibrary->setArguments(args.scriptArgs);

	pPreviewDialog = new PreviewDialog(pSettings, pVSSLibrary);
	QObject::connect(pPreviewDialog, &PreviewDialog::signalWriteLogMessage,
		writeLogMessage);

	QString scriptText = QString::fromUtf8(scriptFile.readAll());

	pPreviewDialog->previewScript(scriptText, scriptFileFullPath,
		args.defaultFrameNumber);

	int exitCode = -1;
	if(pVSSLibrary->isInitialized())
		exitCode = pPreviewDialog->exec();

	return exitCode;
}
