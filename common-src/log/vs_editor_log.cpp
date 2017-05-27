#include "vs_editor_log.h"

#include "../settings/settings_manager.h"

#include <vapoursynth/VapourSynth.h>
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

VSEditorLog::VSEditorLog(QWidget * a_pParent) :
	  StyledLogView(a_pParent)
	, m_pSettingsManager(nullptr)
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

QString VSEditorLog::name() const
{
	return m_name;
}

// END OF QString VSEditorLog::name() const
//==============================================================================

void VSEditorLog::setName(const QString & a_name)
{
	m_name = a_name;
}

// END OF void VSEditorLog::setName(const QString & a_name)
//==============================================================================

void VSEditorLog::setSettingsManager(SettingsManager * a_pSettingsManager)
{
	m_pSettingsManager = a_pSettingsManager;
}

// END OF void VSEditorLog::setSettingsManager(
//		SettingsManager * a_pSettingsManager)
//==============================================================================

bool VSEditorLog::loadSettings()
{
	if(!m_pSettingsManager)
		return false;

	if(m_name.isEmpty())
		return false;

	const std::vector<TextBlockStyle> styles =
		m_pSettingsManager->getLogStyles(m_name);

	for(TextBlockStyle & style : m_styles)
	{
		std::vector<TextBlockStyle>::const_iterator it =
			std::find_if(styles.begin(), styles.end(),
				[&](const TextBlockStyle & a_style) -> bool
				{
					return (a_style.name == style.name);
				});

		if(it != styles.end())
			style = *it;
	}

	return true;
}

// END OF bool VSEditorLog::loadSettings()
//==============================================================================

bool VSEditorLog::saveSettings()
{
	if(!m_pSettingsManager)
		return false;

	if(m_name.isEmpty())
		return false;

	return m_pSettingsManager->setLogStyles(m_name, m_styles);
}

// END OF bool VSEditorLog::saveSettings()
//==============================================================================

void VSEditorLog::slotLogSettingsChanged()
{
	StyledLogView::slotLogSettingsChanged();
	saveSettings();
}

// END OF void VSEditorLog::slotLogSettingsChanged()
//==============================================================================

void VSEditorLog::initializeStyles()
{
	TextBlockStyle stylesToCreate[] = {
		{LOG_STYLE_ERROR, trUtf8("Error"), QColor("#ffeeee"), Qt::darkRed},
		{LOG_STYLE_DEBUG, trUtf8("Debug message"),
			palette().color(QPalette::Active, QPalette::Base),
			palette().color(QPalette::Active, QPalette::Dark)},
		{LOG_STYLE_WARNING, trUtf8("Warning"), QColor("#eeeeff"), Qt::darkBlue},
		{LOG_STYLE_POSITIVE, trUtf8("Positive message"), QColor("#eeffee"),
			Qt::darkGreen},

		// Aliases
		{LOG_STYLE_VS_DEBUG, trUtf8("VapourSynth debug message"),
			LOG_STYLE_DEBUG},
		{LOG_STYLE_VS_WARNING, trUtf8("VapourSynth warning"),
			LOG_STYLE_WARNING},
		{LOG_STYLE_VS_CRITICAL, trUtf8("VapourSynth error"), LOG_STYLE_ERROR},
		{LOG_STYLE_VS_FATAL, trUtf8("VapourSynth fatal error"),
			LOG_STYLE_ERROR},
		{LOG_STYLE_QT_DEBUG, trUtf8("Qt debug message"), LOG_STYLE_DEBUG},
		{LOG_STYLE_QT_INFO, trUtf8("Qt info"), LOG_STYLE_DEFAULT},
		{LOG_STYLE_QT_WARNING, trUtf8("Qt warning"), LOG_STYLE_WARNING},
		{LOG_STYLE_QT_CRITICAL, trUtf8("Qt critical error"), LOG_STYLE_ERROR},
		{LOG_STYLE_QT_FATAL, trUtf8("Qt fatal error"), LOG_STYLE_ERROR},
	};

	for(TextBlockStyle & styleToCreate : stylesToCreate)
		addStyle(styleToCreate);
}

// END OF void VSEditorLog::initializeStyles()
//==============================================================================
