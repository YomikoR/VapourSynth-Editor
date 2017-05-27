#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <ui_main_window.h>

#include <vector>

class SettingsManager;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:

	MainWindow();

	virtual ~MainWindow();

public slots:

	void slotWriteLogMessage(int a_messageType, const QString & a_message);
	void slotWriteLogMessage(const QString & a_message,
		const QString & a_style = LOG_STYLE_DEFAULT);

protected:

	void moveEvent(QMoveEvent * a_pEvent) override;

	void resizeEvent(QResizeEvent * a_pEvent) override;

	void changeEvent(QEvent * a_pEvent) override;

private slots:

private:

	void createActionsAndMenus();

	Ui::MainWindow m_ui;

	SettingsManager * m_pSettingsManager;
};

#endif // MAINWINDOW_H
