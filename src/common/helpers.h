#ifndef HELPERS_H_INCLUDED
#define HELPERS_H_INCLUDED

#include <vapoursynth/VapourSynth.h>

#include <QString>
#include <QTime>
#include <cassert>
#include <algorithm>
#include <functional>

namespace vsedit
{

struct VariableToken
{
	QString token;
	QString description;
	std::function<QString()> evaluate;
};

QString timeToString(double a_seconds, bool a_fullFormat = false);

int mod(int a_value);

QString videoInfoString(const VSVideoInfo * a_cpVideoInfo);

double qtimeToSeconds(const QTime & a_qtime);

QTime secondsToQTime(double a_seconds);

QString subsamplingString(int a_subsamplingW, int a_subsamplingH);

QString subsamplingString(const VSFormat * a_cpFormat);

template<typename T1, typename T2, typename T3>
	void clamp(T1& a_value, const T2& a_low, const T3& a_high)
{
	assert(a_high > a_low);
	if(a_value < a_low)
		a_value = a_low;
	else if(a_value > a_high)
		a_value = a_high;
}

template<typename Container_T, typename Value_T>
	bool contains(const Container_T & a_container, const Value_T & a_value)
{
	return (std::find(std::begin(a_container), std::end(a_container),
		a_value) != std::end(a_container));
}

template<typename T>
	T roundUp(T a_number, T a_multiple)
{
	if(a_multiple == 0)
		return a_number;

	T remainder = a_number % a_multiple;
	if(remainder == 0)
		return a_number;

	return a_number + a_multiple - remainder;
}

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------

}

#endif // HELPERS_H_INCLUDED
