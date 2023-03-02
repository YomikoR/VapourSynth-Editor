#ifndef PREVIEWAREA_H
#define PREVIEWAREA_H

#include <QScrollArea>
#include <QPixmap>
#include <QPoint>

class QLabel;
class ScrollNavigator;
class QKeyEvent;
class QWheelEvent;
class QMouseEvent;
class QEnterEvent;

class PreviewArea : public QScrollArea
{
	Q_OBJECT

public:

	PreviewArea(QWidget * a_pParent = nullptr);

	virtual ~PreviewArea();

	void setWidget(QWidget * a_pWidget) = delete;

	int pixmapWidth() const { return m_pixmapWidth; }

	int pixmapHeight() const { return m_pixmapHeight; }

	void setPixmap(const QPixmap & a_pixmap, bool a_isVideoFrame = false);

	void checkMouseOverPreview(const QPointF & a_pixelPos);

	QPointF pixelPosition() const;

	QPoint getScrollBarPositions() const;

	void getScrollBarPositionsFromPreviewer(const QPoint & pos);

public slots:

	void slotScrollLeft();
	void slotScrollRight();
	void slotScrollTop();
	void slotScrollBottom();
	void slotSetScrollBarPositions();


protected:

	void resizeEvent(QResizeEvent * a_pEvent) override;
	void keyPressEvent(QKeyEvent * a_pEvent) override;
	void wheelEvent(QWheelEvent * a_pEvent) override;
	void mousePressEvent(QMouseEvent * a_pEvent) override;
	void mouseMoveEvent(QMouseEvent * a_pEvent) override;
	void mouseReleaseEvent(QMouseEvent * a_pEvent) override;
	void enterEvent(QEnterEvent * a_pEvent) override;

signals:

	void signalSizeChanged();
	void signalCtrlWheel(QPoint a_angleDelta);
	void signalMouseMiddleButtonReleased();
	void signalMouseRightButtonReleased();
	void signalMouseOverPoint(double a_normX, double a_normY);

private:

	void drawScrollNavigator();

	QLabel * m_pPreviewLabel;

	ScrollNavigator * m_pScrollNavigator;

	bool m_draggingPreview;
	QPoint m_lastCursorPos;
	QPoint m_lastPreviewLabelPos;
	QPointF m_lastScenePos;

	int m_pixmapWidth = 0;
	int m_pixmapHeight = 0;

	bool m_newToPreviewer;
	QPoint m_lastScrollBarPos;
};

#endif // PREVIEWAREA_H
