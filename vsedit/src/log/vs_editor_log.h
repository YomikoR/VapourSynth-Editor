#ifndef VS_EDITOR_LOG_H_INCLUDED
#define VS_EDITOR_LOG_H_INCLUDED

#include "styled_log_view.h"

//==============================================================================

class SettingsManager;

//==============================================================================

extern const char LOG_STYLE_ERROR[];
extern const char LOG_STYLE_DEBUG[];
extern const char LOG_STYLE_WARNING[];
extern const char LOG_STYLE_POSITIVE[];

extern const char LOG_STYLE_VS_DEBUG[];
extern const char LOG_STYLE_VS_WARNING[];
extern const char LOG_STYLE_VS_CRITICAL[];
extern const char LOG_STYLE_VS_FATAL[];

extern const char LOG_STYLE_QT_DEBUG[];
extern const char LOG_STYLE_QT_INFO[];
extern const char LOG_STYLE_QT_WARNING[];
extern const char LOG_STYLE_QT_CRITICAL[];
extern const char LOG_STYLE_QT_FATAL[];

//==============================================================================

QString vsMessageTypeToStyleName(int a_messageType);

//==============================================================================

class VSEditorLog : public StyledLogView
{
	Q_OBJECT

public:

	VSEditorLog(QWidget * a_pParent = nullptr);
	virtual ~VSEditorLog();

	virtual QString name() const;
	virtual void setName(const QString & a_name);

	virtual void setSettingsManager(SettingsManager * a_pSettingsManager);

	virtual bool loadSettings();

	virtual bool saveSettings();

protected slots:

	virtual void slotLogSettingsChanged() override;

protected:

	virtual void initializeStyles();

	QString m_name;

	SettingsManager * m_pSettingsManager;
};

//==============================================================================

#endif // VS_EDITOR_LOG_H_INCLUDED
