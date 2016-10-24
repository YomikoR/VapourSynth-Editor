#include "settings_definitions.h"

//==============================================================================

EncodingPreset::EncodingPreset():
	  type(DEFAULT_ENCODING_TYPE)
	, headerType(DEFAULT_ENCODING_HEADER_TYPE)
{
}

EncodingPreset::EncodingPreset(const QString & a_name):
	  name(a_name)
	, type(DEFAULT_ENCODING_TYPE)
	, headerType(DEFAULT_ENCODING_HEADER_TYPE)
{
}

bool EncodingPreset::operator==(const EncodingPreset & a_other) const
{
	return (name == a_other.name);
}

bool EncodingPreset::operator<(const EncodingPreset & a_other) const
{
	return (name < a_other.name);
}

bool EncodingPreset::isEmpty() const
{
	return name.isEmpty();
}

//==============================================================================

bool StandardAction::operator==(const StandardAction & a_other) const
{
	return id == a_other.id;
}

bool StandardAction::operator<(const StandardAction & a_other) const
{
	return id < a_other.id;
}

//==============================================================================

CodeSnippet::CodeSnippet() :
	  name()
	, text()
{
}

CodeSnippet::CodeSnippet(const QString & a_name, const QString & a_text) :
	  name(a_name)
	, text(a_text)
{
}

bool CodeSnippet::operator==(const CodeSnippet & a_other) const
{
	return (name == a_other.name);
}

bool CodeSnippet::operator<(const CodeSnippet & a_other) const
{
	return (name < a_other.name);
}

bool CodeSnippet::isEmpty() const
{
	return (name.isEmpty() && text.isEmpty());
}

//==============================================================================
