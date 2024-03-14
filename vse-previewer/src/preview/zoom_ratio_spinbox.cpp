#include "zoom_ratio_spinbox.h"

#include <cmath>

ZoomRatioSpinBox::ZoomRatioSpinBox(QWidget * a_pWidget): 
	QDoubleSpinBox(a_pWidget)
	, m_scaleMode{Qt::SmoothTransformation}
	{}

void ZoomRatioSpinBox::setScaleMode(Qt::TransformationMode a_mode)
{
	m_scaleMode = a_mode;
	if(m_scaleMode == Qt::FastTransformation)
	{
		int ival = std::round(value());
		if(ival == 0)
			ival = 1;
		if(ival > maximum())
			ival = std::floor(maximum());
		QDoubleSpinBox::setValue(ival);
	}
}

void ZoomRatioSpinBox::stepBy(int steps)
{
	if(steps == -1)
	{
		if(m_scaleMode == Qt::FastTransformation)
		{
			int ival = (int)value() - 1;
			if(ival <= 0)
				ival = 1;
			QDoubleSpinBox::setValue(ival);
		}
		else
		{
			double val = value();
			if(val > 2.0)
				QDoubleSpinBox::setValue(val - 1.0);
			else if(val > 1.0)
				QDoubleSpinBox::setValue(1.0);
			else if(val > 0.01 + minimum())
				QDoubleSpinBox::setValue(val - 0.01);
			else
				QDoubleSpinBox::setValue(minimum());
		}
	}
	else if(steps == 1)
	{
		if(m_scaleMode == Qt::FastTransformation)
		{
			int ival = (int)value() + 1;
			if(ival > maximum())
				ival = std::floor(maximum());
			QDoubleSpinBox::setValue(ival);
		}
		else
		{
			double val = value();
			if(val < 0.99)
				QDoubleSpinBox::setValue(val + 0.01);
			else if(val < 1.0)
				QDoubleSpinBox::setValue(1.0);
			else if(val + 0.1 < 4.1)
				QDoubleSpinBox::setValue(val + 0.1);
			else if(val + 1.0 < maximum())
				QDoubleSpinBox::setValue(val + 1.0);
			else
				QDoubleSpinBox::setValue(maximum());
		}
	}
	else if(steps > 0)
	{
		stepBy(1);
		stepBy(steps - 1);
	}
	else if(steps < 0)
	{
		stepBy(-1);
		stepBy(steps + 1);
	}
}

void ZoomRatioSpinBox::setValue(double val)
{
	if(m_scaleMode == Qt::FastTransformation)
	{
		int ival = std::round(val);
		if(ival == 0)
			ival = 1;
		if(ival > maximum())
			ival = std::floor(maximum());
		QDoubleSpinBox::setValue(ival);
	}
	else
		QDoubleSpinBox::setValue(val);
}
