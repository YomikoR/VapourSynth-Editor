#include <cmath>

#include "helpers.h"

//==============================================================================

QString vsedit::timeToString(double a_seconds, bool a_fullFormat)
{
	if(a_seconds <= 0.0)
		return QString("0");

	// Milliseconds cut-off
	a_seconds = std::round(a_seconds * 1000.0) / 1000.0;

	// Seconds
	uint64_t integer = (uint64_t)a_seconds;
	int seconds = integer % 60ll;
	integer /= 60ll;
	int minutes = integer % 60ll;
	integer /= 60ll;
	int hours = integer % 60ll;

	QString timeString;

	if((hours > 0) || a_fullFormat)
	{
		timeString = QString("%1:%2:%3").arg(hours)
			.arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
	}
	else
	{
		timeString = QString("%1:%2") .arg(minutes)
			.arg(seconds, 2, 10, QChar('0'));
	}

	// Fraction
	double fraction = a_seconds - std::floor(a_seconds);
	if((fraction > 0.0) || a_fullFormat)
		timeString += QString::number(fraction, 'f', 3).mid(1);

	return timeString;
}

// END OF QString vsedit::timeToString(double a_seconds, bool a_fullFormat)
//==============================================================================

int vsedit::mod(int a_value)
{
	int l_mod = 1 << 6;
	while(a_value % l_mod != 0)
		l_mod >>= 1;
	return l_mod;
}

// END OF int vsedit::mod(int a_value)
//==============================================================================

QString vsedit::videoInfoString(const VSVideoInfo * a_cpVideoInfo)
{
	double fps = 0.0;
	double time = 0.0;
	if(a_cpVideoInfo->fpsDen != 0)
	{
		fps = (double)a_cpVideoInfo->fpsNum / (double)a_cpVideoInfo->fpsDen;
		time = (double)a_cpVideoInfo->numFrames *
		(double)a_cpVideoInfo->fpsDen / (double)a_cpVideoInfo->fpsNum;
	}

	QString infoString = QString("Frames: %frames% | Time: %time% | Size: "
		"%width%x%height% | FPS: %fpsnum%/%fpsden% = %fps% | Format: %format%");
	infoString.replace("%frames%", QString::number(a_cpVideoInfo->numFrames));
	infoString.replace("%time%", vsedit::timeToString(time, true));
	infoString.replace("%width%", QString::number(a_cpVideoInfo->width));
	infoString.replace("%height%", QString::number(a_cpVideoInfo->height));
	infoString.replace("%fpsnum%", QString::number(a_cpVideoInfo->fpsNum));
	infoString.replace("%fpsden%", QString::number(a_cpVideoInfo->fpsDen));
	infoString.replace("%fps%", QString::number(fps));
	infoString.replace("%format%", a_cpVideoInfo->format->name);

	return infoString;
}

// END OF QString vsedit::videoInfoString(const VSVideoInfo * a_cpVideoInfo)
//==============================================================================

double vsedit::qtimeToSeconds(const QTime & a_qtime)
{
    double seconds = (double)a_qtime.msec() / 1000.0;
    seconds += (double)a_qtime.second();
    seconds += (double)a_qtime.minute() * 60.0;
    seconds += (double)a_qtime.hour() * 360.0;
    return seconds;
}

// END OF double vsedit::qtimeToSeconds(const QTime & a_qtime)
//==============================================================================

QTime vsedit::secondsToQTime(double a_seconds)
{
	QTime qtime;

	if(a_seconds <= 0.0)
		return qtime;

	// Milliseconds cut-off
	a_seconds = std::round(a_seconds * 1000.0) / 1000.0;

	// Seconds
	uint64_t integer = (uint64_t)a_seconds;
	int seconds = integer % 60ll;
	integer /= 60ll;
	int minutes = integer % 60ll;
	integer /= 60ll;
	int hours = integer % 60ll;

	int milliseconds = (int)(((a_seconds - std::round(a_seconds)) * 1000.0));

	qtime.setHMS(hours, minutes, seconds, milliseconds);
	return qtime;
}

// END OF QTime vsedit::secondsToQTime(double a_seconds)
//==============================================================================

vsedit::FP32 vsedit::halfToSingle(vsedit::FP16 a_half)
{
	FP32 o = { 0 };

	// From ISPC ref code
	if (a_half.parts.Exponent == 0 && a_half.parts.Mantissa == 0)
		// (Signed) zero
		o.parts.Sign = a_half.parts.Sign;
	else
	{
		if (a_half.parts.Exponent == 0) // Denormal (will convert to normalized)
		{
			// Adjust mantissa so it's normalized (and keep track of exp adjust)
			int e = -1;
			unsigned int m = a_half.parts.Mantissa;
			do
			{
				e++;
				m <<= 1;
			} while ((m & 0x400) == 0);

			o.parts.Mantissa = (m & 0x3ff) << 13;
			o.parts.Exponent = 127 - 15 - e;
			o.parts.Sign = a_half.parts.Sign;
		}
		else if (a_half.parts.Exponent == 0x1f) // Inf/NaN
		{
			// NOTE: It's safe to treat both with the same code path
			// by just truncating lower Mantissa bits in NaNs (this is valid).
			o.parts.Mantissa = a_half.parts.Mantissa << 13;
			o.parts.Exponent = 255;
			o.parts.Sign = a_half.parts.Sign;
		}
		else // Normalized number
		{
			o.parts.Mantissa = a_half.parts.Mantissa << 13;
			o.parts.Exponent = 127 - 15 + a_half.parts.Exponent;
			o.parts.Sign = a_half.parts.Sign;
		}
	}

	return o;
}

// END OF vsedit::FP32 vsedit::halfToSingle(vsedit::FP16 a_half)
//==============================================================================
