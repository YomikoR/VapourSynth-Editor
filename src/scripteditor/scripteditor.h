#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include <QPlainTextEdit>
#include <QPoint>
#include <vector>

#include "../vapoursynth/vsplugindata.h"

class QEvent;
class QKeyEvent;
class QResizeEvent;
class QPaintEvent;
class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;
class QAction;
class ScriptCompleterModel;
class ScriptCompleter;
class SyntaxHighlighter;
class SettingsManager;
class SettingsDialog;

class ScriptEditor : public QPlainTextEdit
{
	Q_OBJECT

public:

	ScriptEditor(QWidget * a_pParent = nullptr);

	virtual ~ScriptEditor();

	QString text() const;

	QPoint cursorPosition() const;

	void setCursorPosition(const QPoint & a_point);

	void setCursorPosition(int a_line, int a_index);

	bool isModified() const;

	void setModified(bool a_modified);

	void setPluginsList(const VSPluginsList & a_pluginsList);

	void setSettingsManager(SettingsManager * a_pSettingsManager);

	std::vector<QAction *> actionsForMenu() const;

public slots:

	void slotLoadSettings();

	void slotComplete();

	void slotInsertCompletion(const QString & a_completionString);

	void slotDuplicateSelection();

	void slotCommentSelection();

	void slotUncommentSelection();

	void slotReplaceTabWithSpaces();

	void slotTab();

	void slotBackTab();

	void slotHome(bool a_select = false);

	void slotInsertTextAtNewLine(const QString & a_text);

signals:

	void signalScriptFileDropped(const QString & a_filePath, bool * a_pHandled);

protected:

	virtual bool eventFilter(QObject * a_pObject, QEvent * a_pEvent) override;

	virtual void resizeEvent(QResizeEvent * a_pEvent) override;

	virtual void keyPressEvent(QKeyEvent * a_pEvent) override;

	virtual void dragEnterEvent(QDragEnterEvent * a_pEvent) override;

	virtual void dragMoveEvent(QDragMoveEvent * a_pEvent) override;

	virtual void dropEvent(QDropEvent * a_pEvent) override;

private slots:

	void slotTextChanged();

	void slotUpdateSideBoxWidth();

	void slotUpdateSideBox(const QRect & a_rect, int a_dy);

	void slotHighlightCurrentBlock();

	void slotShowCustomMenu(const QPoint & a_position);

private:

	void createActionsAndMenus();

	QString getVapourSynthCoreName() const;

	void setChildrenCoreName(const QString & a_coreName);

	int sideBoxWidth() const;

	void paintSideBox(QPaintEvent * a_pEvent);

	void indentNewLine();

	void insertSelectedLinesBegin(const QString & a_text);
	void removeSelectedLinesBegin(const QString & a_text);

	SettingsManager * m_pSettingsManager;

	QWidget * m_pSideBox;

	int m_sideBoxLineWidth;

	int m_sideBoxTextMargin;

	ScriptCompleterModel * m_pCompleterModel;

	ScriptCompleter * m_pCompleter;

	SyntaxHighlighter * m_pSyntaxHighlighter;

	int m_typedCharacters;

	int m_charactersTypedToStartCompletion;

	QString m_plainText;

	QColor m_backgroundColor;

	QColor m_activeLineColor;

	QTextCharFormat m_commonScriptTextFormat;

	QString m_tabText;

	int m_spacesInTab;

	QAction * m_pActionDuplicateSelection;
	QAction * m_pActionCommentSelection;
	QAction * m_pActionUncommentSelection;
	QAction * m_pActionReplaceTabWithSpaces;
	QAction * m_pActionAutocomplete;

	std::vector<QAction *> m_settableActionsList;

	QMenu * m_pContextMenu;
};

#endif // SCRIPTEDITOR_H
