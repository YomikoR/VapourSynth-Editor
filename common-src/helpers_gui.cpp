#include "helpers_gui.h"

#include "helpers.h"

//==============================================================================

QColor vsedit::highlight(const QColor & a_color, int a_strength)
{
	int h = 0;
	int s = 0;
	int l = 0;
	a_color.getHsl(&h, &s, &l);

	if(l > 127)
		l -= a_strength;
	else
		l += a_strength;
	vsedit::clamp(l, 0, 255);

	return QColor::fromHsl(h, s, l);
}

// END OF QColor vsedit::highlight(const QColor & a_color, int a_strength)
//==============================================================================
