#ifndef SETTINGS_MANAGER_CORE_H_INCLUDED
#define SETTINGS_MANAGER_CORE_H_INCLUDED

#include "settings_definitions_core.h"

#include <QObject>
#include <QVariant>
#include <vector>

/// Base class that manages non-GUI related settings
class SettingsManagerCore : public QObject
{
public:

	SettingsManagerCore(QObject * a_pParent);
	virtual ~SettingsManagerCore();

	//----------------------------------------------------------------------

	bool getPortableMode() const;

	bool setPortableMode(bool a_portableMod);

	//----------------------------------------------------------------------

	QStringList getVapourSynthLibraryPaths() const;

	bool setVapourSynthLibraryPaths(const QStringList & a_pathsList);

	bool getPreferVSLibrariesFromList() const;

	bool setPreferVSLibrariesFromList(bool a_prior);

	ResamplingFilter getChromaResamplingFilter() const;

	bool setChromaResamplingFilter(ResamplingFilter a_filter);

	YuvMatrixCoefficients getYuvMatrixCoefficients() const;

	bool setYuvMatrixCoefficients(YuvMatrixCoefficients a_matrix);

	ChromaPlacement getChromaPlacement() const;

	bool setChromaPlacement(ChromaPlacement a_placement);

	double getBicubicFilterParameterB() const;

	bool setBicubicFilterParameterB(double a_parameterB);

	double getBicubicFilterParameterC() const;

	bool setBicubicFilterParameterC(double a_parameterC);

	int getLanczosFilterTaps() const;

	bool setLanczosFilterTaps(int a_taps);

	DitherType getDitherType() const;

	bool setDitherType(DitherType a_dither);

	std::vector<EncodingPreset> getAllEncodingPresets() const;

	EncodingPreset getEncodingPreset(const QString & a_name) const;

	bool saveEncodingPreset(const EncodingPreset & a_preset);

	bool deleteEncodingPreset(const QString & a_name);

	std::vector<JobProperties> getJobs() const;

	bool setJobs(const std::vector<JobProperties> & a_jobs);

	QStringList getRecentJobServers() const;

	bool setRecentJobServers(const QStringList & a_servers);

	QStringList getTrustedClientsAddresses() const;

	bool setTrustedClientsAddresses(const QStringList & a_addresses);

protected:

	QVariant valueInGroup(const QString & a_group, const QString & a_key,
		const QVariant & a_defaultValue = QVariant()) const;

	bool setValueInGroup(const QString & a_group, const QString & a_key,
		const QVariant & a_value);

	bool deleteValueInGroup(const QString & a_group, const QString & a_key);

	QVariant value(const QString & a_key, const QVariant & a_defaultValue =
		QVariant()) const;

	bool setValue(const QString & a_key, const QVariant & a_value);

	QString m_settingsFilePath;
};

#endif // SETTINGS_MANAGER_CORE_H_INCLUDED
