CONFIG += qt

QT += widgets

CONFIG(debug, debug|release) {

	contains(QMAKE_HOST.arch, x86_64) {
		contains(QMAKE_COMPILER, gcc) {
			DESTDIR = ../build/debug-64bit-gcc
			TARGET = vsedit-debug-64bit-gcc
			OBJECTS_DIR = ../generated/obj-debug-64bit-gcc
		}
		contains(QMAKE_COMPILER, msvc) {
			DESTDIR = ../build/debug-64bit-msvc
			TARGET = vsedit-debug-64bit-msvc
			OBJECTS_DIR = ../generated/obj-debug-64bit-msvc
		}
	} else {
		contains(QMAKE_COMPILER, gcc) {
			DESTDIR = ../build/debug-32bit-gcc
			TARGET = vsedit-debug-32bit-gcc
			OBJECTS_DIR = ../generated/obj-debug-32bit-gcc
		}
		contains(QMAKE_COMPILER, msvc) {
			DESTDIR = ../build/debug-32bit-msvc
			TARGET = vsedit-debug-32bit-msvc
			OBJECTS_DIR = ../generated/obj-debug-32bit-msvc
		}
	}

	contains(QMAKE_COMPILER, gcc) {
		QMAKE_CXXFLAGS += -O0
		QMAKE_CXXFLAGS += -g
		QMAKE_CXXFLAGS += -ggdb3
		QMAKE_CXXFLAGS += -Wall
		QMAKE_CXXFLAGS += -Wextra
		QMAKE_CXXFLAGS += -Wredundant-decls
		QMAKE_CXXFLAGS += -Wshadow
		#QMAKE_CXXFLAGS += -Weffc++
		QMAKE_CXXFLAGS += -pedantic
	}

} else {

	CONFIG += warn_off

	contains(QMAKE_HOST.arch, x86_64) {
		contains(QMAKE_COMPILER, gcc) {
			DESTDIR = ../build/release-64bit-gcc
			TARGET = vsedit
			OBJECTS_DIR = ../generated/obj-release-64bit-gcc
		}
		contains(QMAKE_COMPILER, msvc) {
			DESTDIR = ../build/release-64bit-msvc
			TARGET = vsedit
			OBJECTS_DIR = ../generated/obj-release-64bit-msvc
		}
	} else {
		contains(QMAKE_COMPILER, gcc) {
			DESTDIR = ../build/release-32bit-gcc
			TARGET = vsedit-32bit
			OBJECTS_DIR = ../generated/obj-release-32bit-gcc
		}
		contains(QMAKE_COMPILER, msvc) {
			DESTDIR = ../build/release-32bit-msvc
			TARGET = vsedit-32bit
			OBJECTS_DIR = ../generated/obj-release-32bit-msvc
		}
	}

	DEFINES += NDEBUG

	contains(QMAKE_COMPILER, gcc) {
		QMAKE_CXXFLAGS += -O2
		QMAKE_CXXFLAGS += -fexpensive-optimizations
		QMAKE_CXXFLAGS += -funit-at-a-time
	}
}

macx {
	INCLUDEPATH += /usr/local/include
	ICON = ../resources/vsedit.icns
}

win32 {
	INCLUDEPATH += 'C:/Program Files (x86)/VapourSynth/sdk/include/'

	contains(QMAKE_HOST.arch, x86_64) {
		message("x86_64 build")
		contains(QMAKE_COMPILER, msvc) {
			QMAKE_LIBDIR += "E:/SDK/msvc-2013-express/VC/lib/amd64/"
			QMAKE_LIBDIR += "C:/Program Files (x86)/Windows Kits/8.1/Lib/winv6.3/um/x64/"
		}
	} else {
		message("x86 build")
		contains(QMAKE_COMPILER, gcc) {
			QMAKE_LFLAGS += -Wl,--large-address-aware
		}
		contains(QMAKE_COMPILER, msvc) {
			QMAKE_LIBDIR += "E:/SDK/msvc-2013-express/VC/lib/"
			QMAKE_LIBDIR += "C:/Program Files (x86)/Windows Kits/8.1/Lib/winv6.3/um/x86/"
			QMAKE_LFLAGS += /LARGEADDRESSAWARE
		}
	}
}

contains(QMAKE_COMPILER, clang){
	QMAKE_CXXFLAGS += -stdlib=libc++
}

contains(QMAKE_COMPILER, gcc) {
	QMAKE_CXXFLAGS += -std=c++11
	LIBS += -L$$[QT_INSTALL_LIBS]
} else {
	CONFIG += c++11
}

S = $${DIR_SEPARATOR}
D = $$DESTDIR
D = $$replace(D, $$escape_expand(\\), $$S)
D = $$replace(D, /, $$S)
E = $$escape_expand(\n\t)

QMAKE_POST_LINK += $${QMAKE_COPY} ..$${S}resources$${S}vsedit.ico $${D}$${S}vsedit.ico $${E}
QMAKE_POST_LINK += $${QMAKE_COPY} ..$${S}resources$${S}vsedit.svg $${D}$${S}vsedit.svg $${E}
QMAKE_POST_LINK += $${QMAKE_COPY} ..$${S}README $${D}$${S}README $${E}
QMAKE_POST_LINK += $${QMAKE_COPY} ..$${S}LICENSE $${D}$${S}LICENSE $${E}
QMAKE_POST_LINK += $${QMAKE_COPY} ..$${S}CHANGELOG $${D}$${S}CHANGELOG $${E}

TEMPLATE = app

RC_FILE = ../resources/windowsresources.rc

#SUBDIRS

MOC_DIR = ../generated/moc
UI_DIR = ../generated/ui
RCC_DIR = ../generated/rcc

#DEFINES

#TRANSLATIONS

RESOURCES = ../resources/resources.qrc

FORMS += ../src/settings/settingsdialog.ui
FORMS += ../src/preview/preview_advanced_settings_dialog.ui
FORMS += ../src/preview/previewdialog.ui
FORMS += ../src/frame_consumers/benchmark_dialog.ui
FORMS += ../src/frame_consumers/encode_dialog.ui
FORMS += ../src/mainwindow.ui

HEADERS += ../src/common/helpers.h
HEADERS += ../src/common/aligned_vector.h
HEADERS += ../src/common/chrono.h
HEADERS += ../src/settings/settingsmanager.h
HEADERS += ../src/settings/actionshotkeyeditmodel.h
HEADERS += ../src/settings/itemdelegateforhotkey.h
HEADERS += ../src/settings/theme_elements_model.h
HEADERS += ../src/settings/settingsdialog.h
HEADERS += ../src/preview/scrollnavigator.h
HEADERS += ../src/preview/previewarea.h
HEADERS += ../src/preview/timelineslider.h
HEADERS += ../src/preview/preview_advanced_settings_dialog.h
HEADERS += ../src/preview/previewdialog.h
HEADERS += ../src/scripteditor/numbermatcher.h
HEADERS += ../src/scripteditor/syntaxhighlighter.h
HEADERS += ../src/scripteditor/scriptcompletermodel.h
HEADERS += ../src/scripteditor/scriptcompleter.h
HEADERS += ../src/scripteditor/scripteditor.h
HEADERS += ../src/vapoursynth/vsplugindata.h
HEADERS += ../src/vapoursynth/vapoursynthpluginsmanager.h
HEADERS += ../src/vapoursynth/vapoursynthscriptprocessor.h
HEADERS += ../src/frame_consumers/benchmark_dialog.h
HEADERS += ../src/frame_consumers/encode_dialog.h
HEADERS += ../src/mainwindow.h

SOURCES += ../src/common/helpers.cpp
SOURCES += ../src/settings/settingsmanager.cpp
SOURCES += ../src/settings/actionshotkeyeditmodel.cpp
SOURCES += ../src/settings/itemdelegateforhotkey.cpp
SOURCES += ../src/settings/theme_elements_model.cpp
SOURCES += ../src/settings/settingsdialog.cpp
SOURCES += ../src/preview/scrollnavigator.cpp
SOURCES += ../src/preview/previewarea.cpp
SOURCES += ../src/preview/timelineslider.cpp
SOURCES += ../src/preview/preview_advanced_settings_dialog.cpp
SOURCES += ../src/preview/previewdialog.cpp
SOURCES += ../src/scripteditor/numbermatcher.cpp
SOURCES += ../src/scripteditor/syntaxhighlighter.cpp
SOURCES += ../src/scripteditor/scriptcompletermodel.cpp
SOURCES += ../src/scripteditor/scriptcompleter.cpp
SOURCES += ../src/scripteditor/scripteditor.cpp
SOURCES += ../src/vapoursynth/vsplugindata.cpp
SOURCES += ../src/vapoursynth/vapoursynthpluginsmanager.cpp
SOURCES += ../src/vapoursynth/vapoursynthscriptprocessor.cpp
SOURCES += ../src/frame_consumers/benchmark_dialog.cpp
SOURCES += ../src/frame_consumers/encode_dialog.cpp
SOURCES += ../src/mainwindow.cpp
SOURCES += ../src/main.cpp
