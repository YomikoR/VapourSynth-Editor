#include "settings_manager_core.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QSettings>

//==============================================================================

const char SETTINGS_FILE_NAME[] = "/vsedit.config";

//==============================================================================

const char COMMON_GROUP[] = "common";

const char VAPOURSYNTH_LIBRARY_PATHS_KEY[] = "vapoursynth_library_paths";
const char PREFER_VS_LIBRARIES_FROM_LIST_KEY[] = "prefer_vs_libs_from_list";
const char VAPOURSYNTH_PLUGINS_PATHS_KEY[] = "vapoursynth_plugins_paths";
const char CHROMA_RESAMPLING_FILTER_KEY[] = "chroma_resampling_filter";
const char YUV_MATRIX_COEFFICIENTS_KEY[] = "yuv_matrix_coefficients";
const char CHROMA_PLACEMENT_KEY[] = "chroma_placement";
const char BICUBIC_FILTER_PARAMETER_B_KEY[] = "bicubic_filter_parameter_b";
const char BICUBIC_FILTER_PARAMETER_C_KEY[] = "bicubic_filter_parameter_c";
const char LANCZOS_FILTER_TAPS_KEY[] = "lanczos_filter_taps";
const char DITHER_TYPE_KEY[] = "dither_type";
const char RECENT_JOB_SERVERS_KEY[] = "recent_job_servers";
const char TRUSTED_CLIENTS_ADDRESSES_KEY[] = "trusted_clients_addresses";

//==============================================================================

const char ENCODING_PRESETS_GROUP[] = "encoding_presets";

const char ENCODING_PRESET_ENCODING_TYPE_KEY[] = "encoding_type";
const char ENCODING_PRESET_HEADER_TYPE_KEY[] = "header_type";
const char ENCODING_PRESET_EXECUTABLE_PATH_KEY[] = "executable_path";
const char ENCODING_PRESET_ARGUMENTS_KEY[] = "arguments";

//==============================================================================

const char JOBS_GROUP[] = "jobs";

const char JOB_ID_KEY[] = "id";
const char JOB_TYPE_KEY[] = "type";
const char JOB_STATE_KEY[] = "state";
const char JOB_DEPENDS_ON_JOBS_KEY[] = "depends_on_jobs";
const char JOB_TIME_STARTED_KEY[] = "time_started";
const char JOB_TIME_ENDED_KEY[] = "time_ended";
const char JOB_SCRIPT_NAME_KEY[] = "script_name";
const char JOB_SCRIPT_TEXT_KEY[] = "script_text";
const char JOB_ENCODING_TYPE_KEY[] = "encoding_type";
const char JOB_ENCODING_HEADER_TYPE_KEY[] = "encoding_header_type";
const char JOB_EXECUTABLE_PATH_KEY[] = "executable_path";
const char JOB_ARGUMENTS_KEY[] = "arguments";
const char JOB_SHELL_COMMAND_KEY[] = "shell_command";
const char JOB_FIRST_FRAME_KEY[] = "first_frame";
const char JOB_FIRST_FRAME_REAL_KEY[] = "first_frame_real";
const char JOB_LAST_FRAME_KEY[] = "last_frame";
const char JOB_LAST_FRAME_REAL_KEY[] = "last_frame_real";
const char JOB_FRAME_PROCESSED_KEY[] = "frames_processed";
const char JOB_FPS_KEY[] = "fps";

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

QStringList SettingsManagerCore::getVapourSynthPluginsPaths() const
{
	QStringList paths = value(VAPOURSYNTH_PLUGINS_PATHS_KEY).toStringList();
	paths.removeDuplicates();
	return paths;
}

bool SettingsManagerCore::setVapourSynthPluginsPaths(
	const QStringList & a_pathsList)
{
	return setValue(VAPOURSYNTH_PLUGINS_PATHS_KEY, a_pathsList);
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

std::vector<EncodingPreset> SettingsManagerCore::getAllEncodingPresets() const
{
	QSettings settings(m_settingsFilePath, QSettings::IniFormat);
	settings.beginGroup(ENCODING_PRESETS_GROUP);

	std::vector<EncodingPreset> presets;

	QStringList presetNames = settings.childGroups();
	for(const QString & presetName : presetNames)
	{
		settings.beginGroup(presetName);

		EncodingPreset preset;
		preset.name = presetName;
		preset.type = (EncodingType)settings.value(
			ENCODING_PRESET_ENCODING_TYPE_KEY, (int)DEFAULT_ENCODING_TYPE)
			.toInt();
		preset.headerType = (EncodingHeaderType)settings.value(
			ENCODING_PRESET_HEADER_TYPE_KEY, (int)DEFAULT_ENCODING_HEADER_TYPE)
			.toInt();
		preset.executablePath = settings.value(
			ENCODING_PRESET_EXECUTABLE_PATH_KEY).toString();
		preset.arguments = settings.value(
			ENCODING_PRESET_ARGUMENTS_KEY).toString();
		presets.push_back(preset);

		settings.endGroup();
	}

	return presets;
}

EncodingPreset SettingsManagerCore::getEncodingPreset(const QString & a_name) const
{
	QSettings settings(m_settingsFilePath, QSettings::IniFormat);
	settings.beginGroup(ENCODING_PRESETS_GROUP);

	EncodingPreset preset;

	QStringList presetNames = settings.childGroups();
	if(!presetNames.contains(a_name))
		return preset;

	preset.name = a_name;
	settings.beginGroup(a_name);

	preset.type = (EncodingType)settings.value(
		ENCODING_PRESET_ENCODING_TYPE_KEY, (int)DEFAULT_ENCODING_TYPE)
		.toInt();
	preset.headerType = (EncodingHeaderType)settings.value(
		ENCODING_PRESET_HEADER_TYPE_KEY, (int)DEFAULT_ENCODING_HEADER_TYPE)
		.toInt();
	preset.executablePath = settings.value(
		ENCODING_PRESET_EXECUTABLE_PATH_KEY).toString();
	preset.arguments = settings.value(
		ENCODING_PRESET_ARGUMENTS_KEY).toString();

	return preset;
}

bool SettingsManagerCore::saveEncodingPreset(const EncodingPreset & a_preset)
{
	QSettings settings(m_settingsFilePath, QSettings::IniFormat);
	settings.beginGroup(ENCODING_PRESETS_GROUP);
	settings.beginGroup(a_preset.name);

	settings.setValue(ENCODING_PRESET_ENCODING_TYPE_KEY, (int)a_preset.type);
	settings.setValue(ENCODING_PRESET_HEADER_TYPE_KEY,
		(int)a_preset.headerType);
	settings.setValue(ENCODING_PRESET_EXECUTABLE_PATH_KEY,
		a_preset.executablePath);
	settings.setValue(ENCODING_PRESET_ARGUMENTS_KEY, a_preset.arguments);

	settings.sync();
	bool success = (QSettings::NoError == settings.status());
	return success;
}

bool SettingsManagerCore::deleteEncodingPreset(const QString & a_name)
{
	QSettings settings(m_settingsFilePath, QSettings::IniFormat);
	settings.beginGroup(ENCODING_PRESETS_GROUP);

	QStringList subGroups = settings.childGroups();
	if(!subGroups.contains(a_name))
		return false;
	settings.remove(a_name);

	settings.sync();
	bool success = (QSettings::NoError == settings.status());
	return success;
}

//==============================================================================

std::vector<JobProperties> SettingsManagerCore::getJobs() const
{
	QSettings settings(m_settingsFilePath, QSettings::IniFormat);
	settings.beginGroup(JOBS_GROUP);

	std::vector<JobProperties> jobs;

	QStringList numbers = settings.childGroups();
	for(const QString & number : numbers)
	{
		settings.beginGroup(number);

		JobProperties job;

		QString idString = settings.value(JOB_ID_KEY).toString();
		if(idString.isEmpty())
			job.id = QUuid::createUuid();
		else
			job.id = QUuid(idString);

		job.type = (JobType)settings.value(JOB_TYPE_KEY,
			(int)DEFAULT_JOB_TYPE).toInt();
		job.jobState = (JobState)settings.value(JOB_STATE_KEY,
			(int)DEFAULT_JOB_STATE).toInt();

		QStringList dependencyIdStrings =
			settings.value(JOB_DEPENDS_ON_JOBS_KEY).toStringList();
		for(const QString & dependencyIdString : dependencyIdStrings)
		{
			QUuid dependencyId(dependencyIdString);
			job.dependsOnJobIds.push_back(dependencyId);
		}

		job.timeStarted = settings.value(JOB_TIME_STARTED_KEY).toDateTime();
		job.timeEnded = settings.value(JOB_TIME_ENDED_KEY).toDateTime();
		job.scriptName = settings.value(JOB_SCRIPT_NAME_KEY).toString();
		job.scriptText = settings.value(JOB_SCRIPT_TEXT_KEY).toString();

		job.encodingType = (EncodingType)settings.value(JOB_ENCODING_TYPE_KEY,
			(int)DEFAULT_ENCODING_TYPE).toInt();
		job.encodingHeaderType = (EncodingHeaderType)settings.value(
			JOB_ENCODING_HEADER_TYPE_KEY,
			(int)DEFAULT_ENCODING_HEADER_TYPE).toInt();

		job.executablePath = settings.value(JOB_EXECUTABLE_PATH_KEY).toString();
		job.arguments = settings.value(JOB_ARGUMENTS_KEY).toString();
		job.shellCommand = settings.value(JOB_SHELL_COMMAND_KEY).toString();

		job.firstFrame = settings.value(JOB_FIRST_FRAME_KEY,
			DEFAULT_JOB_FIRST_FRAME).toInt();
		job.firstFrameReal = settings.value(JOB_FIRST_FRAME_REAL_KEY,
			job.firstFrame).toInt();
		job.lastFrame = settings.value(JOB_LAST_FRAME_KEY,
			DEFAULT_JOB_LAST_FRAME).toInt();
		job.lastFrameReal = settings.value(JOB_LAST_FRAME_REAL_KEY,
			job.lastFrame).toInt();
		job.framesProcessed = settings.value(JOB_FRAME_PROCESSED_KEY,
			DEFAULT_JOB_FRAMES_PROCESSED).toInt();
		job.fps = settings.value(JOB_FPS_KEY, DEFAULT_JOB_FPS).toDouble();

		jobs.push_back(job);

		settings.endGroup();
	}

	return jobs;
}

bool SettingsManagerCore::setJobs(const std::vector<JobProperties> & a_jobs)
{
	QSettings settings(m_settingsFilePath, QSettings::IniFormat);
	settings.remove(JOBS_GROUP);
	settings.beginGroup(JOBS_GROUP);

	for(size_t i = 0; i < a_jobs.size(); ++i)
	{
		const JobProperties & job = a_jobs[i];

		settings.beginGroup(QString("%1").arg(i, 7, 10, QChar('0')));

		settings.setValue(JOB_ID_KEY, job.id.toString());
		settings.setValue(JOB_TYPE_KEY, (int)job.type);
		settings.setValue(JOB_STATE_KEY, (int)job.jobState);

		QStringList dependencyIdStrings;
		for(const QUuid & id : job.dependsOnJobIds)
			dependencyIdStrings << id.toString();
		settings.setValue(JOB_DEPENDS_ON_JOBS_KEY, dependencyIdStrings);

		settings.setValue(JOB_TIME_STARTED_KEY, job.timeStarted);
		settings.setValue(JOB_TIME_ENDED_KEY, job.timeEnded);
		settings.setValue(JOB_SCRIPT_NAME_KEY, job.scriptName);
		settings.setValue(JOB_SCRIPT_TEXT_KEY, job.scriptText);
		settings.setValue(JOB_ENCODING_TYPE_KEY, (int)job.encodingType);
		settings.setValue(JOB_ENCODING_HEADER_TYPE_KEY,
			(int)job.encodingHeaderType);
		settings.setValue(JOB_EXECUTABLE_PATH_KEY, job.executablePath);
		settings.setValue(JOB_ARGUMENTS_KEY, job.arguments);
		settings.setValue(JOB_SHELL_COMMAND_KEY, job.shellCommand);
		settings.setValue(JOB_FIRST_FRAME_KEY, job.firstFrame);
		settings.setValue(JOB_FIRST_FRAME_REAL_KEY, job.firstFrameReal);
		settings.setValue(JOB_LAST_FRAME_KEY, job.lastFrame);
		settings.setValue(JOB_LAST_FRAME_REAL_KEY, job.lastFrameReal);
		settings.setValue(JOB_FRAME_PROCESSED_KEY, job.framesProcessed);
		settings.setValue(JOB_FPS_KEY, job.fps);

		settings.endGroup();
	}

	settings.sync();
	bool success = (QSettings::NoError == settings.status());
	return success;
}

//==============================================================================

QStringList SettingsManagerCore::getRecentJobServers() const
{
	QStringList recentServers = value(RECENT_JOB_SERVERS_KEY).toStringList();
	recentServers.removeDuplicates();
	return recentServers.mid(0, DEFAULT_RECENT_JOB_SERVERS_NUMBER);
}

bool SettingsManagerCore::setRecentJobServers(const QStringList & a_servers)
{
	QStringList recentServers = a_servers;
	recentServers.removeDuplicates();
	recentServers = recentServers.mid(0, DEFAULT_RECENT_JOB_SERVERS_NUMBER);
	return setValue(RECENT_JOB_SERVERS_KEY, recentServers);
}

//==============================================================================

QStringList SettingsManagerCore::getTrustedClientsAddresses() const
{
	QStringList addresses = value(TRUSTED_CLIENTS_ADDRESSES_KEY).toStringList();
	addresses.removeDuplicates();
	return addresses;
}

bool SettingsManagerCore::setTrustedClientsAddresses(
	const QStringList & a_addresses)
{
	QStringList addresses = a_addresses;
	addresses.removeDuplicates();
	return setValue(TRUSTED_CLIENTS_ADDRESSES_KEY, addresses);
}

//==============================================================================
