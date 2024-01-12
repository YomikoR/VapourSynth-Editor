#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <ui_main_window.h>

#include "../../common-src/settings/settings_definitions_core.h"
#include "../../common-src/settings/settings_definitions.h"

#include <QSystemTrayIcon>
#include <QWebSocket>
#include <QJsonObject>
#include <QHostAddress>
#include <list>

class SettingsManager;
class JobsModel;
class JobEditDialog;
class JobStateDelegate;
class JobDependenciesDelegate;
class VSScriptLibrary;
class QMenu;
class ConnectToServerDialog;
class QLocalServer;
class QLocalSocket;
class TrustedClientsAddressesDialog;
class QTimer;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:

	MainWindow(SettingsManager *settings);

	virtual ~MainWindow();

	void showAndConnect();

public slots:

	void show();
	void close();
	void slotWriteLogMessage(int a_messageType, const QString & a_message);
	void slotWriteLogMessage(const QString & a_message,
		const QString & a_style = LOG_STYLE_DEFAULT);

protected:

	virtual void moveEvent(QMoveEvent * a_pEvent) override;
	virtual void resizeEvent(QResizeEvent * a_pEvent) override;
	virtual void changeEvent(QEvent * a_pEvent) override;
	virtual void showEvent(QShowEvent * a_pEvent) override;
	virtual void closeEvent(QCloseEvent * a_pEvent) override;

private slots:

	void slotTrayIconActivated(QSystemTrayIcon::ActivationReason a_reason);

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

	void slotConnectToServerDialog();

	void slotReconnectToServer();
	void slotConnectToServer(const QHostAddress & a_address);
	void slotConnectToLocalServer();

	void slotShutdownServerAndExit();

	void slotTaskServerNewConnection();
	void slotTaskClientReadyRead();
	void slotTaskClientDisconnected();

	void slotSetTrustedClientsAddresses();

	void slotSaveGeometry();

private:

	enum class WatcherState
	{
		NotConnected,
		ProbingLocal,
		StartingLocal,
		Connecting,
		Connected,
		Disconnecting,
		SwitchingServer,
		ShuttingDown,
		ClosingServerShuttingDown,
	};

	void createActionsAndMenus();

	void editJob(const QModelIndex & a_index);

	void processSMsgJobInfo(const QString & a_message);

	std::vector<int> selectedIndexes();

	void setUiEnabled();

	void resetWindowTitle(int a_jobIndex);

	void changeState(WatcherState a_newState);

	void processTaskList();

	void saveGeometryDelayed();

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

	WatcherState m_state;

	QSystemTrayIcon * m_pTrayIcon;

	QMenu * m_pTrayMenu;

	QAction * m_pActionSetTrustedClientsAddresses;
	QAction * m_pActionExit;
	QAction * m_pActionShutdownServerAndExit;

	ConnectToServerDialog * m_pConnectToServerDialog;

	QHostAddress m_nextServerAddress;

	QLocalServer * m_pTaskServer;
	std::list<QLocalSocket *> m_taskClients;

	std::list<JobProperties> m_taskList;

	QStringList m_trustedClientsAddresses;

	QTimer * m_pGeometrySaveTimer;
	QByteArray m_windowGeometry;

};

#endif // MAINWINDOW_H
