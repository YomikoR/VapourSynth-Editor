#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <ui_main_window.h>

#include <vector>

class SettingsManager;
class VapourSynthPluginsManager;
class VSScriptLibrary;
class PreviewDialog;
class SettingsDialog;
class ScriptBenchmarkDialog;
class EncodeDialog;
class TemplatesDialog;
class JobServerWatcherSocket;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:

	MainWindow();

	virtual ~MainWindow();

public slots:

	void slotWriteLogMessage(int a_messageType, const QString & a_message);
	void slotWriteLogMessage(const QString & a_message,
		const QString & a_style = LOG_STYLE_DEFAULT);

	void slotInsertTextIntoScriptAtNewLine(const QString & a_text);
	void slotInsertTextIntoScriptAtCursor(const QString & a_text);

protected:

	void closeEvent(QCloseEvent * a_pEvent) override;

	void moveEvent(QMoveEvent * a_pEvent) override;

	void resizeEvent(QResizeEvent * a_pEvent) override;

	void changeEvent(QEvent * a_pEvent) override;

private slots:

	void slotNewScript();
	bool slotSaveScript();
	bool slotSaveScriptAs();
	bool slotOpenScript();

	void slotTemplates();

	void slotPreview();
	void slotCheckScript();
	void slotBenchmark();
	void slotEncode();
	void slotEnqueueEncodeJob();
	void slotJobs();

	void slotAbout();

	void slotChangeWindowTitle();

	void slotEditorTextChanged();

	void slotOpenRecentScriptActionTriggered();

	void slotSettingsChanged();

	void slotScriptFileDropped(const QString & a_filePath, bool * a_pHandled);

private:

	void createActionsAndMenus();

	void fillRecentScriptsMenu();

	bool saveScriptToFile(const QString& a_filePath);

	bool loadScriptFromFile(const QString& a_filePath);

	bool safeToCloseFile();

	void setCurrentScriptFilePath(const QString & a_filePath);

	void loadStartUpScript();

	void loadFonts();

	void destroyOrphanQObjects();

	Ui::MainWindow m_ui;

	SettingsManager * m_pSettingsManager;
	VapourSynthPluginsManager * m_pVapourSynthPluginsManager;
	VSScriptLibrary * m_pVSScriptLibrary;

	QAction * m_pActionNewScript;
	QAction * m_pActionOpenScript;
	QAction * m_pActionSaveScript;
	QAction * m_pActionSaveScriptAs;
	QAction * m_pActionTemplates;
	QAction * m_pActionSettings;
	QAction * m_pActionPreview;
	QAction * m_pActionCheckScript;
	QAction * m_pActionBenchmark;
	QAction * m_pActionEncode;
	QAction * m_pActionEnqueueEncodeJob;
	QAction * m_pActionJobs;
	QAction * m_pActionExit;
	QAction * m_pActionAbout;

	std::vector<QAction *> m_settableActionsList;

	QMenu * m_pMenuRecentScripts;

	PreviewDialog * m_pPreviewDialog;
	SettingsDialog * m_pSettingsDialog;
	ScriptBenchmarkDialog * m_pBenchmarkDialog;
	EncodeDialog * m_pEncodeDialog;
	TemplatesDialog * m_pTemplatesDialog;

	QString m_scriptFilePath;
	QString m_lastSavedText;

	std::vector<QObject **> m_orphanQObjects;

	JobServerWatcherSocket * m_pJobServerWatcherSocket;
};

#endif // MAINWINDOW_H
