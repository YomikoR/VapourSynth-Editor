#include <utility>

#include "script_lexer_definitions.h"

Token::Token(QString a_text, int a_start, TokenType a_type):
	  text(std::move(a_text))
	, start(a_start)
	, type(a_type)
{

}

Token::Token(int a_start):
	start(a_start)
{

}

int Token::length() const
{
	return text.length();
}

bool Token::operator<(const Token & a_other) const
{
	return (start < a_other.start);
}
