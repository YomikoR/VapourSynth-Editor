#include "vs_editor_log_definitions.h"

#include "styled_log_view_core.h"

#include <vapoursynth/VapourSynth4.h>
#include <map>
#include <algorithm>

//==============================================================================

const char LOG_STYLE_ERROR[] = "error";
const char LOG_STYLE_DEBUG[] = "debug";
const char LOG_STYLE_WARNING[] = "warning";
const char LOG_STYLE_POSITIVE[] = "positive";

const char LOG_STYLE_VS_DEBUG[] = "vs_debug";
const char LOG_STYLE_VS_WARNING[] = "vs_warning";
const char LOG_STYLE_VS_CRITICAL[] = "vs_critical";
const char LOG_STYLE_VS_FATAL[] = "vs_fatal";

const char LOG_STYLE_QT_DEBUG[] = "qt_debug";
const char LOG_STYLE_QT_INFO[] = "qt_info";
const char LOG_STYLE_QT_WARNING[] = "qt_warning";
const char LOG_STYLE_QT_CRITICAL[] = "qt_critical";
const char LOG_STYLE_QT_FATAL[] = "qt_fatal";

//==============================================================================

QString vsMessageTypeToStyleName(int a_messageType)
{
	QString style(LOG_STYLE_DEFAULT);

	std::map<int, QString> vsTypeToStyleMap = {
		{mtDebug, LOG_STYLE_VS_DEBUG},
		{mtWarning, LOG_STYLE_VS_WARNING},
		{mtCritical, LOG_STYLE_VS_CRITICAL},
		{mtFatal, LOG_STYLE_VS_FATAL},
	};

	std::map<int, QString>::const_iterator it =
		vsTypeToStyleMap.find(a_messageType);
	if(it != vsTypeToStyleMap.end())
		return it->second;

	return style;
}

// END OF QString vsMessageTypeToStyleName(int a_messageType)
//==============================================================================
