#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <vector>

#include <ui_mainwindow.h>

class SettingsManager;
class VapourSynthPluginsManager;
class VSScriptLibrary;
class PreviewDialog;
class SettingsDialog;
class ScriptBenchmarkDialog;
class EncodeDialog;

class MainWindow : public QMainWindow
{
	Q_OBJECT

	public:

		MainWindow();

		virtual ~MainWindow();

	public slots:

		void slotWriteLogMessage(int a_messageType, const QString & a_message);

		void slotInsertLineIntoScript(const QString & a_line);

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

		void slotPreview();

		void slotCheckScript();

		void slotBenchmark();

		void slotEncode();

		void slotAbout();

		void slotChangeWindowTitle();

		void slotEditorTextChanged();

		void slotOpenRecentScriptActionTriggered();

		void slotSettingsChanged();

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
		QAction * m_pActionSettings;
		QAction * m_pActionPreview;
		QAction * m_pActionCheckScript;
		QAction * m_pActionBenchmark;
		QAction * m_pActionEncode;
		QAction * m_pActionExit;
		QAction * m_pActionAbout;
		QAction * m_pActionAutocomplete;

		std::vector<QAction *> m_settableActionsList;

		QMenu * m_pMenuRecentScripts;

		PreviewDialog * m_pPreviewDialog;
		SettingsDialog * m_pSettingsDialog;
		ScriptBenchmarkDialog * m_pBenchmarkDialog;
		EncodeDialog * m_pEncodeDialog;

		QString m_scriptFilePath;
		QString m_lastSavedText;

		std::vector<QObject **> m_orphanQObjects;
};

#endif // MAINWINDOW_H
