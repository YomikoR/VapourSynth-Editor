#include <QApplication>
#include <vapoursynth/VSScript.h>

#include "mainwindow.h"

Q_DECLARE_OPAQUE_POINTER(const VSFrameRef *)
Q_DECLARE_OPAQUE_POINTER(VSNodeRef *)

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);

	qRegisterMetaType<const VSFrameRef *>("const VSFrameRef *");
	qRegisterMetaType<VSNodeRef *>("VSNodeRef *");

	MainWindow mainWindow;
	mainWindow.show();
	return application.exec();
}
