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
	QT += winextras

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
	QMAKE_CXXFLAGS += -Wall
	QMAKE_CXXFLAGS += -Wextra
	QMAKE_CXXFLAGS += -Wredundant-decls
	QMAKE_CXXFLAGS += -Wshadow
	#QMAKE_CXXFLAGS += -Weffc++
	QMAKE_CXXFLAGS += -pedantic

	LIBS += -L$$[QT_INSTALL_LIBS]
} else {
	CONFIG += c++11
}

TEMPLATE = app

VER_MAJ = 15
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

FORMS += ../src/settings/settings_dialog.ui
FORMS += ../src/log/styled_log_view_settings_dialog.ui
FORMS += ../src/script_status_bar_widget/script_status_bar_widget.ui
FORMS += ../src/preview/preview_advanced_settings_dialog.ui
FORMS += ../src/preview/preview_dialog.ui
FORMS += ../src/frame_consumers/benchmark_dialog.ui
FORMS += ../src/frame_consumers/encode_dialog.ui
FORMS += ../src/script_templates/templates_dialog.ui
FORMS += ../src/jobs/job_edit_dialog.ui
FORMS += ../src/jobs/jobs_dialog.ui
FORMS += ../src/main_window.ui

HEADERS += ../src/common/helpers.h
HEADERS += ../src/common/aligned_vector.h
HEADERS += ../src/common/chrono.h
HEADERS += ../src/settings/settings_definitions.h
HEADERS += ../src/settings/settings_manager.h
HEADERS += ../src/settings/actions_hotkey_edit_model.h
HEADERS += ../src/settings/clearable_key_sequence_editor.h
HEADERS += ../src/settings/item_delegate_for_hotkey.h
HEADERS += ../src/settings/theme_elements_model.h
HEADERS += ../src/settings/settings_dialog.h
HEADERS += ../src/log/styled_log_view_structures.h
HEADERS += ../src/log/log_styles_model.h
HEADERS += ../src/log/styled_log_view_settings_dialog.h
HEADERS += ../src/log/styled_log_view.h
HEADERS += ../src/log/vs_editor_log.h
HEADERS += ../src/script_status_bar_widget/script_status_bar_widget.h
HEADERS += ../src/preview/scroll_navigator.h
HEADERS += ../src/preview/preview_area.h
HEADERS += ../src/preview/timeline_slider.h
HEADERS += ../src/preview/preview_advanced_settings_dialog.h
HEADERS += ../src/preview/preview_dialog.h
HEADERS += ../src/script_editor/number_matcher.h
HEADERS += ../src/script_editor/syntax_highlighter.h
HEADERS += ../src/script_editor/script_completer_model.h
HEADERS += ../src/script_editor/script_completer.h
HEADERS += ../src/script_editor/script_editor.h
HEADERS += ../src/vapoursynth/vs_plugin_data.h
HEADERS += ../src/vapoursynth/vapoursynth_plugins_manager.h
HEADERS += ../src/vapoursynth/vs_script_library.h
HEADERS += ../src/vapoursynth/vs_script_processor_structures.h
HEADERS += ../src/vapoursynth/vapoursynth_script_processor.h
HEADERS += ../src/vapoursynth/vs_script_processor_dialog.h
HEADERS += ../src/frame_consumers/benchmark_dialog.h
HEADERS += ../src/frame_consumers/frame_header_writers/frame_header_writer.h
HEADERS += ../src/frame_consumers/frame_header_writers/frame_header_writer_null.h
HEADERS += ../src/frame_consumers/frame_header_writers/frame_header_writer_y4m.h
HEADERS += ../src/frame_consumers/encode_dialog.h
HEADERS += ../src/script_templates/drop_file_category_model.h
HEADERS += ../src/script_templates/templates_dialog.h
HEADERS += ../src/jobs/job.h
HEADERS += ../src/jobs/job_edit_dialog.h
HEADERS += ../src/jobs/jobs_dialog.h
HEADERS += ../src/main_window.h

SOURCES += ../src/common/helpers.cpp
SOURCES += ../src/settings/settings_definitions.cpp
SOURCES += ../src/settings/settings_manager.cpp
SOURCES += ../src/settings/actions_hotkey_edit_model.cpp
SOURCES += ../src/settings/clearable_key_sequence_editor.cpp
SOURCES += ../src/settings/item_delegate_for_hotkey.cpp
SOURCES += ../src/settings/theme_elements_model.cpp
SOURCES += ../src/settings/settings_dialog.cpp
SOURCES += ../src/log/styled_log_view_structures.cpp
SOURCES += ../src/log/log_styles_model.cpp
SOURCES += ../src/log/styled_log_view_settings_dialog.cpp
SOURCES += ../src/log/styled_log_view.cpp
SOURCES += ../src/log/vs_editor_log.cpp
SOURCES += ../src/script_status_bar_widget/script_status_bar_widget.cpp
SOURCES += ../src/preview/scroll_navigator.cpp
SOURCES += ../src/preview/preview_area.cpp
SOURCES += ../src/preview/timeline_slider.cpp
SOURCES += ../src/preview/preview_advanced_settings_dialog.cpp
SOURCES += ../src/preview/preview_dialog.cpp
SOURCES += ../src/script_editor/number_matcher.cpp
SOURCES += ../src/script_editor/syntax_highlighter.cpp
SOURCES += ../src/script_editor/script_completer_model.cpp
SOURCES += ../src/script_editor/script_completer.cpp
SOURCES += ../src/script_editor/script_editor.cpp
SOURCES += ../src/vapoursynth/vs_plugin_data.cpp
SOURCES += ../src/vapoursynth/vapoursynth_plugins_manager.cpp
SOURCES += ../src/vapoursynth/vs_script_library.cpp
SOURCES += ../src/vapoursynth/vs_script_processor_structures.cpp
SOURCES += ../src/vapoursynth/vapoursynth_script_processor.cpp
SOURCES += ../src/vapoursynth/vs_script_processor_dialog.cpp
SOURCES += ../src/frame_consumers/benchmark_dialog.cpp
SOURCES += ../src/frame_consumers/frame_header_writers/frame_header_writer.cpp
SOURCES += ../src/frame_consumers/frame_header_writers/frame_header_writer_null.cpp
SOURCES += ../src/frame_consumers/frame_header_writers/frame_header_writer_y4m.cpp
SOURCES += ../src/frame_consumers/encode_dialog.cpp
SOURCES += ../src/script_templates/drop_file_category_model.cpp
SOURCES += ../src/script_templates/templates_dialog.cpp
SOURCES += ../src/jobs/job.cpp
SOURCES += ../src/jobs/job_edit_dialog.cpp
SOURCES += ../src/jobs/jobs_dialog.cpp
SOURCES += ../src/main_window.cpp
SOURCES += ../src/main.cpp

include(local_quirks.pri)
