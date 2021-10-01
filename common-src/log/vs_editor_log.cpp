#include "vs_editor_log.h"

#include "../settings/settings_manager.h"

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
		{LOG_STYLE_ERROR, tr("Error"), QColor("#ffeeee"), Qt::darkRed},
		{LOG_STYLE_DEBUG, tr("Debug message"),
			palette().color(QPalette::Active, QPalette::Base),
			palette().color(QPalette::Active, QPalette::Dark)},
		{LOG_STYLE_WARNING, tr("Warning"), QColor("#eeeeff"), Qt::darkBlue},
		{LOG_STYLE_POSITIVE, tr("Positive message"), QColor("#eeffee"),
			Qt::darkGreen},

		// Aliases
		{LOG_STYLE_VS_DEBUG, tr("VapourSynth debug message"),
			LOG_STYLE_DEBUG},
		{LOG_STYLE_VS_WARNING, tr("VapourSynth warning"),
			LOG_STYLE_WARNING},
		{LOG_STYLE_VS_CRITICAL, tr("VapourSynth error"), LOG_STYLE_ERROR},
		{LOG_STYLE_VS_FATAL, tr("VapourSynth fatal error"),
			LOG_STYLE_ERROR},
		{LOG_STYLE_QT_DEBUG, tr("Qt debug message"), LOG_STYLE_DEBUG},
		{LOG_STYLE_QT_INFO, tr("Qt info"), LOG_STYLE_DEFAULT},
		{LOG_STYLE_QT_WARNING, tr("Qt warning"), LOG_STYLE_WARNING},
		{LOG_STYLE_QT_CRITICAL, tr("Qt critical error"), LOG_STYLE_ERROR},
		{LOG_STYLE_QT_FATAL, tr("Qt fatal error"), LOG_STYLE_ERROR},
	};

	for(TextBlockStyle & styleToCreate : stylesToCreate)
		addStyle(styleToCreate);
}

// END OF void VSEditorLog::initializeStyles()
//==============================================================================
