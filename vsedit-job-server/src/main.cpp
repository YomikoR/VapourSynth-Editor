#include "web_socket_job_server.h"

#include "../../common-src/application_instance_file_guard/application_instance_file_guard.h"
#include "../../common-src/ipc_defines.h"

#include <QCoreApplication>

int main(int argc, char *argv[])
{
	QCoreApplication application(argc, argv);

	ApplicationInstanceFileGuard guard("vsedit_job_server_running");
	if(!guard.isLocked())
	{
		qCritical("Couldn't start the server. "
			"Another instance is probably already running.");
		return 1;
	}

	VSWebSocketJobServer jobServer(JOB_SERVER_NAME,
		QWebSocketServer::NonSecureMode);

	application.connect(&jobServer, &VSWebSocketJobServer::finish,
		&application, &QCoreApplication::quit);

	bool listening = jobServer.listen(QHostAddress::Any, JOB_SERVER_PORT);
	if(!listening)
	{
		qInfo("Couldn't start the server.");
		return 1;
	}

	int exitCode = application.exec();
	return exitCode;
}
