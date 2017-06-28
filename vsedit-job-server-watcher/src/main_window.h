#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <ui_main_window.h>

#include "../../common-src/settings/settings_definitions_core.h"
#include "../../common-src/settings/settings_definitions.h"

#include <QWebSocket>
#include <QJsonObject>

class SettingsManager;
class JobsModel;
class JobEditDialog;
class JobStateDelegate;
class JobDependenciesDelegate;
class VSScriptLibrary;
class QMenu;
class QSystemTrayIcon;

#ifdef Q_OS_WIN
	class QWinTaskbarButton;
	class QWinTaskbarProgress;
#endif

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:

	MainWindow();

	virtual ~MainWindow();

	void showAndConnect();

public slots:

	void show();
	void slotWriteLogMessage(int a_messageType, const QString & a_message);
	void slotWriteLogMessage(const QString & a_message,
		const QString & a_style = LOG_STYLE_DEFAULT);

protected:

	virtual void moveEvent(QMoveEvent * a_pEvent) override;
	virtual void resizeEvent(QResizeEvent * a_pEvent) override;
	virtual void changeEvent(QEvent * a_pEvent) override;
	virtual void showEvent(QShowEvent * a_pEvent) override;

private slots:

	void slotJobNewButtonClicked();
	void slotJobEditButtonClicked();
	void slotJobMoveUpButtonClicked();
	void slotJobMoveDownButtonClicked();
	void slotJobDeleteButtonClicked();
	void slotJobResetStateButtonClicked();
	void slotStartButtonClicked();
	void slotPauseButtonClicked();
	void slotResumeButtonClicked();
	void slotAbortButtonClicked();

	void slotJobDoubleClicked(const QModelIndex & a_index);

	void slotSelectionChanged();

	void slotSaveHeaderState();

	void slotJobsHeaderContextMenu(const QPoint & a_point);
	void slotShowJobsHeaderSection(bool a_show);

	void slotJobStateChanged(int a_job, JobState a_state);
	void slotJobProgressChanged(int a_job, int a_progress, int a_progressMax);
	void slotSetJobDependencies(const QUuid & a_id,
		std::vector<QUuid> a_dependencies);

	void slotServerConnected();
	void slotServerDisconnected();
	void slotBinaryMessageReceived(const QByteArray & a_message);
	void slotTextMessageReceived(const QString & a_message);
	void slotServerError(QAbstractSocket::SocketError a_error);

	void slotStartLocalServer();
	void slotShutdownServer();

	void slotConnectToLocalServer();

private:

	enum class ServerState
	{
		NotConnected,
		ProbingLocal,
		StartingLocal,
		ShuttingDown,
		Connecting,
		Connected,
		Disconnecting,
	};

	void createActionsAndMenus();

	void saveGeometrySettings();

	void editJob(const QModelIndex & a_index);

	void processSMsgJobInfo(const QString & a_message);

	std::vector<int> selectedIndexes();

	void setUiEnabled();

	void resetWindowTitle(int a_jobIndex);

	static const char WINDOW_TITLE[];

	Ui::MainWindow m_ui;

	SettingsManager * m_pSettingsManager;

	JobsModel * m_pJobsModel;
	JobStateDelegate * m_pJobStateDelegate;
	JobDependenciesDelegate * m_pJobDependenciesDelegate;

	VSScriptLibrary * m_pVSScriptLibrary;
	JobEditDialog * m_pJobEditDialog;

	QMenu * m_pJobsHeaderMenu;

	QWebSocket * m_pServerSocket;

	int m_connectionAttempts;
	int m_maxConnectionAttempts;

	ServerState m_serverState;

	QSystemTrayIcon * m_pTrayIcon;

#ifdef Q_OS_WIN
	QWinTaskbarButton * m_pWinTaskbarButton;
	QWinTaskbarProgress * m_pWinTaskbarProgress;
#endif
};

#endif // MAINWINDOW_H
