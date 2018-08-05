#ifndef SCRIPT_LEXER_DEFINITIONS_H
#define SCRIPT_LEXER_DEFINITIONS_H

#include <QString>
#include <list>
#include <vector>

enum class ScriptContextType
{
	Code,
	ModuleImport,
	Function,
	FunctionHeader,
	FunctionArguments,
	FunctionCallArguments,
	Expression,
};

enum class TokenType
{
	Undecided,
	Module,
	Keyword,
	Operator,
	String,
	Number,
	Comment,
	Variable,
	Function,
	FunctionArgument,
	VSModule,
	VSCore,
	VSNamespace,
	VSFunction,
	VSConstant,
	VSClip,
};

struct ScriptContext
{
	ScriptContextType type;
	int start;
	int end;
};

struct Token
{
	QString text;
	int start;
	TokenType type;
	std::list<ScriptContext> contextStack;

	Token(QString a_text, int a_start, TokenType a_type,
		std::list<ScriptContext>  a_contextStack);
	Token(int a_start);
	int length() const;
	bool operator<(const Token & a_other) const;
};

typedef std::vector<Token> TokenVector;
typedef TokenVector::iterator TokenIterator;

#endif // SCRIPT_LEXER_DEFINITIONS_H
