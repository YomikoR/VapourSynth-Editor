#include "application_instance_file_guard.h"

#include <QCoreApplication>

int main(int argc, char *argv[])
{
	ApplicationInstanceFileGuard guard("vsedit_job_server_running");
	if(!guard.isLocked())
	{
		qInfo("Couldn't start the server. "
			"Another instance is probably already running.");
		return 1;
	}

	QCoreApplication application(argc, argv);
	int exitCode = application.exec();
	return exitCode;
}
