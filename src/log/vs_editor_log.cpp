#include "vs_editor_log.h"

#include <vapoursynth/VapourSynth.h>

//==============================================================================

const char LOG_STYLE_ERROR[] = "error";
const char LOG_STYLE_DEBUG[] = "debug";
const char LOG_STYLE_WARNING[] = "warning";
const char LOG_STYLE_POSITIVE[] = "positive";

//==============================================================================

QString vsMessageTypeToStyleName(int a_messageType)
{
	QString style(LOG_STYLE_DEFAULT);

	switch(a_messageType)
	{
	case mtDebug:
		style = LOG_STYLE_DEBUG;
		break;
	case mtWarning:
		style = LOG_STYLE_WARNING;
		break;
	case mtCritical:
	case mtFatal:
		style = LOG_STYLE_ERROR;
		break;
	default:
		style = LOG_STYLE_DEFAULT;
	}

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
	struct StyleToCreate
	{
		QString name;
		QColor textColor;
		QColor backgroundColor;
	};

	StyleToCreate stylesToCreate[] = {
		{LOG_STYLE_ERROR, Qt::darkRed, QColor("#ffeeee")},
		{LOG_STYLE_DEBUG, palette().color(QPalette::Active, QPalette::Dark),
			palette().color(QPalette::Active, QPalette::Base)},
		{LOG_STYLE_WARNING, Qt::darkBlue, QColor("#eeeeff")},
		{LOG_STYLE_POSITIVE, Qt::darkGreen, QColor("#eeffee")},
	};

	for(StyleToCreate & styleToCreate : stylesToCreate)
	{
		QTextCharFormat charFormat;
		charFormat.setForeground(styleToCreate.textColor);
		TextBlockStyle style = {styleToCreate.name,
			styleToCreate.backgroundColor, charFormat};
		addStyle(style);
	}
}

// END OF void VSEditorLog::initializeStyles()
//==============================================================================
