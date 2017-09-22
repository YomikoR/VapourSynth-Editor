#ifndef VS_EDITOR_LOG_H_INCLUDED
#define VS_EDITOR_LOG_H_INCLUDED

#include "styled_log_view.h"
#include "vs_editor_log_definitions.h"

class SettingsManager;

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

#endif // VS_EDITOR_LOG_H_INCLUDED
