#include <QStringList>

#include "vsplugindata.h"

//==============================================================================

VSData::FunctionArgument::FunctionArgument():
	name()
	, type()
	, optional(false)
	, empty(false)
{

}

VSData::FunctionArgument::FunctionArgument(
	const VSData::FunctionArgument & a_other):
	name(a_other.name)
	, type(a_other.type)
	, optional(a_other.optional)
	, empty(a_other.empty)
{

}

VSData::FunctionArgument::FunctionArgument(
	VSData::FunctionArgument && a_other):
	name(std::move(a_other.name))
	, type(std::move(a_other.type))
	, optional(std::move(a_other.optional))
	, empty(std::move(a_other.empty))
{

}

VSData::FunctionArgument & VSData::FunctionArgument::operator=(
	VSData::FunctionArgument a_other)
{
	std::swap(name, a_other.name);
	std::swap(type, a_other.type);
	std::swap(optional, a_other.optional);
	std::swap(empty, a_other.empty);

	return *this;
}

QString VSData::FunctionArgument::toString() const
{
	return QString("%1 %2").arg(type).arg(name);
}

bool VSData::FunctionArgument::operator<(
	const VSData::FunctionArgument & a_other) const
{
	return (!optional && a_other.optional);
}

//==============================================================================

VSData::Function::Function():
	name()
	, arguments()
{

}

VSData::Function::Function(const VSData::Function & a_other):
	name(a_other.name)
	, arguments(a_other.arguments)
{

}

VSData::Function::Function(VSData::Function && a_other):
	name(std::move(a_other.name))
	, arguments(std::move(a_other.arguments))
{

}

VSData::Function & VSData::Function::operator=(VSData::Function a_other)
{
	if(&a_other == this)
		return *this;

	std::swap(name, a_other.name);
	std::swap(arguments, a_other.arguments);

	return *this;
}

QString VSData::Function::toString() const
{
	std::vector<VSData::FunctionArgument> argumentsCopy = arguments;
	std::sort(argumentsCopy.begin(), argumentsCopy.end());
	size_t argumentsNumber = argumentsCopy.size();
	size_t firstOptional;
	for(firstOptional = 0; (firstOptional < argumentsNumber) &&
		(!argumentsCopy[firstOptional].optional); ++firstOptional){};
	QStringList mandatoryArguments;
	for(size_t i = 0; i < firstOptional; ++i)
		mandatoryArguments << argumentsCopy[i].toString();
	QStringList optionalArguments;
	for(size_t i = firstOptional; i < argumentsNumber; ++i)
		optionalArguments << argumentsCopy[i].toString();
	QString argumentsString = mandatoryArguments.join(", ");
	if(!optionalArguments.isEmpty())
		argumentsString += QString("[, %1]").arg(optionalArguments.join(", "));
	return QString("%1(%2)").arg(name).arg(argumentsString);
}

bool VSData::Function::operator==(const VSData::Function & a_other) const
{
	return (name == a_other.name);
}

bool VSData::Function::operator<(const VSData::Function & a_other) const
{
	return (name.compare(a_other.name, Qt::CaseInsensitive) < 0);
}

//==============================================================================

VSData::Plugin::Plugin():
	filepath()
	, id()
	, pluginNamespace()
	, name()
	, functions()
{

}

VSData::Plugin::Plugin(const VSData::Plugin & a_other):
	filepath(a_other.filepath)
	, id(a_other.id)
	, pluginNamespace(a_other.pluginNamespace)
	, name(a_other.name)
	, functions(a_other.functions)
{

}

VSData::Plugin::Plugin(VSData::Plugin && a_other):
	filepath(std::move(a_other.filepath))
	, id(std::move(a_other.id))
	, pluginNamespace(std::move(a_other.pluginNamespace))
	, name(std::move(a_other.name))
	, functions(std::move(a_other.functions))
{

}

VSData::Plugin & VSData::Plugin::operator=(VSData::Plugin a_other)
{
	if(&a_other == this)
		return *this;

	std::swap(filepath, a_other.filepath);
	std::swap(id, a_other.id);
	std::swap(pluginNamespace, a_other.pluginNamespace);
	std::swap(name, a_other.name);
	std::swap(functions, a_other.functions);

	return *this;
}

bool VSData::Plugin::operator==(const VSData::Plugin & a_other) const
{
	return (id == a_other.id);
}

bool VSData::Plugin::operator<(const VSData::Plugin & a_other) const
{
	return (pluginNamespace.compare(a_other.pluginNamespace,
		Qt::CaseInsensitive) < 0);
}

//==============================================================================
