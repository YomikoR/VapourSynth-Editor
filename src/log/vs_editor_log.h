#ifndef VS_EDITOR_LOG_H_INCLUDED
#define VS_EDITOR_LOG_H_INCLUDED

#include "styled_log_view.h"

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

protected:

	virtual void initializeStyles();
};

//==============================================================================

#endif // VS_EDITOR_LOG_H_INCLUDED
