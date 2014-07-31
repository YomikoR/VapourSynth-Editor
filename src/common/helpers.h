#ifndef HELPERS_H_INCLUDED
#define HELPERS_H_INCLUDED

#include <QString>
#include <QTime>
#include <vapoursynth/VapourSynth.h>

namespace vsedit
{
	QString timeToString(double a_seconds, bool a_fullFormat = false);

	int mod(int a_value);

	QString videoInfoString(const VSVideoInfo * a_cpVideoInfo);

	double qtimeToSeconds(const QTime & a_qtime);

	QTime secondsToQTime(double a_seconds);
}

#endif // HELPERS_H_INCLUDED
