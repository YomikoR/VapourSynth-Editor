#ifndef VSPLUGINDATA_H_INCLUDED
#define VSPLUGINDATA_H_INCLUDED

#include <QString>
#include <vector>

/// Data, gathered from VS core and plugins for syntax highlighting,
/// autocompletion and reference.
namespace VSData
{

struct FunctionArgument
{
	QString name;
	QString type;
	bool optional;
	bool empty;

	FunctionArgument();
	FunctionArgument(const VSData::FunctionArgument & a_other);
	FunctionArgument(VSData::FunctionArgument && a_other);
	VSData::FunctionArgument & operator=(VSData::FunctionArgument a_other);

	QString toString() const;
	bool operator<(const VSData::FunctionArgument & a_other) const;
};

struct Function
{
	QString name;
	std::vector<VSData::FunctionArgument> arguments;

	Function();
	Function(const VSData::Function & a_other);
	Function(VSData::Function && a_other);
	VSData::Function & operator=(VSData::Function a_other);

	QString toString() const;
	bool operator==(const VSData::Function & a_other) const;
	bool operator<(const VSData::Function & a_other) const;
};

struct Plugin
{
	QString filepath;
	QString id;
	QString pluginNamespace;
	QString name;
	std::vector<VSData::Function> functions;

	Plugin();
	Plugin(const VSData::Plugin & a_other);
	Plugin(VSData::Plugin && a_other);
	VSData::Plugin & operator=(VSData::Plugin a_other);

	bool operator==(const VSData::Plugin & a_other) const;
	bool operator<(const VSData::Plugin & a_other) const;
};

}

typedef std::vector<VSData::Plugin> VSPluginsList;

#endif // VSPLUGINDATA_H_INCLUDED
