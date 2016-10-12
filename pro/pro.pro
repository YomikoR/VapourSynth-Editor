CONFIG += qt

QT += widgets

QT_VERSION_WARNING = "WARNING: Linking against Qt version lower than 5.6.1 is likely to cause CLI tools video encoding to crash due to I/O but in Qt."

equals(QT_MAJOR_VERSION, 5) {
	equals(QT_MINOR_VERSION, 6):lessThan(QT_PATCH_VERSION, 1)) {
		message($$QT_VERSION_WARNING)
	}
	lessThan(QT_MINOR_VERSION, 6) {
		message($$QT_VERSION_WARNING)
	}
}

HOST_64_BIT = contains(QMAKE_HOST.arch, "x86_64")
TARGET_64_BIT = contains(QMAKE_TARGET.arch, "x86_64")
ARCHITECTURE_64_BIT = $$HOST_64_BIT | $$TARGET_64_BIT

CONFIG(debug, debug|release) {

	contains(QMAKE_COMPILER, gcc) {
		if($$ARCHITECTURE_64_BIT) {
			DESTDIR = ../build/debug-64bit-gcc
			TARGET = vsedit-debug-64bit-gcc
			OBJECTS_DIR = ../generated/obj-debug-64bit-gcc
		} else {
			DESTDIR = ../build/debug-32bit-gcc
			TARGET = vsedit-debug-32bit-gcc
			OBJECTS_DIR = ../generated/obj-debug-32bit-gcc
		}

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

	contains(QMAKE_COMPILER, msvc) {
		if($$ARCHITECTURE_64_BIT) {
			DESTDIR = ../build/debug-64bit-msvc
			TARGET = vsedit-debug-64bit-msvc
			OBJECTS_DIR = ../generated/obj-debug-64bit-msvc
		} else {
			DESTDIR = ../build/debug-32bit-msvc
			TARGET = vsedit-debug-32bit-msvc
			OBJECTS_DIR = ../generated/obj-debug-32bit-msvc
		}
	}

} else {

	contains(QMAKE_COMPILER, gcc) {
		if($$ARCHITECTURE_64_BIT) {
			DESTDIR = ../build/release-64bit-gcc
			TARGET = vsedit
			OBJECTS_DIR = ../generated/obj-release-64bit-gcc
		} else {
			DESTDIR = ../build/release-32bit-gcc
			TARGET = vsedit-32bit
			OBJECTS_DIR = ../generated/obj-release-32bit-gcc
		}

		QMAKE_CXXFLAGS += -O2
		QMAKE_CXXFLAGS += -fexpensive-optimizations
		QMAKE_CXXFLAGS += -funit-at-a-time
	}

	contains(QMAKE_COMPILER, msvc) {
		if($$ARCHITECTURE_64_BIT) {
			DESTDIR = ../build/release-64bit-msvc
			TARGET = vsedit
			OBJECTS_DIR = ../generated/obj-release-64bit-msvc
		} else {
			DESTDIR = ../build/release-32bit-msvc
			TARGET = vsedit-32bit
			OBJECTS_DIR = ../generated/obj-release-32bit-msvc
		}
	}

	CONFIG += warn_off
	DEFINES += NDEBUG

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

macx {
	INCLUDEPATH += /usr/local/include
	ICON = ../resources/vsedit.icns
}

win32 {
	INCLUDEPATH += 'C:/Program Files (x86)/VapourSynth/sdk/include/'

	DEPLOY_COMMAND = windeployqt
	DEPLOY_TARGET = $$shell_quote($$shell_path($${D}/$${TARGET}.exe))
	QMAKE_POST_LINK += $${DEPLOY_COMMAND} --no-translations $${DEPLOY_TARGET} $${E}

	if($$ARCHITECTURE_64_BIT) {
		message("x86_64 build")
	} else {
		message("x86 build")
		contains(QMAKE_COMPILER, gcc) {
			QMAKE_LFLAGS += -Wl,--large-address-aware
		}
		contains(QMAKE_COMPILER, msvc) {
			QMAKE_LFLAGS += /LARGEADDRESSAWARE
		}
	}
}

contains(QMAKE_COMPILER, clang) {
	QMAKE_CXXFLAGS += -stdlib=libc++
}

contains(QMAKE_COMPILER, gcc) {
	QMAKE_CXXFLAGS += -std=c++11
	LIBS += -L$$[QT_INSTALL_LIBS]
} else {
	CONFIG += c++11
}

TEMPLATE = app

VER_MAJ = 9
VERSION = $$VER_MAJ

RC_ICONS = ../resources/vsedit.ico
QMAKE_TARGET_PRODUCT = 'VapourSynth Editor'
QMAKE_TARGET_COMPANY = 'Aleksey [Mystery Keeper] Lyashin'
QMAKE_TARGET_COPYRIGHT = $$QMAKE_TARGET_COMPANY
QMAKE_TARGET_DESCRIPTION = 'VapourSynth script editor'

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
HEADERS += ../src/vapoursynth/vs_script_processor_structures.h
HEADERS += ../src/vapoursynth/vapoursynthscriptprocessor.h
HEADERS += ../src/vapoursynth/vs_script_processor_dialog.h
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
SOURCES += ../src/vapoursynth/vs_script_processor_structures.cpp
SOURCES += ../src/vapoursynth/vapoursynthscriptprocessor.cpp
SOURCES += ../src/vapoursynth/vs_script_processor_dialog.cpp
SOURCES += ../src/frame_consumers/benchmark_dialog.cpp
SOURCES += ../src/frame_consumers/encode_dialog.cpp
SOURCES += ../src/mainwindow.cpp
SOURCES += ../src/main.cpp

include(local_quirks.pri)
