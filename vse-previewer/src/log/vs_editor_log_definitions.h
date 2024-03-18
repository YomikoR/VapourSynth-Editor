#ifndef VS_EDITOR_LOG_DEFINITIONS_H_INCLUDED
#define VS_EDITOR_LOG_DEFINITIONS_H_INCLUDED

#include <QString>

extern const char LOG_STYLE_ERROR[];
extern const char LOG_STYLE_DEBUG[];
extern const char LOG_STYLE_WARNING[];
extern const char LOG_STYLE_POSITIVE[];

extern const char LOG_STYLE_VS_DEBUG[];
extern const char LOG_STYLE_VS_INFO[];
extern const char LOG_STYLE_VS_WARNING[];
extern const char LOG_STYLE_VS_CRITICAL[];
extern const char LOG_STYLE_VS_FATAL[];

extern const char LOG_STYLE_QT_DEBUG[];
extern const char LOG_STYLE_QT_INFO[];
extern const char LOG_STYLE_QT_WARNING[];
extern const char LOG_STYLE_QT_CRITICAL[];
extern const char LOG_STYLE_QT_FATAL[];

QString vsMessageTypeToStyleName(int a_messageType);

#endif // VS_EDITOR_LOG_DEFINITIONS_H_INCLUDED
