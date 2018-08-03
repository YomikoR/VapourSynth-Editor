#include "script_lexer_definitions.h"

int Token::length() const
{
	return text.length();
}

bool Token::operator<(const Token & a_other) const
{
	return (start < a_other.start);
}
