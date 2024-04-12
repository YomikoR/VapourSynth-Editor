#ifndef TIMELINESLIDER_H
#define TIMELINESLIDER_H

#include <QWidget>
#include <set>

class QKeyEvent;
class QMouseEvent;
class QPaintEvent;
class QWheelEvent;

class TimeLineSlider : public QWidget
{
	Q_OBJECT

public:

	TimeLineSlider(QWidget * a_pParent = nullptr);

	virtual ~TimeLineSlider();

	enum DisplayMode
	{
		Time,
		Frames,
	};

	enum ColorRole
	{
		SlideLine,
		ActiveFrame,
		InactiveFrame,
		CurrentFramePointer,
		SlidingPointer,
		Bookmark,
	};

	int frame() const;

	void setFrame(int a_frame, bool a_refreshCache);

	void setFramesNumber(int a_framesNumber, bool a_refreshCache);

	void setFPS(double a_fps);

	DisplayMode displayMode() const;

	void setDisplayMode(DisplayMode a_displayMode);

	void setBigStep(int a_bigStep);

	void setLabelsFont(const QFont & a_font);
	void setColor(ColorRole a_role, const QColor & a_color);

	void addBookmark(int a_bookmark);
	void removeBookmark(int a_bookmark);
	std::set<int> bookmarks() const;
	void setBookmarks(const std::set<int> & a_bookmarks);
	void clearBookmarks();
	int getClosestBookmark(int a_frame) const;

public slots:

	void slotStepUp();
	void slotStepDown();
	void slotBigStepUp();
	void slotBigStepDown();
	void slotStepBy(int a_step);
	void slotStepBySeconds(double a_seconds);

	void slotBookmarkCurrentFrame();
	void slotUnbookmarkCurrentFrame();
	void slotGoToPreviousBookmark();
	void slotGoToNextBookmark();

signals:

	void signalSliderMoved(int a_frame);

	void signalFrameChanged(int a_frame, bool a_refreshCache);

	void signalSliderPressed();
	void signalSliderReleased();

protected:

	void keyPressEvent(QKeyEvent * a_pEvent);

	void mouseMoveEvent(QMouseEvent * a_pEvent);

	void mousePressEvent(QMouseEvent * a_pEvent);

	void mouseReleaseEvent(QMouseEvent * a_pEvent);

	void paintEvent(QPaintEvent * a_pEvent);

	void wheelEvent(QWheelEvent * a_pEvent);

private:

	int slideLineInnerWidth() const;

	int frameToPos(int a_frame) const;

	int posToFrame(int a_pos) const;

	QRect slideLineRect() const;

	QRect slideLineActiveRect() const;

	void recalculateMinimumSize();

	void setPointerAtFrame(const QMouseEvent * a_pEvent);

	int m_maxFrame;
	double m_fps;

	int m_currentFrame;
	int m_pointerAtFrame;

	DisplayMode m_displayMode;

	int m_bigStep;

	int m_sideMargin;
	int m_bottomMargin;
	int m_slideLineHeight;
	int m_slideLineFrameWidth;
	int m_slideLineTicksSpacing;
	int m_shortTickHeight;
	int m_mediumTickHeight;
	int m_longTickHeight;
	int m_tickTextSpacing;
	int m_textHeight;
	int m_topMargin;
	int m_minimumTicksSpacing;

	bool m_sliderPressed;

	QFont m_labelsFont;
	QColor m_slideLineColor;
	QColor m_activeFrameColor;
	QColor m_inactiveFrameColor;
	QColor m_currentFramePointerColor;
	QColor m_slidingPointerColor;
	QColor m_bookmarkColor;

	std::map<ColorRole, QColor *> m_colorRoleMap;

	std::set<int> m_bookmarks;
};

#endif // TIMELINESLIDER_H
