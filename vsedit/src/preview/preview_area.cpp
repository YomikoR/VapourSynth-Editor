#include "preview_area.h"

#include "scroll_navigator.h"

#include <QLabel>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QEnterEvent>
#include <QScrollBar>
#include <QCoreApplication>

//==============================================================================

PreviewArea::PreviewArea(QWidget * a_pParent) : QScrollArea(a_pParent)
	, m_pPreviewLabel(nullptr)
	, m_pScrollNavigator(nullptr)
	, m_draggingPreview(false)
	, m_lastCursorPos(0, 0)
	, m_lastPreviewLabelPos(0, 0)
	, m_lastScenePos(0.0, 0.0)
{
	m_pPreviewLabel = new QLabel(this);
	m_pPreviewLabel->setPixmap(QPixmap());
	m_pPreviewLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	m_pPreviewLabel->move(0, 0);
	QScrollArea::setWidget(m_pPreviewLabel);
	setWidgetResizable(true);

	m_pScrollNavigator = new ScrollNavigator(this);
	int scrollFrameWidth = frameWidth();
	m_pScrollNavigator->move(pos() +
		QPoint(scrollFrameWidth, scrollFrameWidth));
	m_pScrollNavigator->setVisible(false);

	setAttribute(Qt::WA_Hover, true);
	setMouseTracking(true);
	m_pPreviewLabel->setMouseTracking(true);
}

// END OF PreviewArea::PreviewArea(QWidget * a_pParent)
//==============================================================================

PreviewArea::~PreviewArea()
{

}

// END OF PreviewArea::~PreviewArea()
//==============================================================================

void PreviewArea::setPixmap(const QPixmap & a_pixmap)
{
	m_pPreviewLabel->setPixmap(a_pixmap);
	m_pixmapWidth = a_pixmap.width();
	m_pixmapHeight = a_pixmap.height();
}

// END OF void PreviewArea::setPixmap(const QPixmap & a_pixmap)
//==============================================================================

void PreviewArea::checkMouseOverPreview(const QPointF & a_pixelPos)
{
	if(!m_pPreviewLabel->underMouse())
		return;

	double pX = a_pixelPos.x();
	double pY = a_pixelPos.y();
	if(pX < 0 || pY < 0 ||
		pX >= pixmapWidth() || pY >= pixmapHeight())
		return;

	emit signalMouseOverPoint(pX, pY);
}

// END OF void PreviewArea::checkMouseOverPreview(
//		const QPoint & a_globalMousePos)
//==============================================================================

void PreviewArea::slotScrollLeft()
{
	horizontalScrollBar()->setValue(0);
}

// END OF void PreviewArea::slotScrollLeft()
//==============================================================================

void PreviewArea::slotScrollRight()
{
	QCoreApplication::processEvents();
	QScrollBar * pHorizontalScrollbar = horizontalScrollBar();
	pHorizontalScrollbar->setValue(pHorizontalScrollbar->maximum());
}

// END OF void PreviewArea::slotScrollRight()
//==============================================================================

void PreviewArea::slotScrollTop()
{
	verticalScrollBar()->setValue(0);
}

// END OF void PreviewArea::slotScrollTop()
//==============================================================================

void PreviewArea::slotScrollBottom()
{
	QCoreApplication::processEvents();
	QScrollBar * pVerticalScrollbar = verticalScrollBar();
	pVerticalScrollbar->setValue(pVerticalScrollbar->maximum());
}

// END OF void PreviewArea::slotScrollBottom()
//==============================================================================

void PreviewArea::resizeEvent(QResizeEvent * a_pEvent)
{
	QScrollArea::resizeEvent(a_pEvent);
	emit signalSizeChanged();
}

// END OF void PreviewArea::resizeEvent(QResizeEvent * a_pEvent)
//==============================================================================

void PreviewArea::keyPressEvent(QKeyEvent * a_pEvent)
{
	if(a_pEvent->modifiers() != Qt::NoModifier)
	{
		QScrollArea::keyPressEvent(a_pEvent);
		return;
	}

	int key = a_pEvent->key();
	int wantedKeys[] = {Qt::Key_Left, Qt::Key_Right, Qt::Key_Up, Qt::Key_Down,
		Qt::Key_PageUp, Qt::Key_PageDown, Qt::Key_Home, Qt::Key_End};
	int * pKeysEnd = wantedKeys + sizeof(wantedKeys) / sizeof(*wantedKeys);
	if(pKeysEnd != std::find(wantedKeys, pKeysEnd, key))
	{
		a_pEvent->ignore();
		return;
	}

	QScrollArea::keyPressEvent(a_pEvent);
}

// END OF void PreviewArea::keyPressEvent(QKeyEvent * a_pEvent)
//==============================================================================

void PreviewArea::wheelEvent(QWheelEvent * a_pEvent)
{
	if(a_pEvent->modifiers() == Qt::ControlModifier)
	{
		emit signalCtrlWheel(a_pEvent->angleDelta());
		a_pEvent->ignore();
		return;
	}

	QScrollArea::wheelEvent(a_pEvent);
}

// END OF void PreviewArea::wheelEvent(QWheelEvent * a_pEvent)
//==============================================================================

void PreviewArea::mousePressEvent(QMouseEvent * a_pEvent)
{
	if(a_pEvent->buttons() == Qt::LeftButton)
	{
		m_draggingPreview = true;
		m_lastCursorPos = a_pEvent->globalPosition().toPoint();
		m_lastPreviewLabelPos = m_pPreviewLabel->pos();
		m_pScrollNavigator->setVisible(true);
		drawScrollNavigator();
		a_pEvent->accept();
		return;
	}

	QScrollArea::mousePressEvent(a_pEvent);
}

// END OF void PreviewArea::mousePressEvent(QMouseEvent * a_pEvent)
//==============================================================================

void PreviewArea::mouseMoveEvent(QMouseEvent * a_pEvent)
{
	if((a_pEvent->buttons() & Qt::LeftButton) && m_draggingPreview)
	{
		QPoint newCursorPos = a_pEvent->globalPosition().toPoint();
		QPoint posDifference = newCursorPos - m_lastCursorPos;
		QPoint newPreviewLabelPos = m_lastPreviewLabelPos +
			posDifference;

		horizontalScrollBar()->setValue(-newPreviewLabelPos.x());
		verticalScrollBar()->setValue(-newPreviewLabelPos.y());

		drawScrollNavigator();
		a_pEvent->accept();
		return;
	}
	m_lastScenePos = a_pEvent->scenePosition();
	checkMouseOverPreview(pixelPosition());

	QScrollArea::mouseMoveEvent(a_pEvent);
}

// END OF void PreviewArea::mouseMoveEvent(QMouseEvent * a_pEvent)
//==============================================================================

void PreviewArea::mouseReleaseEvent(QMouseEvent * a_pEvent)
{
	Qt::MouseButton releasedButton = a_pEvent->button();
	if(releasedButton == Qt::LeftButton)
	{
		m_draggingPreview = false;
		m_pScrollNavigator->setVisible(false);
		a_pEvent->accept();
		return;
	}
	else if(releasedButton == Qt::MiddleButton)
		emit signalMouseMiddleButtonReleased();
	else if(releasedButton == Qt::RightButton)
		emit signalMouseRightButtonReleased();

	QScrollArea::mouseReleaseEvent(a_pEvent);
}

void PreviewArea::enterEvent(QEnterEvent *a_pEvent)
{
	m_lastScenePos = a_pEvent->scenePosition();
	checkMouseOverPreview(pixelPosition());
	QScrollArea::enterEvent(a_pEvent);
	setAttribute(Qt::WA_Hover, false);
}

// END OF void PreviewArea::mouseReleaseEvent(QMouseEvent * a_pEvent)
//==============================================================================

void PreviewArea::drawScrollNavigator()
{
	int contentsWidth = this->pixmapWidth();
	int contentsHeight = this->pixmapHeight();
	int viewportX = -m_pPreviewLabel->x();
	int viewportY = -m_pPreviewLabel->y();
	int viewportWidth = viewport()->width();
	int viewportHeight = viewport()->height();

	m_pScrollNavigator->draw(contentsWidth, contentsHeight, viewportX,
		viewportY, viewportWidth, viewportHeight);
}

// END OF void PreviewArea::drawScrollNavigator()
//==============================================================================

QPointF PreviewArea::pixelPosition() const
{
	QPoint lPos = m_pPreviewLabel->geometry().topLeft();
	QMargins lMargin = contentsMargins();
	QPoint mOffset = QPoint(lMargin.left(), lMargin.top());
	return m_lastScenePos - lPos - mOffset;
}
