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

	JobServer jobServer;

	application.connect(&jobServer, &JobServer::finish,
		&application, &QCoreApplication::quit);

	bool started = jobServer.start();
	if(!started)
	{
		qCritical("Couldn't start the server.");
		return 1;
	}

	int exitCode = application.exec();
	return exitCode;
}
