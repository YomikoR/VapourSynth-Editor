#ifndef HELPERS_H_INCLUDED
#define HELPERS_H_INCLUDED

#include <QString>
#include <QTime>
#include <cassert>
#include <vapoursynth/VapourSynth.h>

namespace vsedit
{
	QString timeToString(double a_seconds, bool a_fullFormat = false);

	int mod(int a_value);

	QString videoInfoString(const VSVideoInfo * a_cpVideoInfo);

	double qtimeToSeconds(const QTime & a_qtime);

	QTime secondsToQTime(double a_seconds);

	template<typename T1, typename T2, typename T3>
		void clamp(T1& a_value, const T2& a_low, const T3& a_high)
	{
		assert(a_high > a_low);
		if(a_value < a_low)
			a_value = a_low;
		else if(a_value > a_high)
			a_value = a_high;
	}
}

#endif // HELPERS_H_INCLUDED
