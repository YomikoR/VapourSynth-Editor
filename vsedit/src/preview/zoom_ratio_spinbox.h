#ifndef ZOOM_RATIO_SPINBOX_H_INCLUDED
#define ZOOM_RATIO_SPINBOX_H_INCLUDED

#include "../../../common-src/settings/settings_definitions.h"

#include <QDoubleSpinBox>

class ZoomRatioSpinBox : public QDoubleSpinBox
{
public:
    ZoomRatioSpinBox(QWidget * a_pParent = nullptr);

    void setScaleMode(Qt::TransformationMode a_mode);

    void stepBy(int steps);

public slots:
    void setValue(double val);

private:
    Qt::TransformationMode m_scaleMode;
};

#endif
