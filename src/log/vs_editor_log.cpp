#include "vs_editor_log.h"

#include <vapoursynth/VapourSynth.h>

#include <map>

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

VSEditorLog::VSEditorLog(QWidget * a_pParent) :
	  StyledLogView(a_pParent)
{
	initializeStyles();
}

// END OF VSEditorLog::VSEditorLog(QWidget * a_pParent)
//==============================================================================

VSEditorLog::~VSEditorLog()
{
}

// END OF VSEditorLog::~VSEditorLog()
//==============================================================================

void VSEditorLog::initializeStyles()
{
	TextBlockStyle stylesToCreate[] = {
		{LOG_STYLE_ERROR, QColor("#ffeeee"), Qt::darkRed},
		{LOG_STYLE_DEBUG, palette().color(QPalette::Active, QPalette::Base),
			palette().color(QPalette::Active, QPalette::Dark)},
		{LOG_STYLE_WARNING, QColor("#eeeeff"), Qt::darkBlue},
		{LOG_STYLE_POSITIVE, QColor("#eeffee"), Qt::darkGreen},

		// Aliases
		{LOG_STYLE_VS_DEBUG, LOG_STYLE_DEBUG},
		{LOG_STYLE_VS_WARNING, LOG_STYLE_WARNING},
		{LOG_STYLE_VS_CRITICAL, LOG_STYLE_ERROR},
		{LOG_STYLE_VS_FATAL, LOG_STYLE_ERROR},
		{LOG_STYLE_QT_DEBUG, LOG_STYLE_DEBUG},
		{LOG_STYLE_QT_INFO, LOG_STYLE_DEFAULT},
		{LOG_STYLE_QT_WARNING, LOG_STYLE_WARNING},
		{LOG_STYLE_QT_CRITICAL, LOG_STYLE_ERROR},
		{LOG_STYLE_QT_FATAL, LOG_STYLE_ERROR},
	};

	for(TextBlockStyle & styleToCreate : stylesToCreate)
		addStyle(styleToCreate);
}

// END OF void VSEditorLog::initializeStyles()
//==============================================================================
