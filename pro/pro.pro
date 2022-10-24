lessThan(QT_MAJOR_VERSION, 6) {
	error(You must build with Qt 6 or later)
}

TEMPLATE = subdirs

SUBDIRS += vsedit
SUBDIRS += vsedit-job-server
SUBDIRS += vsedit-job-server-watcher

vsedit.file = ./vsedit/vsedit.pro
vsedit-job-server.file = ./vsedit-job-server/vsedit-job-server.pro
vsedit-job-server-watcher.file = ./vsedit-job-server-watcher/vsedit-job-server-watcher.pro
