#include "helpers.h"

#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <cmath>

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
	int hours = integer;

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

QString vsedit::videoInfoString(const VSVideoInfo * a_cpVideoInfo,
	const VSAPI * a_cpVSAPI)
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
	char formatName[32];
	a_cpVSAPI->getVideoFormatName(&a_cpVideoInfo->format, formatName);
	infoString.replace("%format%", formatName);

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

void vsedit::wait(int a_msec)
{
	if(a_msec <= 0)
		return;

	QTime mark = QTime::currentTime().addMSecs(a_msec);
    while(QTime::currentTime() < mark)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

// END OF void vsedit::wait(int a_msec)
//==============================================================================

QString vsedit::subsamplingString(int a_subsamplingW, int a_subsamplingH)
{
	if((a_subsamplingW == 0) && (a_subsamplingH == 0))
		return QString("444");

	if((a_subsamplingW == 0) && (a_subsamplingH == 1))
		return QString("440");

	if((a_subsamplingW == 1) && (a_subsamplingH == 0))
		return QString("422");

	if((a_subsamplingW == 1) && (a_subsamplingH == 1))
		return QString("420");

	if((a_subsamplingW == 2) && (a_subsamplingH == 0))
		return QString("411");

	if((a_subsamplingW == 2) && (a_subsamplingH == 2))
		return QString("410");

	return QString();
}

// END OF QString vsedit::subsamplingString(int a_subsamplingW,
//		int a_subsamplingH)
//==============================================================================

QString vsedit::subsamplingString(const VSVideoFormat * a_cpFormat)
{
	if(!a_cpFormat)
		return QString();

	return subsamplingString(a_cpFormat->subSamplingW,
		a_cpFormat->subSamplingH);
}

// END OF QString vsedit::subsamplingString(const VSVideoFormat * a_cpFormat)
//==============================================================================

QString vsedit::resolvePathFromApplication(const QString & a_relativePath)
{
	// Remember the working directory and change it to application directory.
	QString cwd = QDir::currentPath();
	QString applicationDirPath = QCoreApplication::applicationDirPath();
	QDir::setCurrent(applicationDirPath);

	QFileInfo fileInfo(a_relativePath);
	// If no parent directory is specified - leave the path as it is.
	if(fileInfo.path() == ".")
		return(a_relativePath);
	QString absolutePath = fileInfo.absoluteFilePath();

	// Restore the working directory.
	QDir::setCurrent(cwd);

	return absolutePath;
}

// END OF QString vsedit::resolvePathFromApplication(
//		const QString & a_relativePath)
//==============================================================================

QByteArray vsedit::jsonMessage(const QString & a_command,
	const QJsonObject & a_jsonObject)
{
	return vsedit::jsonMessage(a_command, QJsonDocument(a_jsonObject));
}

// END OF QByteArray vsedit::jsonMessage(const QString & a_command,
//		const QJsonObject & a_jsonObject)
//==============================================================================

QByteArray vsedit::jsonMessage(const QString & a_command,
	const QJsonArray & a_jsonArray)
{
	return vsedit::jsonMessage(a_command, QJsonDocument(a_jsonArray));
}

// END OF QByteArray vsedit::jsonMessage(const QString & a_command,
//		const QJsonArray & a_jsonArray)
//==============================================================================

QByteArray vsedit::jsonMessage(const QString & a_command,
	const QJsonDocument & a_jsonDocument)
{
	return a_command.toUtf8() + ' ' + a_jsonDocument.toJson();
}

// END OF QByteArray vsedit::jsonMessage(const QString & a_command,
//		const QJsonDocument & a_jsonDocument)
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
