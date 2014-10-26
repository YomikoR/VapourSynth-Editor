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

	//--------------------------------------------------------------------------
	// Half to single precision float conversion
	// by Fabian "ryg" Giesen.

	union FP32
	{
		uint32_t u;
		float f;
		struct
		{
			unsigned int Mantissa : 23;
			unsigned int Exponent : 8;
			unsigned int Sign : 1;
		} parts;
	};

	union FP16
	{
		uint16_t u;
		struct
		{
			unsigned int Mantissa : 10;
			unsigned int Exponent : 5;
			unsigned int Sign : 1;
		} parts;
	};

	FP32 halfToSingle(FP16 a_half);

	//--------------------------------------------------------------------------
}

#endif // HELPERS_H_INCLUDED
