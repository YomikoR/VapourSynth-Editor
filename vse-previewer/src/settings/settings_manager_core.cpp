#include "settings_manager_core.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QSettings>

//==============================================================================

const char SETTINGS_FILE_NAME[] = "/vse-previewer.conf";

//==============================================================================

const char COMMON_GROUP[] = "common";

const char VAPOURSYNTH_LIBRARY_PATHS_KEY[] = "vapoursynth_library_paths";
const char PREFER_VS_LIBRARIES_FROM_LIST_KEY[] = "prefer_vs_libs_from_list";
const char CHROMA_RESAMPLING_FILTER_KEY[] = "chroma_resampling_filter";
const char YUV_MATRIX_COEFFICIENTS_KEY[] = "yuv_matrix_coefficients";
const char CHROMA_PLACEMENT_KEY[] = "chroma_placement";
const char BICUBIC_FILTER_PARAMETER_B_KEY[] = "bicubic_filter_parameter_b";
const char BICUBIC_FILTER_PARAMETER_C_KEY[] = "bicubic_filter_parameter_c";
const char LANCZOS_FILTER_TAPS_KEY[] = "lanczos_filter_taps";
const char DITHER_TYPE_KEY[] = "dither_type";

//==============================================================================

SettingsManagerCore::SettingsManagerCore(QObject * a_pParent) :
	QObject(a_pParent)
{
	QString applicationDir = QCoreApplication::applicationDirPath();

	bool portableMode = getPortableMode();
	if(portableMode)
		m_settingsFilePath = applicationDir + SETTINGS_FILE_NAME;
	else
	{
		m_settingsFilePath = QStandardPaths::writableLocation(
			QStandardPaths::GenericConfigLocation) + SETTINGS_FILE_NAME;
	}
}

SettingsManagerCore::~SettingsManagerCore()
{

}

//==============================================================================

bool SettingsManagerCore::getPortableMode() const
{
	QString applicationDir = QCoreApplication::applicationDirPath();
	QString settingsFilePath = applicationDir + SETTINGS_FILE_NAME;
	QFileInfo settingsFileInfo(settingsFilePath);

	bool portableMode = (settingsFileInfo.exists() &&
		settingsFileInfo.isWritable());
	return portableMode;
}

bool SettingsManagerCore::setPortableMode(bool a_portableMod)
{
	QString applicationDir = QCoreApplication::applicationDirPath();
	QString settingsFilePath = applicationDir + SETTINGS_FILE_NAME;
	QFileInfo settingsFileInfo(settingsFilePath);

	bool portableExists = settingsFileInfo.exists();
	bool currentModePortable = portableExists && settingsFileInfo.isWritable();

	if(a_portableMod == currentModePortable)
		return true;

	// In Windows, even if a dir is not writable, a file in it may still be
	// writable. Therefore, we should take a test by writing a file.
	bool portableWritable = false;
	if(a_portableMod && !portableExists)
	{
		QFile portableFile(applicationDir + SETTINGS_FILE_NAME);
		if((portableWritable = portableFile.open(QIODevice::WriteOnly)))
			portableFile.close();
	}
	else
		portableWritable = currentModePortable;

	if(a_portableMod && !portableWritable)
		return false;

	QString genericConfigDir = QStandardPaths::writableLocation(
		QStandardPaths::GenericConfigLocation);

	QString newSettingsFilePath;
	if(a_portableMod)
		newSettingsFilePath = applicationDir + SETTINGS_FILE_NAME;
	else
		newSettingsFilePath = genericConfigDir + SETTINGS_FILE_NAME;

	// When copying portable settings to common folder - another settings
	// file may already exist there. Need to delete it first.
	if(QFile::exists(newSettingsFilePath))
	{
		bool settingsFileDeleted = QFile::remove(newSettingsFilePath);
		if(!settingsFileDeleted)
			return false;
	}

	bool settingsFileCopied =
		QFile::copy(m_settingsFilePath, newSettingsFilePath);
	QString oldSettingsFilePath = m_settingsFilePath;
	m_settingsFilePath = newSettingsFilePath;

	if(a_portableMod)
		return settingsFileCopied;
	else if(settingsFileCopied)
	{
		bool portableSettingsFileDeleted = QFile::remove(oldSettingsFilePath);
		return portableSettingsFileDeleted;
	}

	return false;
}

bool SettingsManagerCore::getPreferVSLibrariesFromList() const
{
	return value(PREFER_VS_LIBRARIES_FROM_LIST_KEY,
		DEFAULT_PREFER_VS_LIBRARIES_FROM_LIST).toBool();
}

bool SettingsManagerCore::setPreferVSLibrariesFromList(bool a_prior)
{
	return setValue(PREFER_VS_LIBRARIES_FROM_LIST_KEY, a_prior);
}


//==============================================================================

QVariant SettingsManagerCore::valueInGroup(const QString & a_group,
	const QString & a_key, const QVariant & a_defaultValue) const
{
	QSettings settings(m_settingsFilePath, QSettings::IniFormat);
	settings.beginGroup(a_group);
	return settings.value(a_key, a_defaultValue);
}

bool SettingsManagerCore::setValueInGroup(const QString & a_group,
	const QString & a_key, const QVariant & a_value)
{
	QSettings settings(m_settingsFilePath, QSettings::IniFormat);
	settings.beginGroup(a_group);
	settings.setValue(a_key, a_value);
	settings.sync();
	bool success = (QSettings::NoError == settings.status());
	return success;
}

bool SettingsManagerCore::deleteValueInGroup(const QString & a_group,
	const QString & a_key)
{
	QSettings settings(m_settingsFilePath, QSettings::IniFormat);
	settings.beginGroup(a_group);
	settings.remove(a_key);
	settings.sync();
	bool success = (QSettings::NoError == settings.status());
	return success;
}

//==============================================================================

QVariant SettingsManagerCore::value(const QString & a_key,
	const QVariant & a_defaultValue) const
{
	return valueInGroup(COMMON_GROUP, a_key, a_defaultValue);
}

bool SettingsManagerCore::setValue(const QString & a_key,
	const QVariant & a_value)
{
	return setValueInGroup(COMMON_GROUP, a_key, a_value);
}

//==============================================================================

QStringList SettingsManagerCore::getVapourSynthLibraryPaths() const
{
	QStringList paths = value(VAPOURSYNTH_LIBRARY_PATHS_KEY).toStringList();
	paths.removeDuplicates();
	return paths;
}

bool SettingsManagerCore::setVapourSynthLibraryPaths(
	const QStringList & a_pathsList)
{
	return setValue(VAPOURSYNTH_LIBRARY_PATHS_KEY, a_pathsList);
}

//==============================================================================

ResamplingFilter SettingsManagerCore::getChromaResamplingFilter() const
{
	return (ResamplingFilter)value(CHROMA_RESAMPLING_FILTER_KEY,
		(int)DEFAULT_CHROMA_RESAMPLING_FILTER).toInt();
}

bool SettingsManagerCore::setChromaResamplingFilter(ResamplingFilter a_filter)
{
	return setValue(CHROMA_RESAMPLING_FILTER_KEY, (int)a_filter);
}

//==============================================================================

YuvMatrixCoefficients SettingsManagerCore::getYuvMatrixCoefficients() const
{
	return (YuvMatrixCoefficients)value(YUV_MATRIX_COEFFICIENTS_KEY,
		(int)DEFAULT_YUV_MATRIX_COEFFICIENTS).toInt();
}

bool SettingsManagerCore::setYuvMatrixCoefficients(
	YuvMatrixCoefficients a_matrix)
{
	return setValue(YUV_MATRIX_COEFFICIENTS_KEY, (int)a_matrix);
}

//==============================================================================

ChromaPlacement SettingsManagerCore::getChromaPlacement() const
{
	return (ChromaPlacement)value(CHROMA_PLACEMENT_KEY,
		(int)DEFAULT_CHROMA_PLACEMENT).toInt();
}

bool SettingsManagerCore::setChromaPlacement(ChromaPlacement a_placement)
{
	return setValue(CHROMA_PLACEMENT_KEY, (int)a_placement);
}

//==============================================================================

double SettingsManagerCore::getBicubicFilterParameterB() const
{
	return value(BICUBIC_FILTER_PARAMETER_B_KEY,
		DEFAULT_BICUBIC_FILTER_PARAMETER_B).toDouble();
}

bool SettingsManagerCore::setBicubicFilterParameterB(double a_parameterB)
{
	return setValue(BICUBIC_FILTER_PARAMETER_B_KEY, a_parameterB);
}

//==============================================================================

double SettingsManagerCore::getBicubicFilterParameterC() const
{
	return value(BICUBIC_FILTER_PARAMETER_C_KEY,
		DEFAULT_BICUBIC_FILTER_PARAMETER_C).toDouble();
}

bool SettingsManagerCore::setBicubicFilterParameterC(double a_parameterC)
{
	return setValue(BICUBIC_FILTER_PARAMETER_C_KEY, a_parameterC);
}

//==============================================================================

int SettingsManagerCore::getLanczosFilterTaps() const
{
	return value(LANCZOS_FILTER_TAPS_KEY, DEFAULT_LANCZOS_FILTER_TAPS).toInt();
}

bool SettingsManagerCore::setLanczosFilterTaps(int a_taps)
{
	return setValue(LANCZOS_FILTER_TAPS_KEY, a_taps);
}

DitherType SettingsManagerCore::getDitherType() const
{
    return (DitherType)value(DITHER_TYPE_KEY,
		(int)DEFAULT_DITHER_TYPE).toInt();
}

bool SettingsManagerCore::setDitherType(DitherType a_dither)
{
    return setValue(DITHER_TYPE_KEY, (int)a_dither);
}

//==============================================================================
