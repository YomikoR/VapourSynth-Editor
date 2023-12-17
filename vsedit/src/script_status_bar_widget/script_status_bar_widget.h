#ifndef SCRIPT_STATUS_BAR_WIDGET_H_INCLUDED
#define SCRIPT_STATUS_BAR_WIDGET_H_INCLUDED

#include <ui_script_status_bar_widget.h>

#include <vapoursynth/VapourSynth4.h>
#include <QPixmap>

class VSNodeInfo;

class ScriptStatusBarWidget: public QWidget
{
	Q_OBJECT

public:

	ScriptStatusBarWidget(QWidget * a_pParent = nullptr);
	virtual ~ScriptStatusBarWidget();

	virtual bool colorPickerVisible() const;

public slots:

	virtual void setColorPickerVisible(bool a_visible = true);

	virtual void setColorPickerString(const QString & a_string);

	virtual void setQueueState(size_t a_inQueue, size_t a_inProcess,
		size_t a_maxThreads, double a_usedCacheRatio);

	virtual void setNodeInfo(const VSNodeInfo & a_nodeInfo,
		const VSAPI * a_cpVSAPI);

protected:

	Ui::ScriptStatusBarWidget m_ui;

	QPixmap m_readyPixmap;
	QPixmap m_busyPixmap;
};

#endif // SCRIPT_STATUS_BAR_WIDGET_H_INCLUDED
