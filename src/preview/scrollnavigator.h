#ifndef SCROLLNAVIGATOR_H
#define SCROLLNAVIGATOR_H

#include <QWidget>

class QPaintEvent;

class ScrollNavigator : public QWidget
{
	public:

		ScrollNavigator(QWidget * a_pParent = 0);

		virtual ~ScrollNavigator();

		void draw(int a_contentsWidth, int a_contentsHeight, int a_viewportX,
			int a_viewportY, int a_viewportWidth, int a_viewportHeight);

	protected:

		void paintEvent(QPaintEvent * a_pPaintEvent) override;

	private:

		int m_contentsHeight;
		int m_contentsWidth;
		int m_viewportX;
		int m_viewportY;
		int m_viewportHeight;
		int m_viewportWidth;

};

#endif // SCROLLNAVIGATOR_H
