#include "job_server.h"

#include "../../common-src/application_instance_file_guard/application_instance_file_guard.h"
#include "../../common-src/ipc_defines.h"
#include <vapoursynth/VapourSynth.h>

#include <QCoreApplication>

Q_DECLARE_OPAQUE_POINTER(const VSFrameRef *)
Q_DECLARE_OPAQUE_POINTER(VSNodeRef *)

int main(int argc, char *argv[])
{
	QCoreApplication application(argc, argv);

	qRegisterMetaType<const VSFrameRef *>("const VSFrameRef *");
	qRegisterMetaType<VSNodeRef *>("VSNodeRef *");

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
