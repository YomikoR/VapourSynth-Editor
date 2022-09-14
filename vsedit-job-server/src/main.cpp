#include "job_server.h"

#include "../../common-src/application_instance_file_guard/application_instance_file_guard.h"
#include "../../common-src/ipc_defines.h"
#include "../../common-src/version_info.h"
#include <vapoursynth/VapourSynth.h>

#include <QCoreApplication>

Q_DECLARE_OPAQUE_POINTER(const VSFrameRef *)
Q_DECLARE_OPAQUE_POINTER(VSNodeRef *)

int main(int argc, char *argv[])
{
	if(argc > 1)
	{
		if(strcmp(argv[1], "-v") == 0 ||
			strcmp(argv[1], "--version") == 0)
		{
			print_version();
			return 0;
		}
	}

#if defined(Q_OS_WIN)
 	hide_tty();
#endif

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

	if(!guard.unlock())
		qCritical("%s", guard.error().toLocal8Bit().data());

	return exitCode;
}
