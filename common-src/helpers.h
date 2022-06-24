#ifndef HELPERS_H_INCLUDED
#define HELPERS_H_INCLUDED

#include <vapoursynth/VapourSynth4.h>

#include "helpers_vs.h"

#include <QString>
#include <QTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QByteArray>
#include <QDataStream>
#include <QIODevice>
#include <algorithm>
#include <functional>

#if defined(Q_OS_WIN) && (QT_VERSION_MAJOR == 6)
#define APPLY_FONT_KERNING_PATCH
#endif

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

QString videoInfoString(const VSVideoInfo * a_cpVideoInfo,
	const VSAPI * a_cpVSAPI);

QString audioInfoString(const VSAudioInfo * a_cpAudioInfo,
	const VSAPI * a_cpVSAPI);

QString nodeInfoString(const VSNodeInfo &a_nodeInfo,
	const VSAPI * a_cpVSAPI);

double qtimeToSeconds(const QTime & a_qtime);

QTime secondsToQTime(double a_seconds);

void wait(int a_msec);

QString subsamplingString(int a_subsamplingW, int a_subsamplingH);

QString subsamplingString(const VSVideoFormat * a_cpFormat);

QString resolvePathFromApplication(const QString & a_relativePath);

QByteArray jsonMessage(const QString & a_command,
	const QJsonObject & a_jsonObject);
QByteArray jsonMessage(const QString & a_command,
	const QJsonArray & a_jsonArray);
QByteArray jsonMessage(const QString & a_command,
	const QJsonDocument & a_jsonDocument);

template<typename T1, typename T2, typename T3>
	void clamp(T1& a_value, const T2& a_low, const T3& a_high)
{
	Q_ASSERT(a_high > a_low);
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

template<typename T>
	QByteArray toByteArray(const T & a_data)
{
	QByteArray buf;
	QDataStream stream(&buf, QIODevice::WriteOnly);
	stream << a_data;
	return buf;
}

template<typename T>
	T fromByteArray(const QByteArray & a_array)
{
	QDataStream stream(a_array);
	T data;
	stream >> data;
	return data;
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

/* This is a patch for Qt 6 in Windows */
template<typename T>
void disableFontKerning(T * a_pWidget)
{
#if defined(APPLY_FONT_KERNING_PATCH)
	QFont font(a_pWidget->font());
	font.setKerning(false);
	a_pWidget->setFont(font);
#endif
}

//------------------------------------------------------------------------------

}

#endif // HELPERS_H_INCLUDED
