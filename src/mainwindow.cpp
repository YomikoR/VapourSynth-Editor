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

#include "settings/settingsmanager.h"
#include "vapoursynth/vapoursynthscriptprocessor.h"
#include "vapoursynth/vapoursynthpluginsmanager.h"
#include "preview/previewdialog.h"
#include "settings/settingsdialog.h"
#include "common/helpers.h"

#include "mainwindow.h"

//==============================================================================

MainWindow::MainWindow() : QMainWindow()
	, m_pSettingsManager(nullptr)
	, m_pVapourSynthScriptProcessor(nullptr)
	, m_pVapourSynthPluginsManager(nullptr)
	, m_pActionNewScript(nullptr)
	, m_pActionOpenScript(nullptr)
	, m_pActionSaveScript(nullptr)
	, m_pActionSaveScriptAs(nullptr)
	, m_pActionSettings(nullptr)
	, m_pActionPreview(nullptr)
	, m_pActionCheckScript(nullptr)
	, m_pActionExit(nullptr)
	, m_pActionAbout(nullptr)
	, m_pActionAutocomplete(nullptr)
	, m_settableActionsList()
	, m_pMenuRecentScripts(nullptr)
	, m_pPreviewDialog(nullptr)
	, m_pSettingsDialog(nullptr)
	, m_scriptFilePath()
	, m_lastSavedText()
{
	loadFonts();

	m_ui.setupUi(this);

	setWindowIcon(QIcon(":vsedit.ico"));

	m_pSettingsManager = new SettingsManager(this);
	m_pSettingsDialog = new SettingsDialog(m_pSettingsManager, nullptr);

	m_pVapourSynthScriptProcessor =
		new VapourSynthScriptProcessor(m_pSettingsManager, this);

	connect(m_pVapourSynthScriptProcessor,
		SIGNAL(signalWriteLogMessage(int, const QString &)),
		this, SLOT(slotWriteLogMessage(int, const QString &)));

	m_pVapourSynthPluginsManager =
		new VapourSynthPluginsManager(m_pSettingsManager, this);

	// Just a debug.
//	for(const QString & function : m_pVapourSynthPluginsManager->functions())
//		slotWriteLogMessage(mtDebug, QString("core.") + function);

	m_ui.scriptEdit->setPluginsList(
		m_pVapourSynthPluginsManager->pluginsList());
	m_ui.scriptEdit->setCharactersTypedToStartCompletion(
		m_pSettingsManager->getCharactersTypedToStartCompletion());
	m_ui.scriptEdit->setSettingsManager(m_pSettingsManager);
	m_ui.scriptEdit->setSettingsDialog(m_pSettingsDialog);
	m_ui.scriptEdit->slotLoadSettings();

	connect(m_ui.scriptEdit, SIGNAL(textChanged()),
		this, SLOT(slotEditorTextChanged()));
	connect(m_ui.scriptEdit, SIGNAL(modificationChanged(bool)),
		this, SLOT(slotChangeWindowTitle()));

	connect(m_pSettingsDialog, SIGNAL(signalSettingsChanged()),
		this, SLOT(slotSettingsChanged()));

	m_pPreviewDialog = new PreviewDialog(m_pVapourSynthScriptProcessor,
		m_pSettingsManager, m_pSettingsDialog);

	connect(m_pPreviewDialog,
		SIGNAL(signalWriteLogMessage(int, const QString &)),
		this, SLOT(slotWriteLogMessage(int, const QString &)));
	connect(m_pPreviewDialog,
		SIGNAL(signalInsertLineIntoScript(const QString &)),
		this, SLOT(slotInsertLineIntoScript(const QString &)));

	createActionsAndMenus();
	slotChangeWindowTitle();

	QByteArray newGeometry = m_pSettingsManager->getMainWindowGeometry();
	if(!newGeometry.isEmpty())
		restoreGeometry(newGeometry);

	if(m_pSettingsManager->getMainWindowMaximized())
		showMaximized();

	loadStartUpScript();
}

// END OF MainWindow::MainWindow()
//==============================================================================

MainWindow::~MainWindow()
{
	if(m_pPreviewDialog)
		delete m_pPreviewDialog;
	if(m_pSettingsDialog)
		delete m_pSettingsDialog;
}

// END OF MainWindow::~MainWindow()
//==============================================================================

void MainWindow::slotWriteLogMessage(int a_messageType,
	const QString & a_message)
{
	if((a_messageType == mtFatal) || (a_messageType == mtCritical))
		m_ui.logEdit->setTextColor(QColor(255, 0, 0));
	else if(a_messageType == mtWarning)
		m_ui.logEdit->setTextColor(QColor(0, 0, 255));
	else
		m_ui.logEdit->setTextColor(QColor(0, 0, 0));
	m_ui.logEdit->append(a_message);
}

// END OF void MainWindow::slotWriteLogMessage(int a_messageType,
//		const QString & a_message)
//==============================================================================

void MainWindow::slotInsertLineIntoScript(const QString & a_line)
{
	QPoint cursorPosition = m_ui.scriptEdit->cursorPosition();
	m_ui.scriptEdit->setCursorPosition(cursorPosition.x() + 1, 0);
	m_ui.scriptEdit->insertPlainText(a_line + "\n");
}

// END OF void MainWindow::slotInsertLineIntoScript(const QString & a_line)
//==============================================================================

void MainWindow::closeEvent(QCloseEvent * a_pEvent)
{
	if(!safeToCloseFile())
	{
		a_pEvent->ignore();
		return;
	}

	delete m_pPreviewDialog;
	m_pPreviewDialog = nullptr;
	delete m_pSettingsDialog;
	m_pSettingsDialog = nullptr;

	QMainWindow::closeEvent(a_pEvent);
}

// END OF void MainWindow::closeEvent(QCloseEvent * a_pEvent)
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

void MainWindow::slotNewScript()
{
	if(!safeToCloseFile())
		return;

	QString newScriptTemplate(
		"import vapoursynth as vs\n"
		"core = vs.get_core()\n");

	m_scriptFilePath.clear();
	m_lastSavedText = newScriptTemplate;
	m_ui.scriptEdit->setPlainText(newScriptTemplate);
	m_ui.scriptEdit->setCursorPosition(2, 0);
	slotChangeWindowTitle();
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
			trUtf8("/Untitled.vpy");
	}

	QString filePath = QFileDialog::getSaveFileName(this,
		trUtf8("Save VapourSynth script"), offeredFilePath,
		trUtf8("VapourSynth script (*.vpy);;All files (*)"));

	if(!filePath.isEmpty())
		return saveScriptToFile(filePath);

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
		trUtf8("Open VapourSynth script"), offeredPath,
		trUtf8("VapourSynth script (*.vpy);;All files (*)"));

	return loadScriptFromFile(filePath);
}

// END OF bool MainWindow::slotOpenScript()
//==============================================================================

void MainWindow::slotPreview()
{
	m_ui.logEdit->clear();
	m_pVapourSynthScriptProcessor->finalize();
	m_pPreviewDialog->previewScript(m_ui.scriptEdit->text(), m_scriptFilePath);
}

// END OF void MainWindow::slotPreview()
//==============================================================================

void MainWindow::slotCheckScript()
{
	m_ui.logEdit->clear();

	m_pPreviewDialog->close();

	bool correct = m_pVapourSynthScriptProcessor->initialize(
		m_ui.scriptEdit->text(), m_scriptFilePath);
	if(correct)
	{
		QString message = trUtf8("Script was successfully evaluated. "
			"Output video info:\n");
		message += vsedit::videoInfoString(
			m_pVapourSynthScriptProcessor->videoInfo());
		slotWriteLogMessage(mtDebug, message);
	}
	m_pVapourSynthScriptProcessor->finalize();
}

// END OF void MainWindow::slotCheckScript()
//==============================================================================

void MainWindow::slotAbout()
{
	QResource aboutResource(":readme");
	QByteArray aboutData((const char *)aboutResource.data(),
		aboutResource.size());
	QString aboutString = QString::fromUtf8(aboutData);
	QMessageBox::about(this, "VapourSynth Editor", aboutString);
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

	m_pVapourSynthPluginsManager->slotRefill();
	m_ui.scriptEdit->setPluginsList(
		m_pVapourSynthPluginsManager->pluginsList());
	m_ui.scriptEdit->setCharactersTypedToStartCompletion(
		m_pSettingsManager->getCharactersTypedToStartCompletion());
}

// END OF void MainWindow::slotSettingsChanged()
//==============================================================================

void MainWindow::createActionsAndMenus()
{
	QKeySequence hotkey;

//------------------------------------------------------------------------------

	QMenu * pFileMenu = m_ui.menuBar->addMenu(trUtf8("File"));

//------------------------------------------------------------------------------

	m_pActionNewScript = new QAction(this);
	m_pActionNewScript->setIconText(trUtf8("New script"));
	m_pActionNewScript->setIcon(QIcon(QString(":new.png")));
	hotkey = m_pSettingsManager->getHotkey(ACTION_ID_NEW_SCRIPT);
	m_pActionNewScript->setShortcut(hotkey);
	pFileMenu->addAction(m_pActionNewScript);
	m_pActionNewScript->setData(ACTION_ID_NEW_SCRIPT);
	m_settableActionsList.push_back(m_pActionNewScript);

//------------------------------------------------------------------------------

	m_pActionOpenScript = new QAction(this);
	m_pActionOpenScript->setIconText(trUtf8("Open script"));
	m_pActionOpenScript->setIcon(QIcon(QString(":load.png")));
	hotkey = m_pSettingsManager->getHotkey(ACTION_ID_OPEN_SCRIPT);
	m_pActionOpenScript->setShortcut(hotkey);
	pFileMenu->addAction(m_pActionOpenScript);
	m_pActionOpenScript->setData(ACTION_ID_OPEN_SCRIPT);
	m_settableActionsList.push_back(m_pActionOpenScript);

//------------------------------------------------------------------------------

	m_pActionSaveScript = new QAction(this);
	m_pActionSaveScript->setIconText(trUtf8("Save script"));
	m_pActionSaveScript->setIcon(QIcon(QString(":save.png")));
	hotkey = m_pSettingsManager->getHotkey(ACTION_ID_SAVE_SCRIPT);
	m_pActionSaveScript->setShortcut(hotkey);
	pFileMenu->addAction(m_pActionSaveScript);
	m_pActionSaveScript->setData(ACTION_ID_SAVE_SCRIPT);
	m_settableActionsList.push_back(m_pActionSaveScript);

//------------------------------------------------------------------------------

	m_pActionSaveScriptAs = new QAction(this);
	m_pActionSaveScriptAs->setIconText(trUtf8("Save script as..."));
	m_pActionSaveScriptAs->setIcon(QIcon(QString(":saveas.png")));
	hotkey = m_pSettingsManager->getHotkey(ACTION_ID_SAVE_SCRIPT_AS);
	m_pActionSaveScriptAs->setShortcut(hotkey);
	pFileMenu->addAction(m_pActionSaveScriptAs);
	m_pActionSaveScriptAs->setData(ACTION_ID_SAVE_SCRIPT_AS);
	m_settableActionsList.push_back(m_pActionSaveScriptAs);

//------------------------------------------------------------------------------

	pFileMenu->addSeparator();

	m_pMenuRecentScripts = new QMenu(trUtf8("Recent scripts"), this);
	pFileMenu->addMenu(m_pMenuRecentScripts);
	fillRecentScriptsMenu();

	pFileMenu->addSeparator();

//------------------------------------------------------------------------------

	m_pActionExit = new QAction(this);
	m_pActionExit->setIconText(trUtf8("Exit"));
	m_pActionExit->setIcon(QIcon(QString(":exit.png")));
	hotkey = m_pSettingsManager->getHotkey(ACTION_ID_EXIT);
	m_pActionExit->setShortcut(hotkey);
	pFileMenu->addAction(m_pActionExit);
	m_pActionExit->setData(ACTION_ID_EXIT);
	m_settableActionsList.push_back(m_pActionExit);

//------------------------------------------------------------------------------

	QMenu * pEditMenu = m_ui.menuBar->addMenu(trUtf8("Edit"));

//------------------------------------------------------------------------------

	m_pActionSettings = new QAction(this);
	m_pActionSettings->setIconText(trUtf8("Settings"));
	m_pActionSettings->setIcon(QIcon(QString(":settings.png")));
	hotkey = m_pSettingsManager->getHotkey(ACTION_ID_SETTINGS);
	m_pActionSettings->setShortcut(hotkey);
	pEditMenu->addAction(m_pActionSettings);
	m_pActionSettings->setData(ACTION_ID_SETTINGS);
	m_settableActionsList.push_back(m_pActionSettings);

//------------------------------------------------------------------------------

	QMenu * pScriptMenu = m_ui.menuBar->addMenu(trUtf8("Script"));

//------------------------------------------------------------------------------

	m_pActionPreview = new QAction(this);
	m_pActionPreview->setIconText(trUtf8("Preview"));
	m_pActionPreview->setIcon(QIcon(QString(":preview.png")));
	hotkey = m_pSettingsManager->getHotkey(ACTION_ID_PREVIEW);
	m_pActionPreview->setShortcut(hotkey);
	pScriptMenu->addAction(m_pActionPreview);
	m_pActionPreview->setData(ACTION_ID_PREVIEW);
	m_settableActionsList.push_back(m_pActionPreview);

//------------------------------------------------------------------------------

	m_pActionCheckScript = new QAction(this);
	m_pActionCheckScript->setIconText(trUtf8("Check script"));
	m_pActionCheckScript->setIcon(QIcon(QString(":check.png")));
	hotkey = m_pSettingsManager->getHotkey(ACTION_ID_CHECK_SCRIPT);
	m_pActionCheckScript->setShortcut(hotkey);
	pScriptMenu->addAction(m_pActionCheckScript);
	m_pActionCheckScript->setData(ACTION_ID_CHECK_SCRIPT);
	m_settableActionsList.push_back(m_pActionCheckScript);

//------------------------------------------------------------------------------

	QMenu * pHelpMenu = m_ui.menuBar->addMenu(trUtf8("Help"));

//------------------------------------------------------------------------------

	m_pActionAbout = new QAction(this);
	m_pActionAbout->setIconText(trUtf8("About..."));
	hotkey = m_pSettingsManager->getHotkey(ACTION_ID_ABOUT);
	m_pActionAbout->setShortcut(hotkey);
	pHelpMenu->addAction(m_pActionAbout);
	m_pActionAbout->setData(ACTION_ID_ABOUT);
	m_settableActionsList.push_back(m_pActionAbout);

//------------------------------------------------------------------------------

	m_pActionAutocomplete = new QAction(this);
	m_pActionAutocomplete->setIconText(trUtf8("Autocomplete"));
	hotkey = m_pSettingsManager->getHotkey(ACTION_ID_AUTOCOMPLETE);
	m_pActionAutocomplete->setShortcut(hotkey);
	m_ui.scriptEdit->addAction(m_pActionAutocomplete);
	m_pActionAutocomplete->setData(ACTION_ID_AUTOCOMPLETE);
	m_settableActionsList.push_back(m_pActionAutocomplete);

//------------------------------------------------------------------------------

	connect(m_pActionNewScript, SIGNAL(triggered()),
		this, SLOT(slotNewScript()));
	connect(m_pActionOpenScript, SIGNAL(triggered()),
		this, SLOT(slotOpenScript()));
	connect(m_pActionSaveScript, SIGNAL(triggered()),
		this, SLOT(slotSaveScript()));
	connect(m_pActionSaveScriptAs, SIGNAL(triggered()),
		this, SLOT(slotSaveScriptAs()));
	connect(m_pActionExit, SIGNAL(triggered()), this, SLOT(close()));
	connect(m_pActionSettings, SIGNAL(triggered()),
		m_pSettingsDialog, SLOT(slotCall()));
	connect(m_pActionPreview, SIGNAL(triggered()), this, SLOT(slotPreview()));
	connect(m_pActionCheckScript, SIGNAL(triggered()),
		this, SLOT(slotCheckScript()));
	connect(m_pActionAbout, SIGNAL(triggered()), this, SLOT(slotAbout()));
	connect(m_pActionAutocomplete, SIGNAL(triggered()),
		m_ui.scriptEdit, SLOT(slotComplete()));

//------------------------------------------------------------------------------

	ActionDataList settableActionsDataList;
	for(QAction * pAction : m_settableActionsList)
		settableActionsDataList.push_back(ActionData(pAction));
	m_pSettingsDialog->addSettableActions(settableActionsDataList);
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

	QMessageBox::StandardButton choice = QMessageBox::NoButton;
	if(m_scriptFilePath.isEmpty())
	{
		choice = QMessageBox::question(this, trUtf8("Save script?"),
			trUtf8("Would you like to save your script before closing?"),
			QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

		if(choice == QMessageBox::Yes)
		{
			bool saved = slotSaveScriptAs();
			if(!saved)
				return false;
		}
	}
	else
	{
		choice = QMessageBox::question(this, trUtf8("Save script?"),
			trUtf8("Would you like to save script \"%1\" before closing?")
			.arg(m_scriptFilePath),
			QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

		if(choice == QMessageBox::Yes)
		{
			bool saved = slotSaveScript();
			if(!saved)
				return false;
		}
	}

	if(choice == QMessageBox::Cancel)
		return false;

	return true;
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
