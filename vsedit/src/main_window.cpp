#include "main_window.h"

#include "../../common-src/settings/settings_manager.h"
#include "../../common-src/vapoursynth/vs_script_library.h"
#include "../../common-src/vapoursynth/vapoursynth_script_processor.h"
#include "../../common-src/helpers.h"
#include "../../common-src/ipc_defines.h"

#include "vapoursynth/vapoursynth_plugins_manager.h"
#include "preview/preview_dialog.h"
#include "settings/settings_dialog.h"
#include "frame_consumers/benchmark_dialog.h"
#include "frame_consumers/encode_dialog.h"
#include "script_templates/templates_dialog.h"
#include "job_server_watcher_socket.h"

#include <QCoreApplication>
#include <QSettings>
#include <QByteArray>
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
#include <QFontDatabase>
#include <QResource>
#include <QDesktopServices>
#include <QUrl>
#include <QDateTime>
#include <QTimer>

//==============================================================================

MainWindow::MainWindow(SettingsManager *settings) : QMainWindow()
	, m_pSettingsManager(settings)
	, m_pVapourSynthPluginsManager(nullptr)
	, m_pVSScriptLibrary(nullptr)
	, m_pActionNewScript(nullptr)
	, m_pActionOpenScript(nullptr)
	, m_pActionSaveScript(nullptr)
	, m_pActionSaveScriptAs(nullptr)
	, m_pActionTemplates(nullptr)
	, m_pActionSettings(nullptr)
	, m_pActionPreview(nullptr)
	, m_pActionCheckScript(nullptr)
	, m_pActionBenchmark(nullptr)
	, m_pActionEncode(nullptr)
	, m_pActionEnqueueEncodeJob(nullptr)
	, m_pActionJobs(nullptr)
	, m_pActionConsole(nullptr)
	, m_pActionExit(nullptr)
	, m_pActionAbout(nullptr)
	, m_settableActionsList()
	, m_pMenuRecentScripts(nullptr)
	, m_pPreviewDialog(nullptr)
	, m_pSettingsDialog(nullptr)
	, m_pBenchmarkDialog(nullptr)
	, m_pEncodeDialog(nullptr)
	, m_pTemplatesDialog(nullptr)
	, m_scriptFilePath()
	, m_lastSavedText()
	, m_pJobServerWatcherSocket(nullptr)
	, m_pGeometrySaveTimer(nullptr)
	, m_pReloadTextTimer(nullptr)
{
	loadFonts();
	vsedit::disableFontKerning(this);

	m_ui.setupUi(this);

	setWindowIcon(QIcon(":vsedit.ico"));

	if(m_pSettingsManager->inDarkMode())
	{
		// Load qDarkStyle colors
		QFile styleSheetDark(":/dark/style.qss");
		if(!styleSheetDark.open(QFile::ReadOnly | QFile::Text))
		{
			QMessageBox::critical(this,
				QString::fromUtf8("File open error"),
				QString::fromUtf8("Failed to open stylesheet file ")
					+ styleSheetDark.errorString());
		}
		qApp->setStyleSheet(styleSheetDark.readAll());
		// With the current impl of the timeline slider
		// we have to relaunch anyway
		QPalette newPal(qApp->palette());
		newPal.setColor(QPalette::Base, QColor(0, 0, 0));
		newPal.setColor(QPalette::Highlight, QColor(128, 128, 128));
		newPal.setColor(QPalette::Dark, QColor(192, 192, 192));
		newPal.setColor(QPalette::Text, QColor(64, 192, 0));
		qApp->setPalette(newPal);
	}
#ifdef Q_OS_WIN
	else
		qApp->setStyle("fusion");
#endif
	m_pSettingsDialog = new SettingsDialog(m_pSettingsManager, nullptr);

	connect(m_pSettingsDialog, SIGNAL(signalSettingsChanged()),
		this, SLOT(slotSettingsChanged()));

	m_pVSScriptLibrary = new VSScriptLibrary(m_pSettingsManager, this);

	connect(m_pVSScriptLibrary,
		SIGNAL(signalWriteLogMessage(int, const QString &)),
		this, SLOT(slotWriteLogMessage(int, const QString &)));

	m_pVapourSynthPluginsManager = new VapourSynthPluginsManager(
		m_pSettingsManager, m_pVSScriptLibrary->getVSAPI(), this);
	VSPluginsList vsPluginsList = m_pVapourSynthPluginsManager->pluginsList();

	m_ui.scriptEdit->setPluginsList(vsPluginsList);
	m_ui.scriptEdit->setSettingsManager(m_pSettingsManager);

	connect(m_ui.scriptEdit, SIGNAL(textChanged()),
		this, SLOT(slotEditorTextChanged()));
	connect(m_ui.scriptEdit, SIGNAL(modificationChanged(bool)),
		this, SLOT(slotChangeWindowTitle()));
	connect(m_ui.scriptEdit,
		SIGNAL(signalScriptFileDropped(const QString &, bool *)),
		this, SLOT(slotScriptFileDropped(const QString &, bool *)));

	m_ui.logView->setName("main_log");
	m_ui.logView->setSettingsManager(m_pSettingsManager);
	m_ui.logView->loadSettings();

	m_pPreviewDialog =
		new PreviewDialog(m_pSettingsManager, m_pVSScriptLibrary);

	connect(m_pPreviewDialog,
		SIGNAL(signalWriteLogMessage(int, const QString &)),
		this, SLOT(slotWriteLogMessage(int, const QString &)));
	connect(m_pPreviewDialog,
		SIGNAL(signalPasteIntoScriptAtNewLine(const QString &)),
		this, SLOT(slotInsertTextIntoScriptAtNewLine(const QString &)));
	connect(m_pPreviewDialog,
		SIGNAL(signalPasteIntoScriptAtCursor(const QString &)),
		this, SLOT(slotInsertTextIntoScriptAtCursor(const QString &)));
	connect(m_pSettingsDialog, SIGNAL(signalSettingsChanged()),
		m_pPreviewDialog, SLOT(slotSettingsChanged()));

	connect(m_ui.scriptEdit, &QPlainTextEdit::textChanged,
		m_pPreviewDialog, &PreviewDialog::slotScriptTextChanged);

	m_pBenchmarkDialog =
		new ScriptBenchmarkDialog(m_pSettingsManager, m_pVSScriptLibrary);
	connect(m_pBenchmarkDialog,
		SIGNAL(signalWriteLogMessage(int, const QString &)),
		this, SLOT(slotWriteLogMessage(int, const QString &)));

	m_pEncodeDialog = new EncodeDialog(m_pSettingsManager, m_pVSScriptLibrary);
	connect(m_pEncodeDialog,
		SIGNAL(signalWriteLogMessage(const QString &, const QString &)),
		this, SLOT(slotWriteLogMessage(const QString &, const QString &)));

	m_pTemplatesDialog = new TemplatesDialog(m_pSettingsManager);
	m_pTemplatesDialog->setPluginsList(vsPluginsList);

	connect(m_pTemplatesDialog, SIGNAL(signalPasteCodeSnippet(const QString &)),
		this, SLOT(slotInsertTextIntoScriptAtNewLine(const QString &)));

	m_orphanQObjects =
	{
		(QObject **)&m_pPreviewDialog,
		(QObject **)&m_pSettingsDialog,
		(QObject **)&m_pBenchmarkDialog,
		(QObject **)&m_pEncodeDialog,
		(QObject **)&m_pTemplatesDialog
	};

	m_pJobServerWatcherSocket = new JobServerWatcherSocket(this);
	connect(m_pJobServerWatcherSocket,
		SIGNAL(signalWriteLogMessage(const QString &, const QString &)),
		this, SLOT(slotWriteLogMessage(const QString &, const QString &)));

	m_pGeometrySaveTimer = new QTimer(this);
	m_pGeometrySaveTimer->setInterval(DEFAULT_WINDOW_GEOMETRY_SAVE_DELAY);
	connect(m_pGeometrySaveTimer, &QTimer::timeout,
		this, &MainWindow::slotSaveGeometry);

	m_pReloadTextTimer = new QTimer(this);
	m_pReloadTextTimer->setInterval(500);
	connect(m_pReloadTextTimer, &QTimer::timeout,
		this, &MainWindow::slotReloadTextFromDisk);

	createActionsAndMenus();

	slotChangeWindowTitle();

	m_windowGeometry = m_pSettingsManager->getMainWindowGeometry();
	if(!m_windowGeometry.isEmpty())
		restoreGeometry(m_windowGeometry);

	if(m_pSettingsManager->getMainWindowMaximized())
		showMaximized();

	loadStartUpScript();

	if(m_pSettingsManager->getReloadScriptFromDisk())
		m_pReloadTextTimer->start();
}

// END OF MainWindow::MainWindow()
//==============================================================================

MainWindow::~MainWindow()
{
	if(m_pGeometrySaveTimer->isActive())
	{
		m_pGeometrySaveTimer->stop();
		slotSaveGeometry();
	}

	if(m_pReloadTextTimer->isActive())
		m_pReloadTextTimer->stop();

	qInstallMessageHandler(0);
	destroyOrphanQObjects();
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
	QString debugTypes[] = {
		LOG_STYLE_DEBUG,
		LOG_STYLE_QT_DEBUG,
		LOG_STYLE_VS_DEBUG,
	};
	if(m_pSettingsManager->getShowDebugMessages() ||
		!vsedit::contains(debugTypes, a_style))
	{
		m_ui.logView->addEntry(a_message, a_style);
	}

	QString fatalTypes[] = {LOG_STYLE_VS_FATAL, LOG_STYLE_QT_FATAL};
	if(!vsedit::contains(fatalTypes, a_style))
		return;

	QDateTime now = QDateTime::currentDateTime();
	QString timeString = now.toString("hh:mm:ss.zzz");
	QString dateString = now.toString("yyyy-MM-dd");
	QString caption = QObject::tr("VapourSynth Editor fatal error!");
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

void MainWindow::slotInsertTextIntoScriptAtNewLine(const QString & a_text)
{
	m_ui.scriptEdit->slotInsertTextAtNewLine(a_text);
}

// END OF void MainWindow::slotInsertTextIntoScriptAtNewLine(
//		const QString & a_text)
//==============================================================================

void MainWindow::slotInsertTextIntoScriptAtCursor(const QString & a_text)
{
	m_ui.scriptEdit->insertPlainText(a_text);
}

// END OF void MainWindow::slotInsertTextIntoScriptAtCursor(
//		const QString & a_text)
//==============================================================================

void MainWindow::closeEvent(QCloseEvent * a_pEvent)
{
	if(!safeToCloseFile())
	{
		a_pEvent->ignore();
		return;
	}

	destroyOrphanQObjects();

	QMainWindow::closeEvent(a_pEvent);
}

// END OF void MainWindow::closeEvent(QCloseEvent * a_pEvent)
//==============================================================================

void MainWindow::moveEvent(QMoveEvent * a_pEvent)
{
	QMainWindow::moveEvent(a_pEvent);
	saveGeometryDelayed();
}

// END OF void MainWindow::moveEvent(QMoveEvent * a_pEvent)
//==============================================================================

void MainWindow::resizeEvent(QResizeEvent * a_pEvent)
{
	QMainWindow::resizeEvent(a_pEvent);
	saveGeometryDelayed();
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

void MainWindow::slotNewScript()
{
	if(!safeToCloseFile())
		return;

	QString newScriptTemplate = m_pSettingsManager->getNewScriptTemplate();

	m_scriptFilePath.clear();
	m_lastSavedText = newScriptTemplate;
	m_ui.scriptEdit->setPlainText(newScriptTemplate);
	m_ui.scriptEdit->moveCursor(QTextCursor::End);
	m_ui.scriptEdit->setModified(true);

	m_pBenchmarkDialog->resetSavedRange();
}

// END OF void MainWindow::slotNewScript()
//==============================================================================

bool MainWindow::slotSaveScript()
{
	if(!m_ui.scriptEdit->isModified())
		return false;

	if(m_scriptFilePath.isEmpty())
	{
		slotSaveScriptAs();
		return false;
	}

	return saveScriptToFile(m_scriptFilePath);
}

// END OF bool MainWindow::slotSaveScript()
//==============================================================================

bool MainWindow::slotSaveScriptAs()
{
	QString offeredFilePath = m_scriptFilePath;
	if(offeredFilePath.isEmpty())
	{
		QFileInfo fileInfo(m_pSettingsManager->getLastUsedPath());
		offeredFilePath = fileInfo.absoluteDir().path() +
			tr("/Untitled.vpy");
	}

	QString filePath = QFileDialog::getSaveFileName(this,
		tr("Save VapourSynth script"), offeredFilePath,
		tr("VapourSynth script (*.vpy);;All files (*)"));

	if(!filePath.isEmpty())
	{
		bool success = saveScriptToFile(filePath);

		// Copy bookmarks
		if((!success) || offeredFilePath.isEmpty())
			return success;
		QFile::copy(offeredFilePath + TIMELINE_BOOKMARKS_FILE_SUFFIX,
			filePath + TIMELINE_BOOKMARKS_FILE_SUFFIX);
	}

	return false;
}

// END OF bool MainWindow::slotSaveScriptAs()
//==============================================================================

bool MainWindow::slotOpenScript()
{
	if(!safeToCloseFile())
		return false;

	QFileInfo fileInfo(m_pSettingsManager->getLastUsedPath());
	QString offeredPath = fileInfo.absoluteDir().path();

	QString filePath = QFileDialog::getOpenFileName(this,
		tr("Open VapourSynth script"), offeredPath,
		tr("VapourSynth script (*.vpy);;All files (*)"));

	return loadScriptFromFile(filePath);
}

// END OF bool MainWindow::slotOpenScript()
//==============================================================================

void MainWindow::slotTemplates()
{
	m_pTemplatesDialog->call();
}

// END OF void MainWindow::slotTemplates()
//==============================================================================

void MainWindow::slotPreview()
{
	if(m_pPreviewDialog->busy())
	{
		QString message = tr("Preview dialog appears busy processing "
			"frames. Please stop any active actions in the dialog and wait "
			"for script processor to finish processing.");
		m_ui.logView->addEntry(message, LOG_STYLE_WARNING);
		return;
	}

	m_pPreviewDialog->previewScript(m_ui.scriptEdit->text(), m_scriptFilePath);
}

// END OF void MainWindow::slotPreview()
//==============================================================================

void MainWindow::slotCheckScript()
{
	VapourSynthScriptProcessor tempProcessor(m_pSettingsManager,
		m_pVSScriptLibrary, this);

	const VSAPI * cpVSAPI = m_pVSScriptLibrary->getVSAPI();

	connect(&tempProcessor, SIGNAL(signalWriteLogMessage(int, const QString &)),
		this, SLOT(slotWriteLogMessage(int, const QString &)));

	bool correct = tempProcessor.initialize(m_ui.scriptEdit->text(),
		m_scriptFilePath, 0, ProcessReason::Check);
	if(correct)
	{
		VSNodeInfo info = tempProcessor.nodeInfo();
		QString message = tr("Script was successfully evaluated. "
			"Output %1 info:\n").arg(info.isAudio() ? "audio" : "video");
		message += vsedit::nodeInfoString(info, cpVSAPI);
		m_ui.logView->addEntry(message, LOG_STYLE_POSITIVE);
	}
}

// END OF void MainWindow::slotCheckScript()
//==============================================================================

void MainWindow::slotBenchmark()
{
	if(m_pBenchmarkDialog->busy())
	{
		QString message = tr("Benchmark dialog appears busy processing "
			"frames. Please stop any active actions in the dialog and wait "
			"for script processor to finish processing.");
		m_ui.logView->addEntry(message, LOG_STYLE_WARNING);
		return;
	}

	m_pBenchmarkDialog->initialize(m_ui.scriptEdit->text(), m_scriptFilePath);
	m_pBenchmarkDialog->call();
}

// END OF void MainWindow::slotBenchmark()
//==============================================================================

void MainWindow::slotEncode()
{
	if(m_pEncodeDialog->busy())
	{
		m_pEncodeDialog->showActive();
		return;
	}

	bool initialized = m_pEncodeDialog->initialize(
		m_ui.scriptEdit->text(), m_scriptFilePath);
	if(initialized)
		m_pEncodeDialog->showActive();
}

// END OF void MainWindow::slotEncode()
//==============================================================================

void MainWindow::slotEnqueueEncodeJob()
{
	if(m_scriptFilePath.isEmpty())
		return;

	JobProperties properties;
	properties.type = JobType::EncodeScriptCLI;
	properties.scriptName = m_scriptFilePath;

	QByteArray message = vsedit::jsonMessage(WMSG_CLI_ENCODE_JOB,
		properties.toJson());
	m_pJobServerWatcherSocket->sendMessage(message);
}

// END OF void MainWindow::slotEnqueueEncodeJob()
//==============================================================================

void MainWindow::slotJobs()
{
	m_pJobServerWatcherSocket->sendMessage(WMSG_SHOW_WINDOW);
}

// END OF void MainWindow::slotJobs()
//==============================================================================

void MainWindow::slotToggleConsole()
{
#if defined(Q_OS_WIN)
	emit signalToggleAttachedConsole();
#endif
}

void MainWindow::slotAbout()
{
	QResource aboutResource(":readme");
	QByteArray aboutData((const char *)aboutResource.data(),
		aboutResource.size());
	QString aboutString = QString::fromUtf8(aboutData);
	aboutString.append(QString("\n\nBuilt with Qt %1").arg(QT_VERSION_STR));
	QString VSAPIInfo = m_pVSScriptLibrary->VSAPIInfo();
	if(VSAPIInfo.isEmpty())
	{
		aboutString.append(QString("\nVapourSynth not well configured."));
	}
	else
	{
		aboutString.append(QString("\nVapourSynth API Version: " + VSAPIInfo));
		QString VSSAPIInfo = m_pVSScriptLibrary->VSSAPIInfo();
		aboutString.append(QString("\nVSScript API Version: " + VSSAPIInfo));
	}
	QMessageBox msgBox(this);
	msgBox.setText(aboutString);
	msgBox.setWindowTitle("About VapourSynth Editor");
	vsedit::disableFontKerning(&msgBox);
	msgBox.exec();
}

// END OF void MainWindow::slotAbout()
//==============================================================================

void MainWindow::slotChangeWindowTitle()
{
	QString windowTitleText = QString::fromUtf8("VapourSynth Editor - ");

	if(m_scriptFilePath.isEmpty())
		windowTitleText += QString::fromUtf8("(Untitled)");
	else
		windowTitleText += m_scriptFilePath;

	if(m_ui.scriptEdit->isModified())
		windowTitleText += "*";

	setWindowTitle(windowTitleText);
}

// END OF void MainWindow::slotChangeWindowTitle()
//==============================================================================

void MainWindow::slotEditorTextChanged()
{
	bool textMatchesSaved = (m_lastSavedText == m_ui.scriptEdit->text());
	m_ui.scriptEdit->setModified(!textMatchesSaved);
	slotChangeWindowTitle();
}

// END OF void MainWindow::slotEditorTextChanged()
//==============================================================================

void MainWindow::slotOpenRecentScriptActionTriggered()
{
	QAction * pAction = qobject_cast<QAction *>(sender());
	if(pAction == nullptr)
		return;
	if(!safeToCloseFile())
		return;
	loadScriptFromFile(pAction->data().toString());
}

// END OF bool MainWindow::safeToCloseFile()
//==============================================================================

void MainWindow::slotSettingsChanged()
{
	QKeySequence hotkey;
	for(QAction * pAction : m_settableActionsList)
	{
		hotkey = m_pSettingsManager->getHotkey(pAction->data().toString());
		pAction->setShortcut(hotkey);
	}

	m_pVapourSynthPluginsManager->slotRefill(m_pVSScriptLibrary->getVSAPI());
	VSPluginsList vsPluginsList = m_pVapourSynthPluginsManager->pluginsList();
	m_ui.scriptEdit->setPluginsList(vsPluginsList);
	m_ui.scriptEdit->slotLoadSettings();
	m_pTemplatesDialog->setPluginsList(vsPluginsList);
	m_pTemplatesDialog->slotLoadSettings();

	if(m_pSettingsManager->getReloadScriptFromDisk())
		m_pReloadTextTimer->start(500);
	else
		m_pReloadTextTimer->stop();
}

// END OF void MainWindow::slotSettingsChanged()
//==============================================================================

void MainWindow::slotScriptFileDropped(const QString & a_filePath,
	bool * a_pHandled)
{
	*a_pHandled = true;

	if(!safeToCloseFile())
		return;

	loadScriptFromFile(a_filePath);
}

// END OF void MainWindow::slotScriptFileDropped(const QString & a_filePath,
//		bool * a_pHandled)
//==============================================================================

void MainWindow::slotReloadTextFromDisk()
{
	if(m_ui.scriptEdit->isModified())
		return;

	if(m_scriptFilePath.isEmpty())
		return;

	QFile scriptFile(m_scriptFilePath);
	bool loadSuccess = scriptFile.open(QIODevice::ReadOnly | QIODevice::Text);
	if(!loadSuccess)
		return;

	QByteArray utf8Script = scriptFile.readAll();
	QString scriptText = QString::fromUtf8(utf8Script);
	if(scriptText.isEmpty()) // To prevent an occasional bug?
		return;
	if(scriptText == m_ui.scriptEdit->text())
		return;

	QPoint pos = m_ui.scriptEdit->cursorPosition();
	m_ui.scriptEdit->setPlainText(scriptText);
	m_ui.scriptEdit->setCursorPosition(pos);
	m_ui.scriptEdit->setModified(false);

	m_pBenchmarkDialog->resetSavedRange();
}

void MainWindow::slotSaveGeometry()
{
	m_pGeometrySaveTimer->stop();
	m_pSettingsManager->setMainWindowGeometry(m_windowGeometry);
}

// END OF void MainWindow::slotSaveGeometry()
//==============================================================================

void MainWindow::createActionsAndMenus()
{
	struct ActionToCreate
	{
		QAction ** ppAction;
		const char * id;
		QObject * pObjectToConnect;
		const char * slotToConnect;
	};

	ActionToCreate actionsToCreate[] =
	{
		{&m_pActionNewScript, ACTION_ID_NEW_SCRIPT,
			this, SLOT(slotNewScript())},
		{&m_pActionOpenScript, ACTION_ID_OPEN_SCRIPT,
			this, SLOT(slotOpenScript())},
		{&m_pActionSaveScript, ACTION_ID_SAVE_SCRIPT,
			this, SLOT(slotSaveScript())},
		{&m_pActionSaveScriptAs, ACTION_ID_SAVE_SCRIPT_AS,
			this, SLOT(slotSaveScriptAs())},
		{&m_pActionExit, ACTION_ID_EXIT,
			this, SLOT(close())},
		{&m_pActionTemplates, ACTION_ID_TEMPLATES,
			this, SLOT(slotTemplates())},
		{&m_pActionSettings, ACTION_ID_SETTINGS,
			m_pSettingsDialog, SLOT(slotCall())},
		{&m_pActionPreview, ACTION_ID_PREVIEW,
			this, SLOT(slotPreview())},
		{&m_pActionCheckScript, ACTION_ID_CHECK_SCRIPT,
			this, SLOT(slotCheckScript())},
		{&m_pActionBenchmark, ACTION_ID_BENCHMARK,
			this, SLOT(slotBenchmark())},
		{&m_pActionEncode, ACTION_ID_CLI_ENCODE,
			this, SLOT(slotEncode())},
		{&m_pActionEnqueueEncodeJob, ACTION_ID_ENQUEUE_ENCODE_JOB,
			this, SLOT(slotEnqueueEncodeJob())},
		{&m_pActionJobs, ACTION_ID_JOBS,
			this, SLOT(slotJobs())},
#if defined(Q_OS_WIN)
		{&m_pActionConsole, ACTION_ID_TOGGLE_CONSOLE,
			this, SLOT(slotToggleConsole())},
#endif
		{&m_pActionAbout, ACTION_ID_ABOUT,
			this, SLOT(slotAbout())},
	};

	for(ActionToCreate & item : actionsToCreate)
	{
		QAction * pAction = m_pSettingsManager->createStandardAction(
			item.id, this);
		*item.ppAction = pAction;
		m_settableActionsList.push_back(pAction);
		connect(pAction, SIGNAL(triggered()),
			item.pObjectToConnect, item.slotToConnect);
	}

//------------------------------------------------------------------------------

	QMenu * pFileMenu = m_ui.menuBar->addMenu(tr("File"));
	vsedit::disableFontKerning(pFileMenu);
	pFileMenu->addAction(m_pActionNewScript);
	pFileMenu->addAction(m_pActionOpenScript);
	pFileMenu->addAction(m_pActionSaveScript);
	pFileMenu->addAction(m_pActionSaveScriptAs);
	pFileMenu->addSeparator();

	m_pMenuRecentScripts = new QMenu(tr("Recent scripts"), this);
	vsedit::disableFontKerning(m_pMenuRecentScripts);
	pFileMenu->addMenu(m_pMenuRecentScripts);
	fillRecentScriptsMenu();

	pFileMenu->addSeparator();
	pFileMenu->addAction(m_pActionExit);

//------------------------------------------------------------------------------

	QMenu * pEditMenu = m_ui.menuBar->addMenu(tr("Edit"));
	vsedit::disableFontKerning(pEditMenu);

	std::vector<QAction *> editorActions = m_ui.scriptEdit->actionsForMenu();
	for(QAction * pAction : editorActions)
		pEditMenu->addAction(pAction);

	pEditMenu->addSeparator();
	pEditMenu->addAction(m_pActionTemplates);
	pEditMenu->addAction(m_pActionSettings);

//------------------------------------------------------------------------------

	QMenu * pScriptMenu = m_ui.menuBar->addMenu(tr("Script"));
	vsedit::disableFontKerning(pScriptMenu);
	pScriptMenu->addAction(m_pActionPreview);
	pScriptMenu->addAction(m_pActionCheckScript);
	pScriptMenu->addAction(m_pActionBenchmark);
	pScriptMenu->addAction(m_pActionEncode);
	pScriptMenu->addAction(m_pActionEnqueueEncodeJob);
	pScriptMenu->addAction(m_pActionJobs);
#if defined(Q_OS_WIN)
	pScriptMenu->addSeparator();
	pScriptMenu->addAction(m_pActionConsole);
#endif

//------------------------------------------------------------------------------

	QMenu * pHelpMenu = m_ui.menuBar->addMenu(tr("Help"));
	vsedit::disableFontKerning(pHelpMenu);
	pHelpMenu->addAction(m_pActionAbout);
}

// END OF void MainWindow::createActionsAndMenus()
//==============================================================================

void MainWindow::fillRecentScriptsMenu()
{
	m_pMenuRecentScripts->clear();
	QStringList recentSciptsList = m_pSettingsManager->getRecentFilesList();
	for(const QString & filePath : recentSciptsList)
	{
		QAction * pAction = new QAction(m_pMenuRecentScripts);
		pAction->setIconText(filePath);
		pAction->setData(filePath);
		m_pMenuRecentScripts->addAction(pAction);

		connect(pAction, SIGNAL(triggered()),
			this, SLOT(slotOpenRecentScriptActionTriggered()));
	}
}

// END OF void MainWindow::fillRecentScriptsMenu()
//==============================================================================

bool MainWindow::saveScriptToFile(const QString& a_filePath)
{
	if(a_filePath.isEmpty())
		return false;

	QFile scriptFile(a_filePath);
	bool openSuccess = scriptFile.open(QIODevice::WriteOnly | QIODevice::Text);
	if(!openSuccess)
	{
		QMessageBox::critical(this,
			QString::fromUtf8("File open error"),
			QString::fromUtf8("Failed to open the file ") + a_filePath +
			QString::fromUtf8("for writing!"));
		return false;
	}

	QByteArray utf8Script = m_ui.scriptEdit->text().toUtf8();
	qint64 writtenBytes = scriptFile.write(utf8Script);

	if(writtenBytes != utf8Script.size())
	{
		QMessageBox::critical(this,
			QString::fromUtf8("File write error"),
			QString::fromUtf8("Error while writing to the file ") + a_filePath);
		return false;
	}

	setCurrentScriptFilePath(a_filePath);
	m_lastSavedText = m_ui.scriptEdit->text();
	m_ui.scriptEdit->setModified(false);

	return true;
}

// END OF bool MainWindow::saveScriptToFile(const QString& a_filePath)
//==============================================================================

bool MainWindow::loadScriptFromFile(const QString& a_filePath)
{
	if(a_filePath.isEmpty())
		return false;

	QFile scriptFile(a_filePath);
	bool loadSuccess = scriptFile.open(QIODevice::ReadOnly | QIODevice::Text);
	if(!loadSuccess)
	{
		QMessageBox::critical(this,
			QString::fromUtf8("File open error"),
			QString::fromUtf8("Failed to open the file %1.").arg(a_filePath));
		return false;
	}

	setCurrentScriptFilePath(a_filePath);
	QByteArray utf8Script = scriptFile.readAll();
	QString scriptText = QString::fromUtf8(utf8Script);
	m_lastSavedText = scriptText;
	m_ui.scriptEdit->setPlainText(scriptText);

	m_pBenchmarkDialog->resetSavedRange();

	return true;
}

// END OF bool MainWindow::loadScriptFromFile(const QString& a_filePath)
//==============================================================================

bool MainWindow::safeToCloseFile()
{
	bool needPrompt = (m_pSettingsManager->getPromptToSaveChanges() &&
		m_ui.scriptEdit->isModified());

	if(!needPrompt)
		return true;

	QMessageBox quesBox(this);
	vsedit::disableFontKerning(&quesBox);
	quesBox.setWindowTitle(tr("Save script?"));
	if(m_scriptFilePath.isEmpty())
	{
		quesBox.setText(
			tr("Would you like to save your script before closing?"));
	}
	else
	{
		quesBox.setText(
			tr("Would you like to save script \"%1\" before closing?")
			.arg(m_scriptFilePath));
	}
	quesBox.setStandardButtons(
		  QMessageBox::Yes
		| QMessageBox::No
		| QMessageBox::Cancel);
	int ret = quesBox.exec();
	switch(ret)
	{
	case QMessageBox::Yes:
		return slotSaveScript();
	case QMessageBox::Cancel:
		return false;	
	default:
		return true;
	}
}

// END OF bool MainWindow::safeToCloseFile()
//==============================================================================

void MainWindow::setCurrentScriptFilePath(const QString & a_filePath)
{
	if(m_scriptFilePath == a_filePath)
		return;

	m_scriptFilePath = a_filePath;
	m_pSettingsManager->setLastUsedPath(a_filePath);
	slotChangeWindowTitle();
	fillRecentScriptsMenu();
}

// END OF void MainWindow::setCurrentScriptFilePath(const QString & a_filePath)
//==============================================================================

void MainWindow::loadStartUpScript()
{
	slotNewScript();

	QStringList argumentsList = QCoreApplication::arguments();
    if(argumentsList.size() > 1)
		loadScriptFromFile(argumentsList.at(1));
	else if(m_pSettingsManager->getAutoLoadLastScript())
	{
		QString lastUsedPath = m_pSettingsManager->getLastUsedPath();
		if(!lastUsedPath.isEmpty())
			loadScriptFromFile(lastUsedPath);
	}
}

// END OF void MainWindow::loadStartUpScript()
//==============================================================================

void MainWindow::loadFonts()
{
	QResource digitalMiniFontResource(":/fonts/DigitalMini.ttf");
	QByteArray digitalMiniFontData((const char *)digitalMiniFontResource.data(),
		digitalMiniFontResource.size());
	QFontDatabase::addApplicationFontFromData(digitalMiniFontData);
}

// END OF void MainWindow::loadFonts()
//==============================================================================

void MainWindow::destroyOrphanQObjects()
{
	for(QObject ** ppObject : m_orphanQObjects)
	{
		if(!ppObject)
			continue;
		if(!*ppObject)
			continue;
		delete *ppObject;
		*ppObject = nullptr;
	}
}

// END OF void MainWindow::destroyOrphanQObjects()
//==============================================================================

void MainWindow::saveGeometryDelayed()
{
	QApplication::processEvents();
	if(!isMaximized())
	{
		m_windowGeometry = saveGeometry();
		m_pGeometrySaveTimer->start();
	}
}

// END OF void MainWindow::saveGeometryDelayed()
//==============================================================================

void MainWindow::reloadTexts()
{
	QPoint pos = m_ui.scriptEdit->cursorPosition();
	if(!loadScriptFromFile(m_scriptFilePath))
		m_ui.scriptEdit->setModified(true);
	m_ui.scriptEdit->setCursorPosition(pos);
}

// END OF void MainWindow::reloadTexts()
//==============================================================================
