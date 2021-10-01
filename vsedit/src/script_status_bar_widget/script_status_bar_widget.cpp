#include "script_status_bar_widget.h"

#include "../../../common-src/helpers.h"

#include <QLocale>

//==============================================================================

ScriptStatusBarWidget::ScriptStatusBarWidget(QWidget * a_pParent) :
	  QWidget(a_pParent)
	, m_readyPixmap(":tick.png")
	, m_busyPixmap(":busy.png")
{
	m_ui.setupUi(this);
	m_ui.colorPickerWidget->setVisible(false);
	m_ui.colorPickerIconLabel->setPixmap(QPixmap(":color_picker.png"));
	m_ui.colorPickerLabel->clear();
	m_ui.scriptProcessorQueueLabel->clear();
	m_ui.videoInfoLabel->clear();

	m_ui.scriptProcessorQueueIconLabel->setPixmap(m_readyPixmap);
	setQueueState(0, 0, 0);
}

// END OF ScriptStatusBarWidget::ScriptStatusBarWidget(QWidget * a_pParent)
//==============================================================================

ScriptStatusBarWidget::~ScriptStatusBarWidget()
{
}

// END OF ScriptStatusBarWidget::~ScriptStatusBarWidget()
//==============================================================================

bool ScriptStatusBarWidget::colorPickerVisible() const
{
	return m_ui.colorPickerWidget->isVisible();
}

// END OF bool ScriptStatusBarWidget::colorPickerVisible() const
//==============================================================================

void ScriptStatusBarWidget::setColorPickerVisible(bool a_visible)
{
	m_ui.colorPickerWidget->setVisible(a_visible);
}

// END OF void ScriptStatusBarWidget::setColorPickerVisible(bool a_visible)
//==============================================================================

void ScriptStatusBarWidget::setColorPickerString(const QString & a_string)
{
	m_ui.colorPickerLabel->setText(a_string);
}

// END OF void ScriptStatusBarWidget::setColorPickerString(
//		const QString & a_string)
//==============================================================================

void ScriptStatusBarWidget::setQueueState(size_t a_inQueue, size_t a_inProcess,
	size_t a_maxThreads)
{
	if((a_inProcess + a_inQueue) > 0)
		m_ui.scriptProcessorQueueIconLabel->setPixmap(m_busyPixmap);
	else
		m_ui.scriptProcessorQueueIconLabel->setPixmap(m_readyPixmap);

	m_ui.scriptProcessorQueueLabel->setText(
		tr("Script processor queue: %1:%2(%3)")
		.arg(a_inQueue).arg(a_inProcess).arg(a_maxThreads));
}

// END OF void ScriptStatusBarWidget::setQueueState(size_t a_inQueue,
//		size_t a_inProcess, size_t a_maxThreads)
//==============================================================================

void ScriptStatusBarWidget::setVideoInfo(const VSVideoInfo * a_cpVideoInfo)
{
	QString infoString = vsedit::videoInfoString(a_cpVideoInfo);
	m_ui.videoInfoLabel->setText(infoString);
}

// END OF void ScriptStatusBarWidget::setVideoInfo(
//		const VSVideoInfo * a_cpVideoInfo)
//==============================================================================
