lessThan(QT_MAJOR_VERSION, 6) {
	error(You must build with Qt 6 or later)
}

CONFIG += qt

QT += widgets
QT += multimedia

win32 {
	CONFIG += console
}

PROJECT_DIRECTORY = ../vse-previewer
COMMON_DIRECTORY = ..

CONFIG(debug, debug|release) {

	contains(QMAKE_COMPILER, gcc) {
		DESTDIR = $${COMMON_DIRECTORY}/build/debug-gcc
		TARGET = vse-previewer-debug-gcc
		OBJECTS_DIR = $${PROJECT_DIRECTORY}/generated/obj-debug-gcc

		QMAKE_CXXFLAGS += -O0
		QMAKE_CXXFLAGS += -g
		QMAKE_CXXFLAGS += -ggdb3
	}

	contains(QMAKE_COMPILER, msvc) {
		DESTDIR = $${COMMON_DIRECTORY}/build/debug-msvc
		TARGET = vse-previewer-debug-msvc
		OBJECTS_DIR = $${PROJECT_DIRECTORY}/generated/obj-debug-msvc
	}

} else {
	contains(QMAKE_COMPILER, gcc) {
		DESTDIR = $${COMMON_DIRECTORY}/build/release-gcc
		TARGET = vse-previewer
		OBJECTS_DIR = $${PROJECT_DIRECTORY}/generated/obj-release-gcc

		QMAKE_CXXFLAGS += -O2
		QMAKE_CXXFLAGS += -fexpensive-optimizations
		QMAKE_CXXFLAGS += -funit-at-a-time
	}

	macx {
		QMAKE_CXXFLAGS -= -fexpensive-optimizations
	}

	contains(QMAKE_COMPILER, msvc) {
		DESTDIR = $${COMMON_DIRECTORY}/build/release-msvc
		TARGET = vse-previewer
		OBJECTS_DIR = $${PROJECT_DIRECTORY}/generated/obj-release-msvc
	}

	DEFINES += NDEBUG

}

S = $${DIR_SEPARATOR}

D = $$DESTDIR
D = $$replace(D, /, $$S)

SC = $${COMMON_DIRECTORY}/
SC = $$replace(SC, /, $$S)

E = $$escape_expand(\n\t)

QMAKE_POST_LINK += $${QMAKE_COPY} $${SC}$${S}README $${D}$${S}README $${E}
QMAKE_POST_LINK += $${QMAKE_COPY} $${SC}$${S}LICENSE $${D}$${S}LICENSE $${E}
QMAKE_POST_LINK += $${QMAKE_COPY} $${SC}$${S}CHANGELOG $${D}$${S}CHANGELOG $${E}

macx {
	INCLUDEPATH += /usr/local/include
	ICON = $${COMMON_DIRECTORY}/resources/vsedit.icns
}

win32 {
	INCLUDEPATH += 'C:/Program Files/VapourSynth/sdk/include/'

#	DEPLOY_COMMAND = windeployqt
#	DEPLOY_TARGET = $$shell_quote($$shell_path($${D}/$${TARGET}.exe))
#	QMAKE_POST_LINK += $${DEPLOY_COMMAND} --no-translations --no-svg --no-opengl-sw --no-system-d3d-compiler $${DEPLOY_TARGET} $${E}

}

contains(QMAKE_COMPILER, clang) {
	QMAKE_CXXFLAGS += -stdlib=libc++
}

contains(QMAKE_COMPILER, gcc) {
	QMAKE_CXXFLAGS += -std=c++17
	QMAKE_CXXFLAGS += -Wall
	QMAKE_CXXFLAGS += -Wextra
	QMAKE_CXXFLAGS += -Wredundant-decls
	QMAKE_CXXFLAGS += -Wshadow
	#QMAKE_CXXFLAGS += -Weffc++
	QMAKE_CXXFLAGS += -pedantic

	LIBS += -L$$[QT_INSTALL_LIBS]
} else {
	CONFIG += c++17
}

include($${COMMON_DIRECTORY}/pro/common.pri)

TEMPLATE = app

RC_ICONS = $${COMMON_DIRECTORY}/resources/vsedit.ico
QMAKE_TARGET_PRODUCT = 'VSE-Previewer'
QMAKE_TARGET_DESCRIPTION = 'Previewer by VapourSynth Editor'

#SUBDIRS

MOC_DIR = $${PROJECT_DIRECTORY}/generated/moc
UI_DIR = $${PROJECT_DIRECTORY}/generated/ui
RCC_DIR = $${PROJECT_DIRECTORY}/generated/rcc

#DEFINES

#TRANSLATIONS

RESOURCES = $${COMMON_DIRECTORY}/resources/vsedit.qrc

FORMS += $${PROJECT_DIRECTORY}/src/settings/settings_dialog.ui
FORMS += $${PROJECT_DIRECTORY}/src/script_status_bar_widget/script_status_bar_widget.ui
FORMS += $${PROJECT_DIRECTORY}/src/preview/preview_advanced_settings_dialog.ui
FORMS += $${PROJECT_DIRECTORY}/src/preview/preview_dialog.ui

HEADERS += $${PROJECT_DIRECTORY}/src/helpers.h
HEADERS += $${PROJECT_DIRECTORY}/src/helpers_vs.h
HEADERS += $${PROJECT_DIRECTORY}/src/version_info.h
HEADERS += $${PROJECT_DIRECTORY}/src/chrono.h
HEADERS += $${PROJECT_DIRECTORY}/src/settings/settings_definitions_core.h
HEADERS += $${PROJECT_DIRECTORY}/src/settings/settings_definitions.h
HEADERS += $${PROJECT_DIRECTORY}/src/settings/settings_manager_core.h
HEADERS += $${PROJECT_DIRECTORY}/src/settings/settings_manager.h
HEADERS += $${PROJECT_DIRECTORY}/src/log/styled_log_view_core.h
HEADERS += $${PROJECT_DIRECTORY}/src/log/styled_log_view_structures.h
HEADERS += $${PROJECT_DIRECTORY}/src/log/vs_editor_log_definitions.h
HEADERS += $${PROJECT_DIRECTORY}/src/vapoursynth/vs_script_library.h
HEADERS += $${PROJECT_DIRECTORY}/src/vapoursynth/vs_script_processor_structures.h
HEADERS += $${PROJECT_DIRECTORY}/src/vapoursynth/vapoursynth_script_processor.h
HEADERS += $${PROJECT_DIRECTORY}/src/libp2p/p2p.h
HEADERS += $${PROJECT_DIRECTORY}/src/libp2p/p2p_api.h
HEADERS += $${PROJECT_DIRECTORY}/src/libp2p/simd/cpuinfo_x86.h
HEADERS += $${PROJECT_DIRECTORY}/src/libp2p/simd/p2p_simd.h
HEADERS += $${PROJECT_DIRECTORY}/src/vapoursynth/vs_pack_rgb.h
HEADERS += $${PROJECT_DIRECTORY}/src/vapoursynth/vs_set_matrix.h

HEADERS += $${PROJECT_DIRECTORY}/src/settings/actions_hotkey_edit_model.h
HEADERS += $${PROJECT_DIRECTORY}/src/settings/clearable_key_sequence_editor.h
HEADERS += $${PROJECT_DIRECTORY}/src/settings/item_delegate_for_hotkey.h
HEADERS += $${PROJECT_DIRECTORY}/src/settings/theme_elements_model.h
HEADERS += $${PROJECT_DIRECTORY}/src/settings/settings_dialog.h
HEADERS += $${PROJECT_DIRECTORY}/src/script_status_bar_widget/script_status_bar_widget.h
HEADERS += $${PROJECT_DIRECTORY}/src/timeline_slider/timeline_slider.h
HEADERS += $${PROJECT_DIRECTORY}/src/preview/scroll_navigator.h
HEADERS += $${PROJECT_DIRECTORY}/src/preview/preview_area.h
HEADERS += $${PROJECT_DIRECTORY}/src/preview/preview_advanced_settings_dialog.h
HEADERS += $${PROJECT_DIRECTORY}/src/preview/preview_dialog.h
HEADERS += $${PROJECT_DIRECTORY}/src/preview/zoom_ratio_spinbox.h
HEADERS += $${PROJECT_DIRECTORY}/src/vapoursynth/vs_script_processor_dialog.h

SOURCES += $${PROJECT_DIRECTORY}/src/helpers.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/settings/settings_definitions_core.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/settings/settings_definitions.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/settings/settings_manager_core.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/settings/settings_manager.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/log/styled_log_view_core.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/log/styled_log_view_structures.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/log/vs_editor_log_definitions.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/vapoursynth/vs_script_library.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/vapoursynth/vs_script_processor_structures.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/vapoursynth/vapoursynth_script_processor.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/timeline_slider/timeline_slider.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/vapoursynth/vs_pack_rgb.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/vapoursynth/vs_set_matrix.cpp

SOURCES += $${PROJECT_DIRECTORY}/src/settings/actions_hotkey_edit_model.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/settings/clearable_key_sequence_editor.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/settings/item_delegate_for_hotkey.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/settings/theme_elements_model.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/settings/settings_dialog.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/script_status_bar_widget/script_status_bar_widget.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/preview/scroll_navigator.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/preview/preview_area.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/preview/preview_advanced_settings_dialog.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/preview/preview_dialog.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/preview/zoom_ratio_spinbox.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/vapoursynth/vs_script_processor_dialog.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/vsedit_previewer_main.cpp

# libp2p
SOURCES_P2P += $${PROJECT_DIRECTORY}/src/libp2p/p2p_api.cpp
SOURCES_P2P += $${PROJECT_DIRECTORY}/src/libp2p/v210.cpp
SOURCES_P2P += $${PROJECT_DIRECTORY}/src/libp2p/simd/cpuinfo_x86.cpp
SOURCES_P2P += $${PROJECT_DIRECTORY}/src/libp2p/simd/p2p_simd.cpp
SOURCES_P2P += $${PROJECT_DIRECTORY}/src/libp2p/simd/p2p_sse41.cpp

p2p.name = p2p
p2p.input = SOURCES_P2P
p2p.dependency_type = TYPE_C
p2p.variable_out = OBJECTS
p2p.output = ${QMAKE_VAR_OBJECTS_DIR}${QMAKE_FILE_IN_BASE}$${first(QMAKE_EXT_OBJ)}
p2p.commands = $${QMAKE_CXX} $(CXXFLAGS) -DP2P_SIMD $(INCPATH) -c ${QMAKE_FILE_IN}
contains(QMAKE_COMPILER, msvc) {
	p2p.commands += -Fo${QMAKE_FILE_OUT}
} else {
	p2p.commands += -o ${QMAKE_FILE_OUT}
	p2p.commands += -std=c++14
	p2p.commands += -Wno-missing-field-initializers
	p2p.commands += -msse4.1
}
macx {
	p2p.commands += -Wno-gnu
}
QMAKE_EXTRA_COMPILERS += p2p
