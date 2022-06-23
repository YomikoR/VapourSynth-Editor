#include "job_server.h"

#include "../../common-src/application_instance_file_guard/application_instance_file_guard.h"
#include "../../common-src/ipc_defines.h"
#include <vapoursynth/VapourSynth4.h>

#include <QCoreApplication>

Q_DECLARE_OPAQUE_POINTER(const VSFrame *)
Q_DECLARE_OPAQUE_POINTER(VSNode *)

int main(int argc, char *argv[])
{
	QCoreApplication application(argc, argv);

	qRegisterMetaType<const VSFrame *>("const VSFrame *");
	qRegisterMetaType<VSNode *>("VSNode *");

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

	if(!guard.unlock())
		qCritical("%s", guard.error().toLocal8Bit().data());

	return exitCode;
}
