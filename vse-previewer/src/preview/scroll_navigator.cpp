#include "scroll_navigator.h"

#include <QPaintEvent>
#include <QPainter>
#include <QColor>

//==============================================================================

ScrollNavigator::ScrollNavigator(QWidget * a_pParent) : QWidget(a_pParent)
	, m_contentsHeight(0)
	, m_contentsWidth(0)
	, m_viewportX(0)
	, m_viewportY(0)
	, m_viewportHeight(0)
	, m_viewportWidth(0)
{
	setAttribute(Qt::WA_NoSystemBackground);
	setAttribute(Qt::WA_TransparentForMouseEvents);
	setGeometry(0, 0, 100, 100);
}

// END OF ScrollNavigator::ScrollNavigator(QWidget * a_pParent)
//==============================================================================

ScrollNavigator::~ScrollNavigator()
{

}

// END OF ScrollNavigator::~ScrollNavigator()
//==============================================================================

void ScrollNavigator::draw(int a_contentsWidth, int a_contentsHeight,
	int a_viewportX, int a_viewportY, int a_viewportWidth,
	int a_viewportHeight)
{
	m_contentsHeight = a_contentsHeight;
	m_contentsWidth = a_contentsWidth;
	m_viewportX = a_viewportX;
	m_viewportY = a_viewportY;
	m_viewportHeight = a_viewportHeight;
	m_viewportWidth = a_viewportWidth;
	repaint();
}

// END OF void ScrollNavigator::draw(int a_contentsWidth, int a_contentsHeight,
//		int a_viewportX, int a_viewportY, int a_viewportWidth,
//		int a_viewportHeight)
//==============================================================================

void ScrollNavigator::paintEvent(QPaintEvent * a_pPaintEvent)
{
	if((m_contentsWidth == 0) || (m_contentsHeight == 0) ||
		(m_viewportWidth == 0) || (m_viewportHeight == 0) ||
		(m_viewportX >= m_contentsWidth) || (m_viewportY >= m_contentsHeight))
	{
		a_pPaintEvent->ignore();
		return;
	}

	int measures[] = {m_contentsWidth, m_contentsHeight, m_viewportWidth,
		m_viewportHeight};
	int maxMeasure = 0;
	for(int m : measures)
		if(m > maxMeasure)
			maxMeasure = m;

	double dpr = window()->devicePixelRatioF();

	int normalizedContentsWidth = int((double)m_contentsWidth * 100.0 /
		(double)maxMeasure);
	int normalizedContentsHeight = int((double)m_contentsHeight * 100.0 /
		(double)maxMeasure);
	int normalizedVieportX = int(m_viewportX * dpr * 100.0 /
		(double)maxMeasure);
	int normalizedViwportY = int(m_viewportY * dpr * 100.0 /
		(double)maxMeasure);
	int normalizedViewportWidth = int(m_viewportWidth * dpr * 100.0 /
		(double)maxMeasure);
	int normalizedViewportHeight = int(m_viewportHeight * dpr * 100.0 /
		(double)maxMeasure);

	int cX1 = 0;
	int cY1 = 0;
	int cX2 = normalizedContentsWidth - 1;
	int cY2 = normalizedContentsHeight - 1;

	int vX1 = normalizedVieportX;
	int vY1 = normalizedViwportY;
	int vX2 = normalizedVieportX + normalizedViewportWidth - 1;
	int vY2 = normalizedViwportY + normalizedViewportHeight - 1;

	QPainter painter(this);

	// Draw contents rectangle.
	painter.setPen(QColor::fromRgb(255, 0, 255));
	painter.drawLine(cX1, cY1, cX2, cY1);
	painter.drawLine(cX2, cY1, cX2, cY2);
	painter.drawLine(cX2, cY2, cX1, cY2);
	painter.drawLine(cX1, cY2, cX1, cY1);

	// Draw viewport rectangle.
	painter.setPen(QColor::fromRgb(0, 255, 0));
	painter.drawLine(vX1, vY1, vX2, vY1);
	painter.drawLine(vX2, vY1, vX2, vY2);
	painter.drawLine(vX2, vY2, vX1, vY2);
	painter.drawLine(vX1, vY2, vX1, vY1);

    a_pPaintEvent->accept();
}

// END OF void ScrollNavigator::paintEvent(QPaintEvent * a_pPaintEvent)
//==============================================================================
